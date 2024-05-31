#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    int fd_read_write[2] = {0, 0};

    pipe(fd_read_write);

    FILE *tmp = tmpfile();
    int fd = fileno(tmp);
    // dup2(fd, STDOUT_FILENO);

    dup2(fd_read_write[0], fd);
    close(fd_read_write[0]);

    dup2(fd_read_write[1], STDOUT_FILENO);
    close(fd_read_write[1]);


    char *args1[] = {"cat", "Makefile", NULL};
    char *args2[] = {"echo", NULL};

    pid_t pid = vfork();

    if (pid == 0)
    {
        execvp(args1[0], args1);

        _exit(EXIT_FAILURE);
    }
    else
    {
        wait(NULL);
        
        // char buf[1024] = "";
        char *buf = NULL;
        size_t num = 0;

        ssize_t num_bytes = getdelim(&buf, &num, '\0', tmp);

        close(fd);
        fclose(tmp);

        // read(fd, buf, 128);
        buf[0] = 'X';        

        printf("\nbuff:\n%s\n", buf);
        printf("num bytes: %zd", num_bytes);

        free(buf);
    }

    /* code */
    return 0;
}
