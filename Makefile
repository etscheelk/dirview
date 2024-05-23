# Extra NVC flags
flags = -gopt --display_error_number -pedantic \
-Minfo=all -Minit-msg -Minline -Msmartalloc \
-acc=host -nvmalloc


# no fast right now

all: dirview fzfTest

dirview: dirview.c
	nvc dirview.c -Wall -ltickit -Ltickit $(flags) -o dirview

fzfTest: fzfTest.c
	nvc fzfTest.c -Wall $(flags) -o fzfTest

.PHONY:

clean: .PHONY
	rm -f dirview fzfTest *.o
