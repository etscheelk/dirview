#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

// for open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h> // for wait

// https://www.cs.uleth.ca/~holzmann/C/system/pipeforkexec.html <-- source for pipe and stuff

/*
    Process:

        Show directory contents, save to some file 
            (file should be saved to same place as the binary)
        Dirview dir display should read what is in the file.
        The file should be piped into fzf alongside --filter=typerBuffer
            and the new results written to file again.

        Normal fzf behavior is a recursive folder search for all targets,
        but I don't want that. I want it to be local to just the current
        directory. This is why I must read the single-level directory 
        contents and save to a file first. 

*/

int main(int argc, char *argv[])
{
    // use `open` to create a file descriptor
    // int fd = open("testFile.txt", O_RDWR | O_CLOEXEC | O_CREAT, S_IRWXU);
    

    // char *args[] = {"cat", "testFile.txt", NULL};
    // execvp(args[0], args);
    
    // int pipefds[2] = {0, 1};

    int fdpair[2];  // fdpair[0] is read end, fdpair[1] is write end
                    // remember as read and write (not write and read)
    int status = pipe(fdpair);
    if (status == -1)
    {
        printf("pipe failed: %s\n", strerror(errno));
        // perror(strerror(errno));
    }

    printf("pipe output: %d (read) --- %d (write)\n", fdpair[0], fdpair[1]);

    pid_t pid = fork();
    if (pid == -1)
    {
        printf("fork failed: %s\n", strerror(errno));
    }

    if (pid == 0)   // if child
    {
        // close read-end
        // alter write-end to be STDOUT file number (dup2)
        // close write-end of pipe
        // exec

        char *args[] = {"cat", "testFile.txt", NULL};

        close(fdpair[0]);
        dup2(fdpair[1], STDOUT_FILENO);
        close(fdpair[1]);

        // printf("I'm the child and I should be done first\n");

        execvp(args[0], args);
        
        // if here exec failed
        _exit(1);
    }

    else            // if parent
    {
        // close write-end
        // alter read-end to be STDIN file number (dup2)
        // close read-end of pipe
        // exec



        status = 0;
        waitpid(pid, &status, WEXITED);

        // fprintf(stdout, "I'm the parent and I wait for the child to be done");

        char filterBuf[128] = "--filter=";
        strcat(filterBuf, argv[1]);

        char *args[] = {"fzf", filterBuf, NULL};

        close(fdpair[1]);
        dup2(fdpair[0], STDIN_FILENO);
        close(fdpair[0]);
        execvp(args[0], args);
        _exit(1);
    }

    // printf("pipe pair: %d --- %d\n", fdpair[0], fdpair[1]);

    // char *testArgs[] = {"fzf", "--filter=dir", NULL};

    // execvp(testArgs[0], testArgs);
}
