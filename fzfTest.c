#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

// for open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/wait.h> // for wait

// https://www.cs.uleth.ca/~holzmann/C/system/pipeforkexec.html <--- source for pipe and stuff

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

#define FILTER_LEN 128

void printError(void)
{
	perror(strerror(errno));
}

int main(int argc, char *argv[])
{
	errno = 0;
	// use `open` to create a file descriptor
	// int fd = open("testFile.txt", O_RDWR | O_CLOEXEC | O_CREAT, S_IRWXU);

	// char *args[] = {"cat", "testFile.txt", NULL};
	// execvp(args[0], args);

	// int pipefds[2] = {0, 1};

	int fdpair[2]; // fdpair[0] is read end, fdpair[1] is write end
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

	if (pid == 0) // if child
	{
		// close read-end
		// alter write-end to be STDOUT file number (dup2)
		// close write-end of pipe
		// exec

		// child runs this command, prints contents of file,
		// sent to stdout
		char *args[] = {"cat", "testFile.txt", NULL};

		int p = close(fdpair[0]);
		if (p == -1)
		{
			printError();
		}

		// I can write with just string, possible to avoid file saving
		// write(fdpair[1], "ethan\nethan scheelk\nhello\n\r", 30);
		// close(fdpair[1]);

		p = dup2(fdpair[1], STDOUT_FILENO);
		if (p == -1)
		{
			printError();
		}

		p = close(fdpair[1]);
		if (p == -1)
		{
			printError();
		}

		p = execvp(args[0], args);

		// if here exec failed
		// if (p == -1) // not necessary
		{
			perror(strerror(errno));
			printf("This is from errno");
			// printf("%s this is from errno", strerror(errno));
			_exit(1);
		}
	}

	else // if parent
	{
		// close write-end
		// alter read-end to be STDIN file number (dup2)
		// close read-end of pipe
		// exec

		// Wait for child to finish their sending first,
		// so I don't read empty
		status = 0;
		waitpid(pid, &status, WEXITED);

		char filterBuf[FILTER_LEN] = "";
		// can't use quotes around it, i.e. "--filter=\"%s\""
		// Good and bad. BASH is the one usually being helpful with quotes, but
		//      running exec isn't doing bash.
		// However, exec doesn't care if there are spaces in the arguments,
		//      the arguments are already parsed and split!! So it will
		//      still assume it is part of the current argument (filter).
		snprintf(filterBuf, FILTER_LEN, "--filter=%s", argv[1]);

		// Can use strcat (annoying) or snprintf (easier) (probably safer)
		// char filterBuf[FILTER_LEN] = "--filter=";
		// strcat(filterBuf, argv[1]);

		for (int i = 0; i < FILTER_LEN; ++i)
		{
			printf("%3d\t%c\n", filterBuf[i], filterBuf[i]);
		}
		printf("%s\n", filterBuf);

		// ^ I'll need to put quotes around

		char *args[] = {"fzf", "--print-query", filterBuf, NULL};

		int p = close(fdpair[1]);
		if (p == -1)
		{
			printError();
		}

		p = dup2(fdpair[0], STDIN_FILENO);
		if (p == -1)
		{
			printError();
		}

		p = close(fdpair[0]);
		if (p == -1)
		{
			printError();
		}

		p = execvp(args[0], args);
		// if here exec failed
		// if (p == -1) // not necessary
		{
			perror(strerror(errno));
			_exit(1);
		}
	}
}
