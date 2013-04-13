/*_____________________________________________________________________________
image.c: This is a file provides the implementation of the function prototypes
	given in image.h
_______________________________________________________________________________
Author: 
    Aniket V. Oak
Date Created:
    07-Apr-2013
_______________________________________________________________________________*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"

/* imread: This function takes a string and a pointer to image structure. 
	It then opens the image named the string (assumes it is a bmp image) and 
	populates the image structure with pixel data, bmp header and index of
	colours (if any). 
	Returns 0 on success and 1 on failure */

int imread (char * imname, image * im) {

	if (imname == NULL) {
		printf("ERROR: image name not specified %p\n",imname);
		return 1;
		}
	
	im->is_indexed = 0;					// set the indexed field to 0 for start
	im->max_val = im->min_val = 0;		// set min and max values to be 0
	im->is_rgb=0;						// set the rgb flag to 0 for now
	

	FILE * fp;

	fp = fopen(imname,"rb+");			// now open the image file
	if (fp == NULL) {
		printf("ERROR: Reading the image file failed\n");
		return 1;
		}
	
	/* Now read the bmp file header into the bmp header structure */
	if (read_header(&im->h, fp)) {
		printf("ERROR: Reading the header failed\n");
		fclose(fp);
		return 1;
		}

	/* check if we have a colour palette. */
	if  (im->h.num_c > 0) {
		im->is_indexed = 1;
		}

	/* check if the image is rgb. If so, we will need to allocate the
	data array as colour triplet for each pixel. */
	if (im->h.bits == 24 || im->h.num_c > 0) {
		im->is_rgb = 1;
		}
	
	/* NOTE: if the image is indexed or if image is 24 bits, we allocate pixel
	data as colour triplets. There is a possibility that the image may still be
	greyscale, but then maybe it was stored in a wrong way. we can provide a function
	converting from RGB to Greyscale */

	int i,j;

	if (allocate_data_array(im)) {
		printf("ERROR: Error allocating data for image pixels\n");
		fclose(fp);
		return 1;
		}
	
	if (read_pixels(im,fp)) {
		printf("ERROR: Error reading pixel data\n");
		free(im->c_data);
		free(im->g_data);
		fclose(fp);
		return 1;
		}

	
	/* done */
	return 0;
	}






/* read_pixels: This function takes an image structure and a file handle,
	it then calls an appropriate function to read the pixel data according to
	the nature of the image */

int read_pixels (image *im, FILE *fp) {
	/* Check if the image is RGB. If so, call appropriate function to read the 
	RGB pixel data. Otherwise, call the function to read the greyscale data */

	if (im->is_rgb) {
		return read_rgb_pixels(im,fp);
		}
	else {
		return read_grey_pixels(im,fp);
		}
	}




/* read_grey_pixels: This function takes an image structure and a file pointer,
	and reads the pixel data into the structure assuming that the image is NOT
	indexed and is 8 bit encoded */

int read_grey_pixels (image *im, FILE *fp) {

	
	int i,j;
	int bytes = im->h.bits/8;		// should be 1
	unsigned char temp;
	/* calculate the paddin in the image rows. bitmaps are rounded to be multiples of
	32 bits per row */
	int pad = (4 - ((im->h.width * bytes) % 4)) % 4;
	
	if (bytes != 1) {
		printf("Unsupported greyscale image with %d bit encoding\n",im->h.bits);
		return 1;
		}

	/* Now we have the header, and the colour palette, so now we can read the pixel data.
	Set the pointer appropriately using the header information and then start reading */
	if (fseek(fp, im->h.offset, SEEK_SET) ) {
		printf("ERROR: Could not locate image data\n");
		}

	for (i=im->h.height-1; i >= 0; i--) {
		for (j=0; j < im->h.width; j++) {
			fread(&temp, 1, bytes, fp);
			im->g_data[i][j] = (float) temp;
			}
		fread(&temp, 1, pad, fp);
		}
	}



