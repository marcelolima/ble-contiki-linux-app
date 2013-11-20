CC=gcc
FILES=server6.c client6.c
OUT=server6 client6

build: $(FILES)
	$(CC) -o server6 server6.c
	$(CC) -o client6 client6.c

.PHONY: clean

clean: $(OUT)
	rm -f *.o $(OUT)

rebuild: clean build
