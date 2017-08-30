/**
 * The user wants to make a call to execev. We append ./sandbox in front of the
 * command string they're trying to pass. Therefore, the process they're execve-ing
 * will also be sandboxed.
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <err.h>

#define EXECPGSIZE 4096

extern char **environ;
const char *ld_path = "/lib64/ld-linux-x86-64.so.2";


int
getlen(char *const arr[])
{
  int i;

  for (i = 0; arr[i] != NULL; i++)
    ;

  return i;
}

/**
 * Wrapper around call to execve. This functions performs @execve() but appends the name
 * of the sandbox in front of the command. This way the program to exec will be sandboxed.
 * @param programToExecute: Full path to program to execute.
 * @param argv: Array of strings where each entry is an argument to @programToExecute.
 * @param envp: Optional enviornment to pass in. See man page of exec for description
 *              of expected format.
 * @param sandboxExecutable: binary of *this* program with full path so we can append
 *                           it in front of our exec command.
 * @return: 0 on success.
 *
 */
int exec_execev(const char *programToExecute, char *const argv[],
                char *const envp[], const char* sandboxExecutable){
  /* printf("programToExecute %s\n", programToExecute); */
  int i;
  int fd;
  int len;
  char page[EXECPGSIZE];

  fd = open(programToExecute, O_RDONLY);
  if (fd < 0) {
    return -errno;
  }

  len = read(fd, page, EXECPGSIZE);
  if (len < 2) {
    return -ENOEXEC;
  }

  close(fd);

  if (len < EXECPGSIZE)
    page[len] = 0;

  /* The program the user wants to execute is an ELF file. */
  if (page[0] == 0x7f && page[1] == 'E' && page[2] == 'L'&& page[3] == 'F') {
    int i;
    int arglen = getlen(argv);
    const char **new_argv = (const char **)malloc(sizeof(char *)*(arglen+4));

    /* Per convention, the first argument should be the name of the executable.
       In our case, this is the sandbox. The first argument to the sandbox should
       be our loader. Then we give the name of the executable /we/ wanted to run.
     */
    new_argv[0] = sandboxExecutable;
    new_argv[1] = ld_path;
    new_argv[2] = programToExecute;
    for (i = 1; i <= arglen; i++)
      new_argv[i + 2] = argv[i];
    new_argv[arglen + 3] = NULL;

    int status = fork();
    if (status < 0)
      return status;
    else if (status > 0)
      exit(0);

    execve(sandboxExecutable, (char* const*)new_argv, envp);

    // Execve only return when the it ran sucessfully. Otherwise it continues
    // with an error. Since we are here there was an error.

    // Print Command attempted.
    err(1, "sandbox/exec_execev.c: Execve for elf file attempted but failed.\nError");

    fprintf(stderr, "Command attempted:\n");
    for (i = 0; new_argv[i] != NULL; i++)
      fprintf(stderr, "'%s' ", new_argv[i]);
    exit(1);
    return -errno;
  }

  /* User attempting to call shell script. */
  if (page[0] == '#' || page[1] == '!') {
    bool no_args;
    int i;
    int arglen = getlen(argv);
    const char **new_argv = (const char **)malloc(sizeof(char *)*(arglen+7));

    // Parse interpreter and arguments.  According to FreeBSD's historical
    // note in sys/kern/imgact_shell.c the most compatible behavoir is to
    // parse all interpreter arguments as a single argv into the
    // application.  If there are no arguments the first argument will be
    // the script itself.  Parsing excess whitespace from the beginning and
    // end is optional and most systems do not do that.

    // Call sandbox with the following arguements:
    // /lib64/ld... <interp> <argstring> <script> <arg1> ... <argn>

    for (i = 2; i < EXECPGSIZE; i++) {
      if (page[i] != ' ' && page[i] != '\t')
        break;
    }
    int interp_begin = i;
    for (; i < EXECPGSIZE; i++) {
      if (page[i] == ' ' || page[i] == '\t' ||
          page[i] == '\n' || page[i] == '\0')
        break;
    }
    int interp_end = i;
    if (interp_begin == interp_end || page[interp_begin] == '\0') {
      return -ENOEXEC;
    }

    int arg_begin, arg_end;
    if (page[interp_end] == '\n') {
      no_args = true;
    } else {
      no_args = false;
      for (; i < EXECPGSIZE; i++) {
        if (page[i] == '\n' || page[i] == '\0')
          break;
      }

      arg_begin = interp_end + 1;
      arg_end = i;
      page[i] = '\0';
    }
    page[interp_end] = '\0';

    new_argv[0] = sandboxExecutable;
    new_argv[1] = ld_path;
    new_argv[2] = page + interp_begin;
    if (no_args) {
      new_argv[3] = programToExecute;
      for (i = 0; i <= arglen; i++)
        new_argv[i + 4] = argv[i];
      new_argv[arglen + 5] = NULL;
    } else {
      new_argv[3] = page + arg_begin;
      new_argv[4] = programToExecute;
      for (i = 0; i <= arglen; i++)
        new_argv[i + 5] = argv[i];
      new_argv[arglen + 6] = NULL;
    }

    for (i = 0; new_argv[i] != NULL; i++)
      printf("'%s' ", new_argv[i]);
    printf("\n");

    int status = fork();
    if (status < 0)
      return status;
    else if (status > 0)
      exit(0);

    execve(sandboxExecutable, (char* const*)new_argv, envp);

    // Execve only return when the it ran sucessfully. Otherwise it continues
    // with an error. Since we are here there was an error.
    err(1, "sandbox/exec.c: exec_execve for a shell script attempted but failed.\nError");
    fprintf(stderr, "Command attempted:\n");
    for (i = 0; new_argv[i] != NULL; i++){
      fprintf(stderr, "'%s' ", new_argv[i]);
    }

    exit(1);
    return -errno;
  }

  return -ENOEXEC;
}


#ifdef TEST_EXEC

int main(int argc, char *argv[])
{
  char *const args[] = { NULL };

  //exec_execev("/bin/ls", args, environ);
  //exec_execev("test.sh", args, environ);

  return 0;
}

#endif /* TEST_EXEC */
