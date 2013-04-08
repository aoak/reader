/*_____________________________________________________________________________
image.h: This is a header file for giving basic functionality of the image 
	reading and processing.
	The file contains structure definitions of a bmp image, bmp header, and a
	colour in colour paletter. Current functions include following:
		imread:					Read a bmp file in a structure
		read_header:			Read the header of the bmp file in a structure

NOTE: Currently takes into consideration only grayscale bmp files.
_______________________________________________________________________________
Author: 
    Aniket V. Oak
Date Created:
    07-Apr-2013
_______________________________________________________________________________*/


#ifndef _IMAGE_GUARD
#define _IMAGE_GUARD


/* A structure for header of a bmp file
	BMP file header consists of several fields. they are parsed in a 
	structure below. The fields have important parameters like image size,
	pixel data size, rows, columns, number of bits per pixel, offset to the
	image data, number of colours in the palette etc. */

typedef struct {
	int type;			// 'BM'
	int size;			// BMP image file size
	int reserved;		// reserved bytes
	int offset;			// offset where the image data starts

	int hsize;			// size of this header in bytes
	int width;			// image width
	int height;			// image height
	int c_planes;		// colour planes
	int bits;			// bytes per pixel

	int comp;			// compression method used
	int imsize;			// size of the raw image data
	int hres;			// horizontal resolution pixel/meter
	int vres;			// vertical resolution pixel/meter
	int num_c;			// number of colours in colour palette
	int imp_c;			// number of important colours

	} header;


/* A bmp file may have colour index for the colours in the palette.
	This structure is representation of each colour in the palette.
	Note that we discard the forth parameter which is always set to 0. */

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
	} index_ele;

/* The structure for bmp image itself.
	This structure has the double pointer to image pixel data. This is floating
	point data.
	The structure also contains a header for this bmp image file. (See structure 
	for bmp file header above.
	Along with the header and pixel data, the image structure has a pointer to
	colour palette elements. This pointer points to array of palette colours if
	the image is indexed.
	Whether the image is indexed is shown by field 'is_indexed' which is set to 1
	if the image is indexed */

typedef struct {
	float ** data;
	header h;
	index_ele * c_index;
	int is_indexed;
	} image;


/* imread: This function takes a string and a pointer to image structure. 
	It then opens the image named the string (assumes it is a bmp image) and 
	populates the image structure with pixel data, bmp header and index of
	colours (if any). 
	Returns 0 on success and 1 on failure */

int imread(char *, image *);


/* read_header: This function takes a pointer to a bmp header structure and 
	a file handle of a bmp file opened for reading. It then populates the
	bmp header structure with header fields. 
	Returns 0 on success and 1 on failure */

int read_header(header *, FILE *);





#endif
