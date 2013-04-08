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

	
	int i,j;

	/* allocate memory for pixel data. This is pointer to pointers */

	im->data = (float **) malloc (sizeof(float *) * im->h.height );

	if (im->data == NULL) {
		printf("ERROR: Could not allocate memory for the image data\n");
		fclose(fp);
		return 1;
		}


	for (i=0; i < im->h.height; i++ ) { 
		/* Using calloc here to initialize the pixels to 0. just in case
		if there is some problem with header and we find less number of 
		pixel data than we should */
		im->data[i] = (float *) calloc( im->h.width, sizeof(float) );
		if (im->data[i] == NULL) {
			printf("ERROR: Could not allocate memory for the image data row\n");
			fclose(fp);
			free(im->data);
			return 1;
			}
		}


	// now check if this is index bmp. If so, we have to allocate and populate the index.
	if (im->h.num_c) {
		im->is_indexed = 1;				// set the flag indicating the image is indexed
		unsigned char trash;			// this is to hold the junk in colour palette

		/* allocate the memory for colour palette */
		im->c_index = (index_ele *) malloc (sizeof(index_ele) * im->h.num_c);
		if (im->c_index == NULL) {

			/* clean up if we are not able to prepare the index. No point in continuing 
			because we will probably return wrong pixel data */
			fclose(fp);
			for (i=0; i < im->h.height; i++ )
				free(im->data[i]);
			free(im->data);
			printf("ERROR: Could not create colour index\n");
			}


		/* Each colour is represented by 4 bytes, each representing blue, green and red
		in that order followed by a 0x00. */
		for (i=0; i < im->h.num_c; i++) {
			fread(&im->c_index[i].b, 1, 1, fp);
			fread(&im->c_index[i].g, 1, 1, fp);
			fread(&im->c_index[i].r, 1, 1, fp);
			fread(&trash, 1, 1, fp);
//	dbg		printf("index %3d = %3d %3d %3d %3d\n",i,im->c_index[i].b,im->c_index[i].g,im->c_index[i].r,trash);
			}
		}


	/* Now we have the header, and the colour palette, so now we can read the pixel data.
	Set the pointer appropriately using the header information and then start reading */
	if (fseek(fp, im->h.offset, SEEK_SET) ) {
		printf("ERROR: Could not locate image data\n");
		fclose(fp);
		for (i=0; i < im->h.height; i++ )
			free(im->data[i]);
		free(im->data);
		return 1;
		}



	for (i=im->h.height-1; i >= 0; i--) {
		for (j=0; j < im->h.width; j++) {
			
			int foo=0;
			fread(&foo, im->h.bits/8, 1, fp);

			/* If the image is indexed, find the appropriate intensity value.
			otherwise, just set the pixel value to read value */
			if (im->is_indexed == 1) {
				im->data[i][j] = (float) im->c_index[foo].b;
				}
			else {
				im->data[i][j] = (float) foo;
				}
			}
		}
	
	/* done */
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


