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


#define TRAINING_DATA 52
#define MAX_NAME_LEN 20
#define TRAINING_SESSIONS 300
#define TEST_DATA 24

void parse_supervisor_data(char * charnames[],int charresults[]);
void train (char * charnames[],int charresults[]);
void unit_test(char * charnames[],int charresults[]);
void test ();


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
//	test();
//	print_ann(&n);


	char testname[MAX_NAME_LEN] = {0};
	char choice;

	printf("Do you want to test some more images? (y/n): ");
	choice = getc(stdin);
	if (choice != 'y' && choice != 'Y')
		return;
	
	do {
		printf("Enter the image file name: ");
		scanf("%s",testname);

		printf("Reading image %s\n",testname);
		imread(testname,&im);
		colour_to_grey(&im,'A');
		binarize(&im,120);
		get_image_vector(&im,n.in);
		fwd_propogation (&n);
		for (i=0; i < 26; i++ ) {
			j = (int) n.outputs[1][i];
			if (j != 0)
				printf("%c ",i+'A');
			}
		printf("\n");

		free_image(&im);

		printf("Want to test another character? (y/n): ");
		getc(stdin);
		choice = getc(stdin);

		} while ( choice == 'y' || choice == 'Y' );

	free_ann(&n);
	}




void train (char * charnames[],int charresults[]) {
	int i,j,k;
	int max_sessions = TRAINING_SESSIONS;
	char *imname;
	int err = 0;
	
	parse_supervisor_data(charnames,charresults);

	for (i=0; i < max_sessions; i++) {
		err=0;
		for (k=0; k < TRAINING_DATA; k++) {
			for (j=0; j < 26; j++) 
				n.ex_output[j] = 0;
			
			n.ex_output[charresults[k]]=1;
			imname = charnames[k];
			imread(imname, &im);
			colour_to_grey(&im,'A');
			binarize(&im,120);
			get_image_vector(&im,n.in);
			fwd_propogation (&n);
			for (j=0; j < 26; j++) {
				if (n.outputs[1][j] != n.ex_output[j])
					err++;
				}
			err_backpropogation (&n);
			free_image(&im);
			}
//		printf("session %d: error %d\n",i,err);		
		}
	}





void parse_supervisor_data(char * charnames[],int charresults[]) {
	char filename[] = "supervisor.txt";
	FILE * fp;
	int i;
	char foo;

	fp = fopen(filename,"r");
	if (fp == NULL) {
		printf("Error opening %s\n",filename);
		printf("File %s needs to be present in the current directory with the details of training samples and expected outputs\n",filename);
		exit(0);
		}
	
	for (i=0; i < TRAINING_DATA; i++) {
		fscanf(fp,"%s %c\n",charnames[i],&foo);
		charresults[i] = foo - 'A';
		}

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
		printf("Test image name: %s, expected result: %c, Actual result: ",imname,charresults[i]+'A');
		for (j=0; j < 26; j++ ) {
			if ( ((int) n.outputs[1][j]) != 0)
				printf("%c ",j+'A');
			}
		printf("\n");
		}
	}



void test () {

	char testlist[] = "test.txt";
	char testname[MAX_NAME_LEN];
	int i,j,k;
	FILE * fp;

	fp = fopen(testlist,"r");
	if (fp == NULL) {
		printf("Error opening %s\n",testlist);
		exit(0);
		}
	
	for (i=0; i < TEST_DATA; i++) {
		fscanf(fp,"%s\n",testname);
		imread(testname,&im);
		colour_to_grey(&im,'A');
		binarize(&im,120);
		get_image_vector(&im,n.in);
		fwd_propogation (&n);
		printf("Test image name: %s, Actual result: ",testname);
		for (j=0; j < 26; j++ ) {
			if ( ((int) n.outputs[1][j]) != 0)
				printf("%c ",j+'A');
			}
		printf("\n");

		free_image(&im);
		}
	
	fclose(fp);
	}
