CFLAGS += -lturbojpeg -lm -g

all: compress_image

compress_image: compress_image.c
	$(CC) -o compress_image compress_image.c $(CFLAGS)