/* read_rgb_pixels: This function is to read rgb pixel data from the image. It 
	accepts the file handle and the image structure and reads the rgb pixel
	data

	NOTE: We can handle 8 bit indexed image which can be greyscale (RGB having
	same value) or coloured (RGB having different values in the palette). OR
	we can handle 24 bit RGB image where each byte is R, G, and B.

	WE DO NOT HANDLE 24 BIT INDEXED BMP IMAGES. I just think that should not happen.
	there is no sense in having a colour palette if you have 24 bit encoding. This
	kind of image will be simply read as an 24 but image ignoring the palette */



int read_rgb_pixels(image *im,FILE *fp) {
	
	/* The reading style will change depending on whether the image is indexed
	of whether it is 24 bit RGB */
	int i,j;
	int bytes = im->h.bits/8;
	/* calculate the paddin in the image rows. bitmaps are rounded to be multiples of
	32 bits per row */
	int pad = (4 - ((im->h.width * bytes) % 4)) % 4;
	printf("Image is padded with %d bytes\n",pad);

	/* Following procedure is to read the indexed data */
	if (im->is_indexed == 1) {
		if (read_colour_palette(im,fp)) {
			return 1;
			}

	/* Now we have the header, and the colour palette, so now we can read the pixel data.
	Set the pointer appropriately using the header information and then start reading */
	if (fseek(fp, im->h.offset, SEEK_SET) ) {
		printf("ERROR: Could not locate image data\n");
		}

		for (i=im->h.height-1; i >= 0; i--) {
			int temp=0;
			for (j=0; j < im->h.width; j++) {
				temp=0;
				fread(&temp, 1, bytes, fp);
				im->c_data[i][j].r = im->c_index[temp].r;
				im->c_data[i][j].g = im->c_index[temp].g;
				im->c_data[i][j].b = im->c_index[temp].b;
				}
			fread(&temp, 1, pad, fp);
			}
		}
	else {
	/* Else the following procedure is to read 24 bit RGB data with no 
	colour palette */
		
	if (fseek(fp, im->h.offset, SEEK_SET) ) {
		printf("ERROR: Could not locate image data\n");
		}

		for (i=im->h.height-1; i >= 0; i--) {
			unsigned char temp;
			for (j=0; j < im->h.width; j++) {
				fread(&im->c_data[i][j].r, 1, 1, fp);
				fread(&im->c_data[i][j].g, 1, 1, fp);
				fread(&im->c_data[i][j].b, 1, 1, fp);
				}
			fread(&temp, 1, pad, fp);
			}
		}

	return 0;
	}




/* read_colour_palette: This function accepts an image structure and a file
	pointer and then allocates and populates the colour palette. 
	The file handle is assumed to point at the colour index */

int read_colour_palette (image *im, FILE *fp) {

	unsigned char trash;			// this is to hold the junk in colour palette
	int i;

	/* allocate the memory for colour palette */
	im->c_index = (colour *) malloc (sizeof(colour) * im->h.num_c);
	if (im->c_index == NULL) {

		/* clean up if we are not able to prepare the index. No point in continuing 
		because we will probably return wrong pixel data */
		fclose(fp);
		printf("ERROR: Could not create colour index\n");
		return 1;
		}


	/* Each colour is represented by 4 bytes, each representing blue, green and red
	in that order followed by a 0x00. */
	for (i=0; i < im->h.num_c; i++) {
		fread(&im->c_index[i].b, 1, 1, fp);
		fread(&im->c_index[i].g, 1, 1, fp);
		fread(&im->c_index[i].r, 1, 1, fp);
		fread(&trash, 1, 1, fp);
		}

	return 0;
	}




/* allocate_data_array: This function allocates the memory for pixel data. It allocates
	float data if the image is not an RGB (!24_bit && !indexed). Otherwise it
	allocates the colour vectors for each pixel. */

