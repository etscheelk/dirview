#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include <string.h>
#include <dirent.h> // directory abstraction (DIR struct)
#include <errno.h>

#include <math.h>

#include <sys/types.h> // types and stat for open
#include <sys/stat.h> // stat means status
#include <fcntl.h>
#include <sys/wait.h> // waitpid


/*
    API functions:
        1. read definitive directory contents and save to file
        2. take in typerBuffer content and fuzzy filter
            the contents of said file, return as string or 
            save to new file. --> see fzfTest
        3. if saving to file, a function to read it and return
            a string to driver file
*/

/*
    Questions:
        1. Should this file handle the file descriptors?
            Pipe obviously it must handle itself, but what 
            about passing a file descriptor of the storage
            files that must be handled?

            I currently lean towards having this file handle
            the files entirely itself. The driver file should
            only concern itself with getting the string results. 

        2. I actually don't need to save to a file. But should I?
            I don't know the size that the output will be
            (the size of their directory, that is), so this
            shouldn't be statically allocated 
                (or I can statically allocate max space).
            So dynamically allocate (ew) or save to a file. 
            I think saving to file and reading with `cat file`
            to stdout will be easier. 

            Remote code execution? Filter takes things quite
            literally into its search bar. Most dangerous thing
            would be to delete user's file. Don't think that
            can happen, but I'm not a security professional. 

        3. Should driver file know the name of the file(s)?
            Can't decide. 
*/

// FILENAME_MAX defined as 4096 in stdio. This is... too much.

// https://stackoverflow.com/questions/3437404/min-and-max-in-c
// C doesn't have min and max by default. Forgot that detail. 
// #define max(a,b)             \
// ({                           \
//     __typeof__ (a) _a = (a); \
//     __typeof__ (b) _b = (b); \
//     _a > _b ? _a : _b;       \
// })

#define min(a,b)             \
({                           \
    __typeof__ (a) _a = (a); \
    __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b;       \
})



int dirview_readdir();
int dirview_filterdir();

#define FILE_NAME_LEN 256
#define DIRENT_NAME_LEN 256
#define FILTER_LEN 128
#define DIR_TEXT_NAME ("./dircontents.txt")
#define DIR_TEXT_NAME_FILTERED ("./dircontentsfilt.txt")

// assume for now, save to file called "dir"
// assume local position for now `./`
int dirview_readdir()
{
    DIR *d = opendir("./");
    if (d == NULL)
    {
        return -1;
    }

    struct dirent *dir;
    char thisFileName[FILE_NAME_LEN]; // max length name we could print
    memset(thisFileName, 0, FILE_NAME_LEN);

    // O_TRUNC shortens file
    // int fd = open("dircontents.txt", O_RDWR | O_CLOEXEC | O_CREAT | O_TRUNC, S_IRWXU);
    FILE *f = fopen(DIR_TEXT_NAME, "w");
    while ( (dir = readdir(d)) != NULL )
    {
        size_t len = strnlen(dir->d_name, DIRENT_NAME_LEN); // see dirent definition, 256 bytes

        // question: copying over len will ignore \0. Consider implications. 
        size_t numToCopy = len < FILE_NAME_LEN ? len : FILE_NAME_LEN;

        memcpy(thisFileName, dir->d_name, numToCopy); // note: memcpy when fields non-overlapping, see man page

        // 1. use open (a system call). Maybe slower, maybe not portable
        // write(fd, thisFileName, numToCopy);
        // write(fd, "\n", 1);

        // 2a. FILE write, know how many chars to write
        fwrite(thisFileName, sizeof(char), numToCopy, f);
        fwrite("\n", sizeof(char), 1, f);

        // 2b. use print f, but have to zero out memory each time
        // fprintf(f, "%s\n", thisFileName);
        // memset(thisFileName, 0, FILE_NAME_LEN);
    }

    closedir(d);
    fclose(f);
    // close(fd);

    return EXIT_SUCCESS;
}


int dirview_filterdir(const char * filter)
{
    errno = 0;

    int fd_read_write[2] = {0, 0};
    int p = pipe(fd_read_write);
    if (p == -1)
    {
        perror("pipe failed");
        _exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1)
    {
        perror("fork failed");
        _exit(EXIT_FAILURE);
    }

    if (pid == 0)   // child
    {
		// close read-end
		// alter write-end to be STDOUT file number (dup2)
		// close write-end of pipe
		// exec

		// child runs this command, prints contents of file,
		// sent to stdout

        // replace with system-agnostic method?
        char *args[] = {"cat", DIR_TEXT_NAME, NULL};

        p = close(fd_read_write[0]);
        if (p == -1)
        {
            perror("child pipe read close failed");
            _exit(EXIT_FAILURE);
        }

        p = dup2(fd_read_write[1], STDOUT_FILENO);
        if (p == -1)
        {
            perror("child dup2 on pipe write end failed");
            _exit(EXIT_FAILURE);
        }

        p = close(fd_read_write[1]);
        if (p == -1)
        {
            perror("child pipe write end close failed");
            _exit(EXIT_FAILURE);
        }

        p = execvp(args[0], args);

        // only here if exec failed
        perror("child exec did not run");
        _exit(EXIT_FAILURE);
    }
    else            // parent
    {
		// close write-end
		// alter read-end to be STDIN file number (dup2)
		// close read-end of pipe
		// exec

		// Wait for child to finish their sending first,
		// so I don't read empty

        p = close(fd_read_write[1]);
        if (p == -1)
        {
            perror("parent write end pipe closed failed");
        }

        p = dup2(fd_read_write[0], STDIN_FILENO);
        if (p == -1)
        {
            perror("parent dup2 on read end failed");
        }

        p = close(fd_read_write[0]);
        if (p == -1)
        {
            perror("parent read end pipe close failed");
        }

        int status = 0;
        // waitpid(pid, &status, WEXITED);
        wait(&status);

        char filterBuf[FILTER_LEN];
        memset(filterBuf, 0, FILTER_LEN);

        // can't use quotes around it, i.e. "--filter=\"%s\""
		// Good and bad. BASH is the one usually being helpful with quotes, but
		//      running exec isn't doing bash.
		// However, exec doesn't care if there are spaces in the arguments,
		//      the arguments are already parsed and split!! So it will
		//      still assume it is part of the current argument (filter).
		snprintf(filterBuf, FILTER_LEN, "--filter=%s", filter);

        char *args[] = {"fzf", filterBuf, NULL};

        pid_t pid2 = fork();

        if (pid2 == 0)
        {
            execvp(args[0], args);

            _exit(EXIT_FAILURE);
        }
        else
        {
            status = 0;
            // waitpid(pid2, &status, WIFEXITED);
            wait(&status);
        }
    }


    return EXIT_SUCCESS;
}


// remove main
int main(int argc, char const *argv[])
{
    /* code */
    dirview_readdir();

    dirview_filterdir(argv[1]);

    return 0;
}
