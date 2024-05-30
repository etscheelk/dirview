# Extra NVC flags
flags = -gopt --display_error_number -pedantic \
-Minfo=all -Minit-msg -Minline  #-Msmartalloc \
-acc=host # -nvmalloc


# no fast right now

all: dirReader_o dirview fzfTest

dirview: dirview.c dirReader_o
	nvc dirview.c dirReader.o -Wall -ltickit -Ltickit $(flags) -o dirview -D DIRVIEW_LINKED

fzfTest: fzfTest.c
	nvc fzfTest.c -Wall $(flags) -o fzfTest

dirReader_o: dirReader.c
	nvc dirReader.c -c $(flags) -o dirReader.o -D DIRVIEW_LINKED

.PHONY:

clean: .PHONY
	rm -f dirview fzfTest dirReader *.o
