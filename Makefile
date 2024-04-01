dirview: dirview.c
	nvc dirview.c -Wall -ltickit -Ltickit -o dirview

.PHONY:

clean: .PHONY
	rm -f dirview *.o
