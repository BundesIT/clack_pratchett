default: clack-pratchett.so

clack-pratchett.so:
	tsxs -o clack-pratchett.so clack-pratchett.c

install: clack-pratchett.so
	tsxs -i -o clack-pratchett.so

clean:
	rm -f *.o *.lo *.so
