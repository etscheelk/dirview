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

#define FILE_NAME_LEN 256
// FILENAME_MAX defined as 4096 in stdio. This is... too much.

// https://stackoverflow.com/questions/3437404/min-and-max-in-c
// C doesn't have min and max by default. Forgot that detail. 
// #define max(a,b)             \
// ({                           \
//     __typeof__ (a) _a = (a); \
//     __typeof__ (b) _b = (b); \
//     _a > _b ? _a : _b;       \
// })

// #define min(a,b)             \
// ({                           \
//     __typeof__ (a) _a = (a); \
//     __typeof__ (b) _b = (b); \
//     _a < _b ? _a : _b;       \
// })


int dirview_readdir();
int dirview_filterdir();

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
    int fd = open("dircontents.txt", O_RDWR | O_CLOEXEC | O_CREAT, S_IRWXU);
    while ( (dir = readdir(d)) != NULL )
    {
        size_t len = strnlen(dir->d_name, FILENAME_MAX);

        // question: copying over len will ignore \0. Consider implications. 
        size_t numToCopy = len < FILE_NAME_LEN ? len : FILE_NAME_LEN;

        memcpy(thisFileName, dir->d_name, numToCopy); // note: memcpy when fields non-overlapping, see man page
        write(fd, thisFileName, numToCopy);
        write(fd, "\n", 1);
    }

    closedir(d);
    close(fd);

    return EXIT_SUCCESS;
}


int dirview_filterdir()
{


    return EXIT_SUCCESS;
}


// remove main
int main(int argc, char const *argv[])
{
    /* code */
    dirview_readdir();

    return 0;
}
