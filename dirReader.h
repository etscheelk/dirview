#ifndef DIR_READER_H
#define DIR_READER_H

#define FILE_NAME_LEN 256
#define DIRENT_NAME_LEN 256
#define FILTER_LEN 128
#define DIR_TEXT_NAME ("./dircontents.txt")
#define DIR_TEXT_NAME_FILTERED ("./dircontentsfilt.txt")

int dirview_readdir();
int dirview_filterdir(char *filter);
char *dirview_readfilter();

#endif