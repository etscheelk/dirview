#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

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

    int pipefds[2] = {0, 1};
    int status = pipe(pipefds);
    if (status == -1)
    {
        fprintf(stderr, "pipe failed: %s", strerror(errno));
    }

    char *testArgs[] = {"fzf", "--filter=dir", NULL};

    execvp(testArgs[0], testArgs);
}