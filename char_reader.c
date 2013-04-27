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


#define TRAINING_DATA 46
#define MAX_NAME_LEN 20
#define TRAINING_SESSIONS 7000

void parse_supervisor_data(char * charnames[],int charresults[]);
void train (char * charnames[],int charresults[]);
void unit_test(char * charnames[],int charresults[]);


image im;
ann n;


void main (int argc, char ** argv) {

	int nnum[] = {30, 26};						// two layers, each having 30 and 26 neurons
	int i,j;
	char * charnames[TRAINING_DATA];
	int charresults[TRAINING_DATA];

	initialize_ann(&n,0.018, 2, 46*46,nnum);

	for (i=0; i < TRAINING_DATA; i++) {
		charnames[i] = (char *) malloc (sizeof(char) * MAX_NAME_LEN);
		if (charnames[i] == NULL) {
			printf("Omg\n");
			exit(0);
			}
		}
	train(charnames,charresults);
	unit_test(charnames,charresults);
//	print_ann(&n);


/*	char testname[MAX_NAME_LEN] = {0};
	char choice;
	
	do {
		printf("Enter the image file name: ");
		scanf("%s",testname);

		printf("Reading image %s\n",testname);
		imread(testname,&im);
		colour_to_grey(&im,'A');
		binarize(&im,100);
		get_image_vector(&im,n.in);
		fwd_propogation (&n);
		for (i=0; i < 26; i++ ) {
			j = (int) n.outputs[1][i];
			if (j != 0)
				printf("%1d ",i);
			}
		printf("\n");

		free_image(&im);

		printf("Want to test another fruit? (y/n): ");
		getc(stdin);
		choice = getc(stdin);

		} while ( choice == 'y' || choice == 'Y' );*/

	}




void train (char * charnames[],int charresults[]) {
	int i,j;
	int max_sessions = TRAINING_SESSIONS;
	char *imname;
	
	parse_supervisor_data(charnames,charresults);

	for (i=0; i < max_sessions; i++) {
		for (j=0; j < 26; j++) 
			n.ex_output[j] = 0;
				
		int x= i % TRAINING_DATA;
		n.ex_output[charresults[x]]=1;
		imname = charnames[x];
		imread(imname, &im);
		colour_to_grey(&im,'A');
		binarize(&im,120);
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







void unit_test(char * charnames[],int charresults[]) {

	char * imname;
	int i,j;

	printf("Starting unit tests\n");
	for (i=0; i < TRAINING_DATA; i++) {
		imname = charnames[i];
		imread(imname, &im);
		colour_to_grey(&im,'A');
		binarize(&im,120);
		get_image_vector(&im,n.in);
		fwd_propogation (&n);
		printf("Test image name: %s, expected result: %d, Actual result: ",imname,charresults[i]);
		for (j=0; j < 26; j++ ) {
			if ( ((int) n.outputs[1][j]) != 0)
				printf("%1d ",j);
			}
		printf("\n");
		}
	}



