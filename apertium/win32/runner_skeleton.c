#include <stdio.h>
#include <conio.h>
#include <process.h>
#include <direct.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#define PATH_BUF_SIZE 8191
#define NUM_EXEC_ARGS 3
#define ARG_BUF_SIZE (8191 - NUM_EXEC_ARGS)
#define ENV_VAR_SIZE 32768

/* Strip the last component off a pathname.
   Thus, parent("a\b\c") -> "a\b" */
char* parent(char* parent_buf) {
        char* pos = strrchr(parent_buf, '\\');
        pos[0] = '\0';

        return parent_buf;
}

/* Remove the .exe if the user invoked this executable with its extension.
   That is, if the user typed something like apertium.exe instead of apertium. */
char* remove_extension(char* buf) {
        char* pos = strrchr(buf, '.');

        if (pos != NULL && strcmp(pos, ".exe") == 0) {
                pos[0] = '\0';
        }

        return buf;
}

#define MIN(x, y) ((x) < (y) ? x : y)

int main(int argc, char* argv[]) {
        char *args[ARG_BUF_SIZE];
        char base_path[PATH_BUF_SIZE + 1];
        char script_path[PATH_BUF_SIZE + 1];
        char shell_path[PATH_BUF_SIZE + 1];
        char env_path[ENV_VAR_SIZE];
        int argi;

        _fullpath(shell_path, argv[0], PATH_BUF_SIZE);
        strcpy(script_path, shell_path);
        strcpy(base_path, shell_path);

        parent(shell_path);
        strcat(shell_path, "\\sh.exe");

        remove_extension(script_path);
        parent(base_path);

        args[0] = shell_path;
        args[1] = "--norc";
        args[2] = script_path;

        /* Any parameters passed on the command line will be passed through to the shell script */
        for (argi = 0; argi < MIN(argc - 1, ARG_BUF_SIZE); argi++) {
                printf("%s\n", argv[argi + 1]);
                args[argi + NUM_EXEC_ARGS] = argv[argi + 1];
        }
        /* Signal the end of the argument list */
        args[argi + NUM_EXEC_ARGS] = NULL;

        /* Add this executable's directory to the path */
        strcpy(env_path, "PATH=");
        strcat(env_path, getenv("PATH"));
        strcat(env_path, ";");
        strcat(env_path, base_path);
        _putenv(env_path);

        _spawnv(_P_WAIT, args[0], &args[1]);

        _flushall();
}
