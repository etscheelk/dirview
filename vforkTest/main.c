#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define N 2

#define forxy \
for (int x = 0; x < N; ++x) \
    for (int y = 0; y < N; ++y)



int main(int argc, char *argv[])
{
    #ifdef fun
    forxy {
        printf("%d -- %d\n", x, y);
    }
    #endif

    int fd_read_write[2] = {0, 0};

    int fds2[2] = {0, 0};

    pipe(fd_read_write);
    // pipe(fds2);

    // fds2[]

    FILE *tmp = tmpfile();
    int fd = fileno(tmp);
    // dup2(fd, STDOUT_FILENO);
    dup2(STDOUT_FILENO, fd);

    dup2(fd_read_write[0], STDIN_FILENO);
    close(fd_read_write[0]);

    dup2(fd_read_write[1], STDOUT_FILENO);
    close(fd_read_write[1]);


    char *args1[] = {"cat", "Makefile", NULL};
    char *args2[] = {"echo", NULL};

    pid_t pid = vfork();

    if (pid == 0)
    {
        // printf("before exec\n");
        execvp(args1[0], args1);

        _exit(EXIT_FAILURE);
    }
    else
    {
        
        // printf("Before Wait\n");
        // wait(NULL);
        // printf("After wait\n");
        // fprintf(stderr, "After wait\n");
        
        // char buf[1024] = "";
        char *buf = NULL;
        size_t num = 0;

        ssize_t num_bytes = getdelim(&buf, &num, '\0', tmp);

        close(fd);
        fclose(tmp);

        // read(fd, buf, 128);
        buf[0] = 'X';        

        printf("num bytes: %zd\n", num_bytes);
        printf("\n--buff--\n%s\n--buff done--\n", buf);

        for (int i = 0; i < num_bytes; ++i) {
            printf("%c\n", buf[i]);
        }

        free(buf);
    }

    /* code */
    return 0;
}
