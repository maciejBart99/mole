all: mole
mole: main.o cli.o indexing.o queries.o storage.o file_type.o bulk_io.o
	gcc -Wall -o mole main.o cli.o indexing.o queries.o storage.o file_type.o bulk_io.o -lpthread
main.o: src/main.c lib/cli.h
	gcc -Wall -c src/main.c
cli.o: src/cli.c lib/cli.h lib/queries.h lib/shared.h lib/indexing.h
	gcc -Wall -c src/cli.c -lpthread
indexing.o: src/indexing.c lib/indexing.h lib/shared.h lib/file_type.h lib/storage.h
	gcc -Wall -c src/indexing.c -lpthread
queries.o: src/queries.c lib/queries.h lib/queries.h lib/shared.h lib/file_type.h
	gcc -Wall -c src/queries.c
storage.o: src/storage.c lib/storage.h lib/shared.h
	gcc -Wall -c src/storage.c
file_type.o: src/file_type.c lib/file_type.h lib/shared.h
	gcc -Wall -c src/file_type.c
bulk_io.o: src/bulk_io.c lib/bulk_io.h
	gcc -Wall -c src/bulk_io.c
clean:
	rm *.o mole
.PHONY: clean all