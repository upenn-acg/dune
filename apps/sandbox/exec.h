#ifndef EXEC_H
#define EXEC_H

int
exec_execev(const char *filename, char *const argv[], char *const envp[],
            const char* exePath);

#endif
