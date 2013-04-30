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
#include <errno.h>
#include <math.h>

float luminosity (colour c);
float lightness (colour c);
float averaging (colour c);

/* imread: This function takes a string and a pointer to image structure. 
	It then opens the image named the string (assumes it is a bmp image) and 
	populates the image structure with pixel data, bmp header and index of
	colours (if any). 
	Returns 0 on success and 1 on failure */

int imread (char * imname, image * im) {

	if (imname == NULL) {
		fprintf(stderr,"ERROR: image name not specified %p\n",imname);
		return 1;
		}
	
	im->is_indexed = 0;					// set the indexed field to 0 for start
	im->max_val = im->min_val = 0;		// set min and max values to be 0
	im->is_rgb=0;						// set the rgb flag to 0 for now
	

	FILE * fp;

	fp = fopen(imname,"rb+");			// now open the image file
	if (fp == NULL) {
		fprintf(stderr,"ERROR %d: Reading the image file: %s failed\n",errno,imname);
		return 1;
		}
	
	/* Now read the bmp file header into the bmp header structure */
	if (read_header(&im->h, fp)) {
		fprintf(stderr,"ERROR: Reading the header failed\n");
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
		fprintf(stderr,"ERROR: Error allocating data for image pixels\n");
		fclose(fp);
		return 1;
		}
	
	if (read_pixels(im,fp)) {
		fprintf(stderr,"ERROR: Error reading pixel data\n");
		free(im->c_data);
		free(im->g_data);
		fclose(fp);
		return 1;
		}

	fclose(fp);	
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
	im->max_val=0;
	im->min_val=255;
	int bytes = im->h.bits/8;		// should be 1
	unsigned char temp;
	/* calculate the paddin in the image rows. bitmaps are rounded to be multiples of
	32 bits per row */
	int pad = (4 - ((im->h.width * bytes) % 4)) % 4;
	
	if (bytes != 1) {
		fprintf(stderr,"Unsupported greyscale image with %d bit encoding\n",im->h.bits);
		return 1;
		}

	/* Now we have the header, and the colour palette, so now we can read the pixel data.
	Set the pointer appropriately using the header information and then start reading */
	if (fseek(fp, im->h.offset, SEEK_SET) )
		fprintf(stderr,"ERROR: Could not locate image data\n");
		

	for (i=im->h.height-1; i >= 0; i--) {
		for (j=0; j < im->h.width; j++) {
			fread(&temp, 1, bytes, fp);
			im->g_data[i][j] = (float) temp;
			im->max_val = temp > im->max_val ? temp : im->max_val;
			im->min_val = temp < im->min_val ? temp : im->min_val;
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
	//	printf("Image is padded with %d bytes\n",pad);

	/* Following procedure is to read the indexed data */
	if (im->is_indexed == 1) {
		if (read_colour_palette(im,fp)) {
			return 1;
			}

	/* Now we have the header, and the colour palette, so now we can read the pixel data.
	Set the pointer appropriately using the header information and then start reading */
	if (fseek(fp, im->h.offset, SEEK_SET) ) {
		fprintf(stderr,"ERROR: Could not locate image data\n");
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
		fprintf(stderr,"ERROR: Could not locate image data\n");
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
		fprintf(stderr,"ERROR: Could not create colour index\n");
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
		fprintf(stderr,"ERROR: Could not allocate memory for the image data\n");
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
			fprintf(stderr,"ERROR: Could not allocate memory for the image data row\n");
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
		fprintf(stderr,"ERROR: Image binarization should be done on greyscale images.\n");
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



/* colour_to_grey: This function takes an RGB image and converts it to grey scale.
	returns 1 on failure and 0 on success.
	Conversion takes place according to one of the 3 methods
	1. 'L': Luminosity- weighted averaging of the pixels. (default)
	2. 'A': Averaging- Averaging of the pixels
	3. 'I': lIghtness- Average of the min and max of the colours.

	NOTE: Original image structure is altered. */

int colour_to_grey (image *im, char method) {

	if (im->is_rgb == 0) {
		return 1;
		}
	
	float (* convert)();

	im->max_val = 0;
	im->min_val = 255;

	switch (method) {
		case 'L': 	convert = luminosity;
					break;
		case 'A':	convert = averaging;
					break;
		case 'I':	convert = lightness;
					break;
	default:	convert = luminosity;
					break;
		}
	
	// first set the RGB flag to 0 and get a new memory allocated to the image data
	im->is_rgb = 0;
	allocate_data_array(im);

	int i,j;
	for (i=0; i < im->h.height; i++)
		for (j=0; j < im->h.width; j++) {
			im->g_data[i][j] = convert(im->c_data[i][j]);
			im->max_val = im->g_data[i][j] > im->max_val ? im->g_data[i][j] : im->max_val;
			im->min_val = im->g_data[i][j] < im->min_val ? im->g_data[i][j] : im->min_val;
			}

	
	/* If we are here, we can now safely deallocate the colour data */
	for (i=0; i < im->h.height; i++)
		free(im->c_data[i]);
	free(im->c_data);

	im->is_indexed=0;
	return 0;
	}




float averaging (colour c) {
	return ((float) (c.r + c.b + c.g)) / 3;
	}



float lightness (colour c) {
	int max,min;
	
	max = (c.r > c.b) ? c.r : c.b;
	max = (max > c.g) ? max : c.g;

	min = (c.r < c.b) ? c.r : c.b;
	min = (min < c.g) ? min : c.g;

	return ((float) (max + min)) / 2;
	}


float luminosity (colour c) {
	/* 0.21 R + 0.71 G + 0.07 B is the formula. This is because humans are
	more sensitive to green colour */
	return ((0.21 * c.r + 0.71 * c.b + 0.07 * c.g) / 3);
	}




/* get_image_vector: This function takes an greyscale image and a pointer
	to an array of floats. It then populates the array (vector) by rowvise
	image pixel data.
	NOTE: Assumes enough memory is allocated for the vector. Also assumes 
	the vector to be a float vector */

int get_image_vector (image *im, float *vect) {
	
	if (im->is_indexed != 0 || im->is_rgb != 0) {
		fprintf(stderr,"Greyscale image expected for conversion into a vector\n");
		return 1;
		}
	
	int i,j,ind = 0;
	for (i=0; i < im->h.height; i++)
		for (j=0; j < im->h.width; j++)
			vect[ind++] = im->g_data[i][j];
	
	return 0;
	}






/* free_image: This function takes an image structure and deallocates all the 
	memory in its arrays. */

void free_image (image * im) {
	
	if (im->c_index)
		free(im->c_index);				// free colour palette if any
	
	int i;
	for (i=0; i < im->h.height; i++) {
		if (im->is_indexed || im->is_rgb)
			free(im->c_data[i]);
		else
			free(im->g_data[i]);
		}
	
	im->is_indexed = -1;
	im->is_rgb = -1;
	im->h.height = 0;
	im->h.width = 0;
	}






/* calculate_cov: This function calculates the covariance matrix of the given image
	data. Expects a greyscale image.
	Returns pointer to the floating point cov matrix on success or NULL on failure */


float ** calculate_cov (image *im) {

	if (im->is_indexed == 1 || im->is_rgb == 1) {
		fprintf(stderr,"ERROR: Expected input for covariance calculation is a greyscale image\n");
		return NULL;
		}
	
	int i,j,k;
	int m=im->h.height;
	int n=im->h.width;
	float ** cov;
	
	cov = (float **) malloc (sizeof(float *) * n);
	if (cov == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for covariance matrix failed\n");
		return NULL;
		}
	
	for (i=0; i < n; i++) {
		cov[i] = (float *) malloc (sizeof(float) * n);
		if (cov == NULL) {
			fprintf(stderr,"ERROR: Allocating memory for covariance matrix failed\n");
			return NULL;
			}
		}
	
	float mean[n];
	float sum=0;
	for (i=0; i < n; i++) {
		for (j=0; j < m; j++)
			sum += im->g_data[j][i];
		mean[i] = sum / m;
		sum = 0;
		}
			
	for (i=0; i < n; i++)
		for (j=0; j < n; j++) {
			cov[i][j] = 0;
			for (k=0; k < m; k++)
				//cov[i][j] += (im->g_data[k][i] - mean[i]) * (im->g_data[k][j] - mean[j]);
				cov[i][j] += (im->g_data[i][k] - mean[k]) * (im->g_data[j][k] - mean[k]);
			cov[i][j] /= (m);
			//cov[i][j] /= (m-1);
			}
	return cov;
	}





/* principal_components: This function takes an image structure and returns a triple
	float pointer of which the first is pointer to a eigen value vector and the second
	is the pointer to the matrix whose columns are eigen vectors of the image and hence
	the principal components of the image.

	NOTE: The first pointer is a float ** (2D) pointer whose first dimention is of length 1.
		This is because we need a pointer to a vector (float * - 1D) but the matrix returned
		is a of float ** pointers, hence we had to add this dummy dimention. 
		The eigen values should be accessed like:
			matrix[0][0][i] to access ith eigen value. */



float *** principal_components (image * im) {
	
	float ** cov;
	cov = calculate_cov(im);
	if (cov == NULL) {
		fprintf(stderr,"ERROR: Calculating covariance matrix of the image failed.\n");
		return NULL;
		}
	
	return eig(cov, im->h.width, im->h.width);
	}












/* qr_decomposition: This function takes a SQUARE matrix and decomposes
	it into two matrices Q and R. A triple pointer is returned. This is
	actually a pointer to two 2D matrices. 
	The method used to calculate the decomposition is Gram–Schmidt process */



float *** qr_decomposition (float ** mat, int rows, int cols) {

	float ** q,** r;
	int i, j, k;

	float *** qr_mat = (float ***) malloc (sizeof(float **) * 2);
	if (qr_mat == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for decomposition matrices failed\n");
		return NULL;
		}
	
	q = (float **) malloc (sizeof(float *) * rows);
	r = (float **) malloc (sizeof(float *) * rows);
	if (q == NULL || r == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for decompositing matrix failed\n");
		return NULL;
		}
	
	for (i=0; i < rows; i++) {
		q[i] = (float *) malloc (sizeof(float) * cols);
		r[i] = (float *) malloc (sizeof(float) * rows);
		if (q[i] == NULL || r[i] == NULL) {
			fprintf(stderr,"ERROR: Allocating memory for decomposition matrix failed\n");
			return NULL;
			}
		}
	
	/* Set the matrix we are going to return */
	qr_mat[0] = q;
	qr_mat[1] = r;

	/* Q is computed by dividing each column of the image matrix by its norm */
	for (i=0; i < cols; i++) {

		/* First I need a vector in which I can copy the column of original matrix */
		float * of, *on;
		of = (float *) malloc (sizeof(float) * rows);
		on = (float *) malloc (sizeof(float) * rows);
		if (of == NULL || on == NULL) {
			fprintf(stderr,"ERROR: Calculating the projection failed. Out of memory\n");
			return NULL;
			}
		if (i == 0) {
			/* This case is easiest, just find the norm of this vector (column 0)
			   and set first column of Q as vector divided by its norm */
			for (j=0; j < rows; j++) 
				of[j] = mat[j][i];
			
			double norm = calculate_column_norm(of, rows);
			for (j=0; j < rows; j++)
				q[j][i] = mat[j][i] / norm;
			}
		else {
			/* else part is complecated. There are number of steps.
			First we need the vector of which projections are to be calculated */
			for (j=0; j < rows; j++) {
				of[j] = mat[j][i];
				q[j][i] = mat[j][i];
				}

			/* Now we need to calculate projection of this vector wrt each of the previous columns of
			the Q matrix. (We have already divided the column with norm) */
			for (k=0; k < i; k++) {
				float * proj;
				/* Copy the vector on which the current vector is to be projected */
				for (j=0; j < rows; j++)
					on[j] = q[j][k];

				/* Get the projection */
				proj = project_vector(on,of,rows);
				if (proj == NULL) {
					fprintf(stderr,"ERROR: Calculating the projection failed.\n");
					return NULL;
					}

				/* Subtract the projection from the original 'of' vector */
				for (j=0; j < rows; j++)
					q[j][i] -= proj[j];
					
				free(proj);
				}
			/* Now copy this current vector we are operating on, get its norm and
			   divide the vector by its norm */
			for (j=0; j < rows; j++)
				of[j] = q[j][i];
			double norm = calculate_column_norm(of, rows);
			for (j=0; j < rows; j++) 
				q[j][i] /= norm;
			}
		free(on);
		free(of);
		}

	/* Now R is computed by formula Q_transpose * A */

	for (i=0; i < rows; i++)
		for (j=0; j < rows; j++) {
			r[i][j] = 0;
			for (k=0; k < rows; k++)
				r[i][j] += (q[k][i] * mat[k][j]);
			}
	
	return qr_mat;
	}




/* calculate_column_norm: This function takes a vector of floats and number of rows. 
	It then calculates the norm of the given column by using formula 
			norm = sqrt (sum (data[i] ^ 2)) */

double calculate_column_norm (float * d, int r) {

	long double norm = 0;
	int i;

	for (i=0; i < r; i++)
		norm += pow(d[i],2);
	
	return sqrt(norm);
	}


/* project_vector: This function takes two vectors and projects the second vector on
	first and returns the projection vector */

float * project_vector (float *on, float *of, int len) {

	float * p;
	int i;

	p = (float *) malloc (sizeof(float) * len);
	if (p == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for projection vector failed.\n");
		return NULL;
		}

	double num,den;
	for (i=0; i < len; i++) {
		num += on[i] * of[i];
		den += pow(on[i],2);
		}
	for (i=0; i < len; i++) 
		p[i] = (num/den) * on[i];
	
	return p;
	}





/* schur: This function takes a matrix and number of iterations and returns the 
    matrix' Schur form. Number of iterations refer to the QR algorithm iterations */

float ** schur (float ** mat, int rows, int cols, int iterations) {
	
	float *** qr_mat;
	float ** schur_form, ** rq;
	int i,j;

	qr_mat = qr_decomposition(mat,rows,cols);
	if (qr_mat == NULL) {
		fprintf(stderr,"ERROR: QR decomposition failed when %d iterations remaining.\n",iterations);
		return NULL;
		}
	
	rq = matrix_mult(qr_mat[1], rows, cols, qr_mat[0], rows, cols);

	/* Now free the qr_mat which is a float *** */
	for (i=0; i < 2; i++) {
		for (j=0; j < rows; j++)
			free(qr_mat[i][j]);
		free(qr_mat[i]);
		}
	free(qr_mat);



	iterations--;
	if (iterations > 0) {
		schur_form = schur(rq, rows, cols, iterations); 
		for (i=0; i < rows; i++)
			free(rq[i]);
		free(rq);
		}
	else 
		schur_form = rq;

	return schur_form;
	}



/* matrix_mult: Simple matrix multiplication */


float ** matrix_mult (float ** a, int ra, int ca, float ** b, int rb, int cb) {

	float ** result;
	int i, j, k;

	if (ca != rb) {
		fprintf(stderr,"Matrix dimentions do not agree for multiplication");
		return NULL;
		}

	result = (float **) malloc (sizeof(float *) * ra);
	if (result == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for multiplication result failed\n");
		return NULL;
		}
	
	for (i=0; i < ra; i++) {
		result[i] = (float *) malloc (sizeof(float) * cb);
		if (result[i] == NULL) {
			fprintf(stderr,"ERROR: Allocating memory for multiplication result failed\n");
			return NULL;
			}
		}
	
	for (i=0; i < ra; i++)
		for (j=0; j < cb; j++) {
			result[i][j] = 0;
			for (k=0; k < ca; k++)
				result[i][j] += a[i][k] * b[k][j];
			}
	
	return result;
	}







/* eig_val: This function takes a matrix and computes its eigen values using
	QR algorithm. Returns a vector of eigen values which are digonal of the
	Schur form of input matrix after 50 iterations of QR algorithm */

float * eig_val (float ** mat, int rows, int cols) {
	
	int i;
	float * eval;
	eval = (float *) malloc (sizeof(float) * rows);
	if (eval == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for eigen value vector failed.\n");
		return NULL;
		}

	float ** schur_form = schur(mat, rows, cols, 50);


	for (i=0; i < rows; i++)
		eval[i] = schur_form[i][i];

	for (i=0; i < rows; i++)
		free(schur_form[i]);
	free(schur_form);

	return eval;
	}






/* eig_vect_eig_val: This function takes a matrix and an eigen value of that
	matrix. It then finds out the eigen vector corresponding that matrix 
	This is a big function (just like QR decomposition) because I have 
	implemented solving of system of linear equations in this function
	itself. Maybe on later day, it will be good to move all this matrix
	functions to a new library having generic matrix functions. */



float * eig_vect_eig_val (float ** mat, int rows, int cols, float eval) {

	int i,j,k;
	float ** r;
	float * vect;
	int piv_cols[cols], piv_rows[rows];

	/* First try to allocate the memory for local copy of the matrix and the
	   eigen vector. No point doing anything if that fails */

	vect = (float *) malloc (sizeof(float) * cols);
	if (vect == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for eigen vector failed\n");
		return NULL;
		}

	r = (float **) malloc (sizeof(float *) * rows);
	if (r == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for matrix reduction failed\n");
		return NULL;
		}
	
	for (i=0; i < rows; i++) {
		r[i] = (float *) malloc (sizeof(float) * cols);
		if (r[i] == NULL) {
			fprintf(stderr,"ERROR: Allocating memory for matrix reduction failed\n");
			return NULL;
			}
		}
	
	/* first prepare the matrix by doing (A - lamda I) */
	for (i=0; i < rows; i++)
		for (j=0; j < cols; j++)
			if (i == j)
				r[i][j] = mat[i][j] - eval;
			else
				r[i][j] = mat[i][j];

	
	for (i=0; i < cols; i++)
		piv_cols[i] = -1;
	int num_pivots=0;

	for (i=0; i < cols && num_pivots < rows; i++) {
		/* If the element under consideration is 0, then we need to try and find
		   and different pivot for us */
		if (r[num_pivots][i] == 0) {
			/* Our potential pivot turned out to be a 0. So lets try to do a
			   row swap to get out of this situation */
			for (j=num_pivots+1; j < rows; j++) {
				if (r[j][i] != 0)
					break;			// found a non zero potential pivot
				}
			/* If j is equal to rows, that means there is no non zero hero
			   below our potential pivot zero. Nothing we can do about it.
			   just skip this column. This will be a free variable. set it to 1 */
			if (j == rows) {
				vect[i] = 1;
				continue;
				}

			/* row swap using a temp variable. Just like in high school ;) */
			float temp;
			for (k=0; k < cols; k++) {
				temp = r[num_pivots][k];
				r[num_pivots][k] = r[j][k];
				r[j][k] = temp;
				}
			}

		/* If we are here means we got a good pivot, or we got away with row swapping.
			anyway, we can proceed now to eliminate all the elements below our pivot */
		float pivot = r[num_pivots][i];
		piv_cols[i] = 1;

		for (j=i+1; j < rows; j++) {
			float d = r[j][i] / pivot;
			for (k=0; k < cols; k++)
				r[j][k] -= d * r[num_pivots][k];
			}

		/* After this step, We should iterate over the entire matrix and those elements which
		are almost zero, lets make them 0 */
		for (k=0; k < rows; k++) {
			for (j=0; j < cols; j++) {
				if ((int) (r[k][j] * 10) == 0)
					r[k][j] = 0;
				}
			}

		num_pivots++;
		}
	
	/* Now we have the matrix in reduced form. Now we find the eigen vector */
	// printf("Number of pivots for this matrix = %d. Number of rows %d\n",num_pivots,rows);
	
	for (i=num_pivots-1; i >= 0; i--) {
		float sum_free = 0;
		for (j=cols-1; j >= 0; j--) {
			/* The last pivot we encountered will be in row num_pivots-1 */
			if (piv_cols[j] == 1) {
				/* This means there is a pivot in this column */
				vect[j] = (-sum_free)/r[i][j];
				piv_cols[j] = -1;
				break;
				}
			else {
				/* This means this column is a free column */
				sum_free += (r[i][j] * vect[j]);
				}
			}
		}
	

	for (i=0; i < rows; i++)
		free(r[i]);
	free(r);
	return vect;
	}





/* eig_vect: This function takes a matrix and a vector of eigen values. It then
    returns a matrix whose each column is eigen vector corresponding to that
    particular position in eigen value vector */

float ** eig_vect (float ** mat, int rows, int cols, float * eval, int len) {

	int i,j;
	float ** ev;

	ev = (float **) malloc (sizeof(float *) * cols);
	if (ev == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for eigen vector matrix failed\n");
		return NULL;
		}
	
	for (i=0; i < cols; i++) {
		ev[i] = (float *) malloc (sizeof(float) * len);
		if (ev[i] == NULL) {
			fprintf(stderr,"ERROR: Allocating memory for eigen vector matrix failed\n");
			return NULL;
			}
		}
	
	for (i=0; i < len; i++) {
		float * e = eig_vect_eig_val(mat, rows, cols, eval[i]);
		for (j=0; j < cols; j++)
			ev[j][i] = e[j];
		free(e);
		}
	
	return ev;
	}




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

float *** eig (float ** mat, int rows, int cols) {

	float *** eigen;
	eigen = (float ***) malloc (sizeof(float **) * 2);
	if (eigen == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for eigen vector eigen value matrix failed\n");
		return NULL;
		}

	eigen[0] = (float **) malloc (sizeof(float *) * 1);
	if (eigen[0] == NULL) {
		fprintf(stderr,"ERROR: Allocating memory for eigen vector-eigen value matrix failed\n");
		return NULL;
		}
	eigen[0][0] = eig_val(mat, rows, cols);
	if (eigen[0][0] == NULL)
		return NULL;

	eigen[1] = eig_vect(mat, rows, cols, eigen[0][0], cols);
	if (eigen[1] == NULL)
		return NULL;

	return eigen;
	}
