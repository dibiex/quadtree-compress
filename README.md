# quadtree-compress
The application uses quadtree structures to compress .ppm images.

## Build
Use the Makefile
~~~
make
~~~
##  Uses 
### Compress image into image format
Compresses the image into an image format.
~~~
Command:
./quadtree -m n <compression_level> <image_input> <image_output>
Example:
./quadtree -m n 370 image.ppm image_out.ppm
~~~
<img src=https://i.imgur.com/ualZIbc.png width="256" height="256"/>

### Compress image into binary format
Compresses the image into a binary format that can be interpreted by the program.
~~~
Command:
./quadtree -c <compression_level> <image_input> <binary_output>
Example:
./quadtree -c 370 image.ppm binary.out
~~~
### Decompress image
Decompresses the binary output of the compress comman into an image.
~~~
Command:
./quadtree -d <binary_input> <image_output>
Example:
./quadtree -d binary.out image_out.ppm
~~~
### Flip Image
Flips image horizontally or vertically.

Horizontal flip:
~~~
Command:
./quadtree -m h <compression_level> <image_input> <image_output>
Example:
./quadtree -m h 100 image.ppm horizontal.ppm
~~~
<img src=https://i.imgur.com/cBrSKnK.png width="256" height="256"/>

Vertical flip:
~~~
Command:
./quadtree -m v <compression_level> <image_input> <image_output>
Example:
./quadtree -m v 100 image.ppm vertical.ppm
~~~
<img src=https://i.imgur.com/zd3jOXf.png width="256" height="256"/>

For flipping without compression use the compression_level 0
~~~
./quadtree -m v 0 image.ppm vertical.ppm
~~~
<img src=https://i.imgur.com/isJsLUW.png width="256" height="256"/>

### Overlay two images
~~~
Command:
./quadtree -o <compression_level> <image_input1> <image_input2> <overlay_output>
Example:
./quadtree -o 0 image1.ppm image2.ppm overlay.ppm
~~~
<img src=https://i.imgur.com/WQcut5b.jpg width="256" height="256"/> <img src=https://i.imgur.com/J4llid0.png width="256" height="256"/> <img src=https://i.imgur.com/3qdYckA.png width="256" height="256"/>
