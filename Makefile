# Extra NVC flags
flags = -gopt --display_error_number -pedantic \
-Minfo=all -Minit-msg -Minline -Msmartalloc \
-acc=host -nvmalloc


# no fast right now

dirview: dirview.c
	nvc dirview.c -Wall -ltickit -Ltickit $(flags) -o dirview

.PHONY:

clean: .PHONY
	rm -f dirview *.o
