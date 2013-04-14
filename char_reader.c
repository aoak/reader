#include <stdio.h>
#include <stdlib.h>
#include "image.h"
#include "neural.h"




image im;
ann n;



void main (int argc, char ** argv) {

	
	int nnum[] = {30, 26};
	initialize_ann(&n,0.018, 2, 46*46,nnum);
	train();
	print_ann(&n);

	
/*	binarize(&im,100);
	if (get_image_vector(&im,n.in))
		exit(1);
	*/
	float ** cov;
	cov = calculate_cov(&im);
	int i,j;
	for (i=0; i < im.h.height; i++) {
		for (j=0; j < im.h.height; j++)
			printf("%.3d ",(int)cov[i][j]);
		printf("\n");
		}

	char imname[] = "zc.bmp";
	printf("Reading image %s\n",imname);
	imread(imname,&im);
	colour_to_grey(&im,'A');
	binarize(&im,100);
	get_image_vector(&im,n.in);
	fwd_propogation (&n);
	printf(" ----- END -----\n");
	for (i=0; i < 26; i++)
		printf("%d ",(int) n.outputs[1][i]);
	printf("\n");
	free_image(&im);
	}




train () {
	int i,j;
	int max_sessions = 1000;
	char imname1[] = "za.bmp";
	char imname2[] = "zb.bmp";
	char imname3[] = "zc.bmp";
	char* names[] = {imname1, imname2, imname3};
	char *imname;

	for (i=0; i < max_sessions; i++) {
		for (j=0; j < 26; j++) 
			n.ex_output[j] = 0;
				
		int x= i%3;
		n.ex_output[x]=1;
		imname = names[x];
		imread(imname, &im);
		colour_to_grey(&im,'A');
		binarize(&im,100);
		get_image_vector(&im,n.in);
		fwd_propogation (&n);
		err_backpropogation (&n);
		free_image(&im);
		}
	}
