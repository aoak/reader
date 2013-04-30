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
	} colour;
	

/* The structure for bmp image itself.
	This structure has the double pointer to image pixel data. This is floating
	point data.
	The structure also contains a header for this bmp image file. (See structure 
	for bmp file header above.
	Along with the header and pixel data, the image structure has a pointer to
	colour palette elements. This pointer points to array of palette colours if
	the image is indexed.
	Whether the image is indexed is shown by field 'is_indexed' which is set to 1
	if the image is indexed 
	fields min_val and max_val are minimum and maximum pixel value of the image data
	*/

typedef struct {
	float ** g_data;				// pixel data of greyscale image
	colour ** c_data;				// pixel data of colour image
	header h;						// bmp header structure
	colour * c_index;				// pointer to the colour palette
	int is_indexed;					// 'is the image indexed?' flag
	int is_rgb;						// 'is the image rgb' flag
	float max_val, min_val;			// minimum and maximum value of pixels
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

/* binarize: This function takes an image and a floating point threshold
	value. It then converts the image to binary such that pixels having
	value greater than or equal to the threshold will be 1 and those below
	threshold will be 0. */

void binarize(image *,float);


/* read_pixels: This function takes an image structure and a file handle,
	it then calls an appropriate function to read the pixel data according to
	the nature of the image */

int read_pixels (image *, FILE *);

/* read_grey_pixels: This function takes an image structure and a file pointer,
	and reads the pixel data into the structure assuming that the image is NOT
	indexed and is 8 bit encoded */

int read_grey_pixels (image *, FILE *);



/* read_rgb_pixels: This function is to read rgb pixel data from the image. It 
	accepts the file handle and the image structure and reads the rgb pixel
	data

	NOTE: We can handle 8 bit indexed image which can be greyscale (RGB having
	same value) or coloured (RGB having different values in the palette). OR
	we can handle 24 bit RGB image where each byte is R, G, and B.

	WE DO NOT HANDLE 24 BIT INDEXED BMP IMAGES. I just think that should not happen.
	there is no sense in having a colour palette if you have 24 bit encoding. This
	kind of image will be simply read as an 24 but image ignoring the palette */



int read_rgb_pixels(image *,FILE *);




/* read_colour_palette: This function accepts an image structure and a file
	pointer and then allocates and populates the colour palette. 
	The file handle is assumed to point at the colour index */

int read_colour_palette (image *, FILE *);


/* allocate_data_array: This function allocates the memory for pixel data. It allocates
	float data if the image is not an RGB (!24_bit && !indexed). Otherwise it
	allocates the colour vectors for each pixel. */

int allocate_data_array (image *);


/* colour_to_grey: This function takes an RGB image and converts it to grey scale.
	returns 1 on failure and 0 on success.
	Conversion takes place according to one of the 3 methods
	1. 'L': Luminosity- weighted averaging of the pixels. (default)
	2. 'A': Averaging- Averaging of the pixels
	3. 'I': lIghtness- Average of the min and max of the colours.

	NOTE: Original image structure is altered. 
		  Also, if the image is indexed image with all R, G and B values same
		  (lame greyscale image) then you probably want to convert it using the
		  averaging method ('A')*/

int colour_to_grey (image *, char );



/* get_image_vector: This function takes an greyscale image and a pointer
	to an array of floats. It then populates the array (vector) by rowvise
	image pixel data.
	NOTE: Assumes enough memory is allocated for the vector. Also assumes 
	the vector to be a float vector */

int get_image_vector (image *, float *);



/* free_image: This function takes an image structure and deallocates all the 
478     memory in its arrays. */

void free_image (image * );




/* calculate_cov: This function calculates the covariance matrix of the given image
	data. Expects a greyscale image.
	Returns pointer to the floating point cov matrix on success or NULL on failure */


float ** calculate_cov (image *);


/* principal_components: This function takes an image structure and returns a triple
    float pointer of which the first is pointer to a eigen value vector and the second
	is the pointer to the matrix whose columns are eigen vectors of the image and hence
	the principal components of the image.

	NOTE: The first pointer is a float ** (2D) pointer whose first dimention is of length 1.
	This is because we need a pointer to a vector (float * - 1D) but the matrix returned
	is a of float ** pointers, hence we had to add this dummy dimention. 
	The eigen values should be accessed like:
			matrix[0][0][i] to access ith eigen value. */



float *** principal_components (image * im);


/* calculate_column_norm: This function takes a vector of floats and number of rows. 
	It then calculates the norm of the given column by using formula 
		norm = sqrt (sum (data[i] ^ 2)) */

double calculate_column_norm (float *, int);



/* qr_decomposition: This function takes a SQUARE matrix and decomposes
	it into two matrices Q and R. A triple pointer is returned. This is
	actually a pointer to two 2D matrices. 
	The method used to calculate the decomposition is Gram–Schmidt process */

float *** qr_decomposition (float **, int, int);


/* project_vector: This function takes two vectors and projects the second vector on
    first and returns the projection vector */
 
float * project_vector (float *, float *, int);


/* schur: This function takes a matrix and number of iterations and returns the 
	matrix' Schur form. Number of iterations refer to the QR algorithm iterations */

float ** schur (float **, int, int, int);



/* matrix_mult: Simple matrix multiplication */

float ** matrix_mult (float **, int, int, float **, int, int);


/* eig_val: This function takes a matrix and computes its eigen values using
	QR algorithm. Returns a vector of eigen values which are digonal of the
	Schur form of input matrix after 50 iterations of QR algorithm */

float * eig_val (float **, int, int);


/* eig_vect_eig_val: This function takes a matrix and an eigen value of that
	matrix. It then finds out the eigen vector corresponding that matrix */

float * eig_vect_eig_val (float **, int, int, float);




/* eig_vect: This function takes a matrix and a vector of eigen values. It then
	returns a matrix whose each column is eigen vector corresponding to that
	particular position in eigen value vector */

float ** eig_vect (float **, int, int, float *, int);




/* eig: This function takes a matrix and returns a eigen structure
	of which the pointer at zeroth location is pointer to a vector
	of eigen values. Pointer at location 1 is pointer to a matrix
	whose columns are eigen vectors corresponding to the eigen value
	at the same index in eigen value vector

    NOTE: The pointer at the first location is a float ** but we actually
    only need float *. As we are returning float ***, it is necessary that
    each element is float **. Hence, while accessing the eigen value vector,
    we have to add a dummy index and access the eigen values like 
    eigen[0][0][i] to access ith eigen value. */


float *** eig (float **, int, int);


#endif
