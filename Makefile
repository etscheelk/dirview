dirview: dirview.c
	clang dirview.c -Wall -ltickit -Ltickit -o dirview

.PHONY:

clean: .PHONY
	rm -f dirview *.o
