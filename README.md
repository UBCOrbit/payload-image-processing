# Payload
#### UBC Orbit's Payload Software.
This C program handles the downsampling, cropping, and compression of raw image data. It outputs a JPEG image within a few KB of 100KB. The function “compress_image” calls the individual functions that handle the above processes.

The test.c file runs the program through a set of test images to simulate real use.
## Setup
Linux:
````
sudo apt install libturbojpeg-dev

make
````