int allocate_data_array (image *im) {

	/* allocate memory for pixel data. This is pointer to pointers. Depending
	on whether the image is RGB or Grey, we need to allocate float or colour vector */

	if (im->is_rgb == 1)
		im->c_data = (colour **) malloc (sizeof(colour *) * im->h.height );
	else
		im->g_data = (float **) malloc (sizeof(float *) * im->h.height );

	if (im->g_data == NULL && im->c_data == NULL) {
		printf("ERROR: Could not allocate memory for the image data\n");
		return 1;
		}

	int i;
	for (i=0; i < im->h.height; i++ ) { 
		/* Using calloc here to initialize the pixels to 0. just in case
		if there is some problem with header and we find less number of 
		pixel data than we should */
		int flag=0;
		if (im->is_rgb == 1) {
			im->c_data[i] = (colour *) malloc ( sizeof(colour) * im->h.width );
			flag = im->c_data[i] == NULL ? 1 : 0 ;
			}
		else {
			im->g_data[i] = (float *) calloc( im->h.width, sizeof(float) );
			flag = im->c_data[i] == NULL ? 1 : 0 ;
			}

		if (flag == 1) {
			printf("ERROR: Could not allocate memory for the image data row\n");
			free(im->c_data);
			free(im->g_data);
			return 1;
			}
		}

	return 0;
	}



/* read_header: This function takes a pointer to a bmp header structure and 
	a file handle of a bmp file opened for reading. It then populates the
	bmp header structure with header fields. 
	Returns 0 on success and 1 on failure */

int read_header(header * h, FILE * fp) {
	
	/* All the fields in the header field are int. This is because, on some platforms
	the width of the datatype such as short, unsigned etc. might vary. Using int means
	we will be able to hold all the data, which is maximum 4 bytes in each field.
	But, this means we have to set all the fields to 0 because we are reading variable
	length data in int which is probably bigger. We should make sure that there is no
	stray garbage in them */

	h->type = 0; fread(&h->type, 2, 1, fp);				// 2 bytes having 'BM'
	h->size = 0; fread(&h->size, 4, 1, fp);				// 4 bytes having size of BMP file
	h->reserved = 0; fread(&h->reserved, 4, 1, fp);		// 4 reserved bytes
	h->offset = 0; fread(&h->offset, 4, 1, fp);			// 4 byte offset where the data of the image file starts

	h->hsize = 0; fread(&h->hsize, 4, 1, fp);			// 4 byte size of this header
	h->width = 0; fread(&h->width, 4, 1, fp);			// 4 byte width of the image
	h->height = 0; fread(&h->height, 4, 1, fp);			// 4 byte height of the image
	h->c_planes = 0; fread(&h->c_planes, 2, 1, fp);		// 2 bytes number of colourplanes being used. Must be 1
	h->bits=0; fread(&h->bits, 2, 1, fp);				// 2 bytes specifying number of bits per pixel. We are converting it to bytes

	h->comp = 0; fread(&h->comp, 4, 1, fp);				// 4 bytes specifying compression method used
	h->imsize = 0; fread(&h->imsize, 4, 1, fp);			// 4 bytes specifying raw image data size
	h->hres = 0; fread(&h->hres, 4, 1, fp);				// 4 bytes specifying horizontal resolution pixels/meter
	h->vres = 0; fread(&h->vres, 4, 1, fp);				// 4 bytes specifying vertical resolution pixels/meter
	h->num_c = 0; fread(&h->num_c, 4, 1, fp);			// 4 bytes specifying number of colours in colour palette
	h->imp_c = 0; fread(&h->imp_c, 4, 1, fp);			// 4 bytes specifying number of important colours. 0 when all are imp. Generally ignored.

	return 0;
	}


/* binarize: This function takes an image and a floating point threshold
	value. It then converts the image to binary such that pixels having
	value greater than or equal to the threshold will be 1 and those below
	threshold will be 0. */

void binarize(image * im,float t) {
	int i,j;
	if (im->is_rgb == 1) {
		printf("ERROR: Image binarization should be done on greyscale images.\n");
		return;
		}

	if (t > im->max_val) {
		printf("WARNING: Threshold for binarization is greater than maximum pixel value\n\
		The image will be entirely white after the operations\n");
		}
	else if (t < im->min_val) {
		printf("WARNING: Threshold for binarization is lesser than minimum pixel value\n\
		The image will be entirely black after the operations\n");
		}
	

	for (i=0; i < im->h.height; i++) {
		for (j=0; j < im->h.width; j++) {
			im->g_data[i][j] = im->g_data[i][j] >= t ? 1.0 : 0.0;
			}
		}
	}
