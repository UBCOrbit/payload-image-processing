CFLAGS += -lturbojpeg

all: compress_image

compress_image: compress_image.c downsample.c
	$(CC) -o compress_image compress_image.c $(CFLAGS)

