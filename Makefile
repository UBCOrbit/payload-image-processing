CFLAGS += -lturbojpeg -lm -g

# https://stackoverflow.com/questions/4349553/multiple-source-files-in-c-how-exactly-do-makefiles-work

test: test.c compress_image.o
	$(CC) -o test test.c compress_image.o $(CFLAGS)

compress_image.o: compress_image.c
	$(CC) -c compress_image.c $(CFLAGS)