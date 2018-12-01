CFLAGS += -lturbojpeg -lm -g

all: test compress_image

test: test.c
	$(CC) -o test test.c $(CFLAGS)

compress_image: compress_image.c
	$(CC) -o compress_image compress_image.c $(CFLAGS)
