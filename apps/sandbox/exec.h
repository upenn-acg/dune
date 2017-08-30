#ifndef EXEC_H
#define EXEC_H
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
int
exec_execev(const char *filename, char *const argv[], char *const envp[],
            const char* sandboxExecutable);

#endif
