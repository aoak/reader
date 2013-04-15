/* char_reader: This is the program that trains a neural network for
	recognizing characters. It then prompts the user to enter the 
	input character image name, and tries to recognize it.

	The character files and the expected results are read from supervisor.txt
	The neural network is a multilayer perceptron network trained using a 
	back propagation algorithm.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"
#include "neural.h"


#define TRAINING_DATA 10
#define MAX_NAME_LEN 20

void parse_supervisor_data(char * charnames[],int charresults[]);
void train();


image im;
ann n;



void main (int argc, char ** argv) {

	int nnum[] = {30, 26};						// two layers, each having 30 and 26 neurons
	int i;

	initialize_ann(&n,0.018, 2, 46*46,nnum);
	train();
//	print_ann(&n);

	

	char testname[MAX_NAME_LEN] = {0};
	char choice;
	
	do {
		printf("Enter the image file name:\n");
		scanf("%s",testname);

		printf("Reading image %s\n",testname);
		imread(testname,&im);
		colour_to_grey(&im,'A');
		binarize(&im,100);
		get_image_vector(&im,n.in);
		fwd_propogation (&n);
		for (i=0; i < 26; i++ ) 
			printf("%1d ", (int) n.outputs[1][i]);
		printf("\n");

		free_image(&im);

		printf("Want to test another fruit? (y/n): ");
		getc(stdin);
		choice = getc(stdin);

		} while ( choice == 'y' || choice == 'Y' );

	}




void train () {
	int i,j;
	int max_sessions = 1000;
	char * charnames[TRAINING_DATA];
	int charresults[TRAINING_DATA];
	char *imname;

	for (i=0; i < TRAINING_DATA; i++) {
		charnames[i] = (char *) malloc (sizeof(char) * MAX_NAME_LEN);
		if (charnames[i] == NULL) {
			printf("Omg\n");
			exit(0);
			}
		}
	
	parse_supervisor_data(charnames,charresults);

	for (i=0; i < max_sessions; i++) {
		for (j=0; j < 26; j++) 
			n.ex_output[j] = 0;
				
		int x= i % TRAINING_DATA;
		n.ex_output[charresults[x]]=1;
		imname = charnames[x];
		imread(imname, &im);
		colour_to_grey(&im,'A');
		binarize(&im,100);
		get_image_vector(&im,n.in);
		fwd_propogation (&n);
		err_backpropogation (&n);
		free_image(&im);
		}
	}





void parse_supervisor_data(char * charnames[],int charresults[]) {
	char filename[] = "supervisor.txt";
	FILE * fp;
	int i;

	fp = fopen(filename,"r");
	if (fp == NULL) {
		printf("Error opening %s\n",filename);
		exit(0);
		}
	
	for (i=0; i < TRAINING_DATA; i++)
		fscanf(fp,"%s %d\n",charnames[i],&charresults[i]);

	fclose(fp);
	}










