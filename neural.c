/*
neural.c
	This file provides implementations of the prototype functions in the 
	neural.h file.

This file is part of 'reader'

Copyright (C) 2013  Aniket Oak

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/


#include "neural.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>




/* initialize_neuron: This is a function to initialize a neuron. It should take a
   pointer to a neuron, and number of inputs it will have. It should then initialize
   output to 0 and bias to random value and allocate an array for weights
   according to the number of inputs.
  
   NOTE: This does not set the weights of a neuron to particular value. It sets them
		 to random thing. The right function for the former will be set_neuron

   Success will be returned as 1 and failure as 0 */

int initialize_neuron (neuron *n, int num_weights, activation a) {

	if (n == NULL) {	// this shouldnt happen
		printf("Null pointer passed: neuron = %p\n",n);
		return 0;
		}

	n->ex_output = 0.0;	// expected output should be set by the calling program later
	n->output = 0.0;	// set output to 0
	n->bias = 1.0;		// set bias to 0
	
	n->weights = (float *) malloc (sizeof(float) * num_weights );
	
	if (n->weights == NULL) {	// check for malloc failures
		printf("Could not allocate memory for weights");
		return 0;
		}

	int i;
	for (i=0; i < num_weights; i++) {
		n->weights[i] = -1 + ((2* (float)rand())/RAND_MAX);
		}
	n->bias_wt = -1 + ((2* (float)rand())/RAND_MAX);

	n->num_in = num_weights;	// now set the number of inputs
	n->act = a;

	// all well which ends well
	return 1;
	}


/* free_neuron: This function will free the memory used for weights of the neuron.
   It will take a neuron pointer and free its weights.

   Will return 1 on success and 0 on failure

   NOTE: This will not delete the neuron as neurons will be declared in user program
   and output and bias will still be on the stack. This function is just there to 
   free the momory chunk allocated by initialize_neuron so that we don't have leakage */

int free_neuron(neuron *n) {

        if (n == NULL) {        // this shouldnt happen
		printf("Null pointer passed: neuron = %p\n",n);
                return 0;
                }

	free( n->weights );
	return 1;
	}


/* set_neuron: This function will take an array of floats and initialize them as weights
   of the given neuron. It also takes a bias weight and initializes the bias of the neuron.
   The weights are initialized by copying instead of just pointing to the same array. 

   Returns 1 on success and 0 on failure */

int set_neuron (neuron *n, float *w, float bias, activation a) {
	int i;

	if (n == NULL || w == NULL) {
		printf("Null pointer passed: neuron = %p, weights = %p\n",n,w);
		return 0;
		}

	/* now we have to copy the weights to the neuron. There should be ideally a
	check for length of weights array, but we will assume that the program 
	calling this function will take care of passing a proper array */

	for (i=0; i < n->num_in; i++) {
		n->weights[i] = w[i];
		}

	n->bias_wt = bias;
	n->act = a;

	return 1;
	}




/* print_neuron: This function prints the perceptron parameters in a good way
   Takes a pointer to a neuron.

   Returns 1 on success and 0 on failure */

int print_neuron (neuron *n) {
	int i;

	if( n == NULL) {
		printf("Null pointer passed: neuron = %p\n",n);
		return 0;
		}

	printf("Number of inputs: %d\n",n->num_in);
	printf("Weights: ");
	for (i=0; i < n->num_in; i++)
		printf("%f ",n->weights[i]);
	printf("\n");

	printf("bias weight: %f\n",n->bias_wt);
	printf("output = %f, expected output = %f\n",n->output, n->ex_output);

	return 1;
	}



/* perceptron_update: This function takes a neuron pointer, the parameter eta and a 
   pointer to an input vector. It then updates the weights of the neuron according
   to the perceptron learning algorithm.

   NOTE: for perceptron learning algorithm, see page 6, sidebar in 
   ``Artificial Neural Networks: A tutorial'', IEEE Computer 1996 by
   Anil Jain, Jianchang Mao, 

   Returns 1 on successful update and 0 on failure */

int perceptron_update (neuron *n, float *in, float eta) {
	int i;

	if (n == NULL || in == NULL) {
                printf("Null pointer passed: neuron = %p, inputs = %p\n",n,in);
                return 0;
                }

	/* now use perceptron learning algorithm, which is given as:
		w(t+1)_i = w(t)_i + eta * (d - y) * x_i 	*/

	for (i = 0; i < n->num_in; i++) {
		n->weights[i] += (eta * ( n->ex_output - n->output) * in[i] );
		}

	// now update the bias weight by the same rule
	n->bias_wt += (eta * ( n->ex_output - n->output) );

	return 1;
	}



/* perceptron_update_output: This function takes a vector of inputs and a pointer
   to a neuron. It then calculates the output of the neuron.

   Returns 1 on success and 0 on failure 

   NOTE: This uses step activation function for now. We might want to add support
   for custom activation functions later */

int perceptron_update_output (neuron *n,float *in) {
//int perceptron_update_output (neuron *n,float *in, activation act) {
	int i;
	float sigma_wx = 0.0;

        if (n == NULL || in == NULL) {
                printf("Null pointer passed: neuron = %p, inputs = %p\n",n,in);
                return 0;
                }

	for (i = 0; i < n->num_in; i++) {
		sigma_wx += n->weights[i] * in[i];
	}

	n->output = n->act(sigma_wx + n->bias_wt);
	
	return 1;
	}



/* initialize_ann: this function takes a pointer to an ANN and initializes it in the 
   sense that it allocates and initializes all the arrays in following manner:

   It takes pointer to an ann, learning rate, number of inputs to the network and number of layers. It also 
   needs to have an array of length (number_of_layers) specifying how many neurons in each layer.

   It will then allocate arrays to the inputs and the number of neurons in each layer. It will also
   initiate the arrays as appropriate. All neurons in layer i-1 are connected to those in layer i. 

   Returns 1 on success and 0 on failure */


int initialize_ann (ann *net, float e, int layers, int inputs, int *n) {

	int i;

	if (net == NULL || n == NULL) {
		printf("Null pointer passed: ANN = %p, number of neurons = %p\n",net, n);
		return 0;
		}

	if (layers == 0 || inputs == 0) {
		printf("Number of layers or inputs to the net can not be zero\n");
		return 0;
		}

	/* initialize the basic parameters */
	net->num_layers = layers;
	net->num_in = inputs;
	net->eta = e;

	/* now we have to first allocate an array of inputs and first layer of neurons */
	net->in = (float *) malloc ( sizeof(float) * inputs );

	/* similarly allocate the array for expected output */
	net->ex_output = (float *) malloc ( sizeof(float) * n[layers-1] );

	i = 0;
	while (i < layers) {
		printf("Initializing layer %d\n",i+1);
		int n1 = n[i];
		net->num_neurons[i] = n[i];

		/* if the number of neurons in hidden are 0 then we have a problem */
		if ( n1 <= 0 && n1 < layers ) {
			printf("Number of neurons in hidden layer (layer %d) can not be zero.\n",n1+1);
			return 0;
			}

		// allocate the memory for neurons in this layer
		net->layers[i] = (neuron *) malloc ( sizeof(neuron) * n1 );

		// now allocate memory for the output of this layer
		net->outputs[i] = (float *) malloc ( sizeof(float) * n1 );

		if (net->layers[i] == NULL || net->outputs[i] == NULL) {
			printf("Error allocating memory for layer %d\n",i);
			return 0;
			}

		// now initialize these neurons
		int x;
		for (x = 0; x < n1; x++) {
			int ins_this_layer;
			ins_this_layer = (i == 0 ? net->num_in : net->num_neurons[i-1] );
			initialize_neuron ( &net->layers[i][x], ins_this_layer, step_activation);
			}
		i++;
		}

	/* as a precaution, set extra pointers to null */
	for (i = layers; i < MAXLAYERS; i++) {
//		printf("initializing extra slot %d\n",i+1);
		net->outputs[i] = NULL;
		net->layers[i] = NULL;
		net->num_neurons[i] = 0;
		}

	return 1;
	}


/* free_ann: This function takes a pointer to an ANN and frees all the memory allocated for it.
   The things on stack will still remain 

   Returns 1 on success and 0 on failure */


int free_ann (ann *net) {

	int i;

	if(! net) {
		return 1;
		}

	/* first free the neuron array and outputs */

	for (i=0; i < MAXLAYERS; i++) {
		int j;
		for (j=0; j < net->num_neurons[i]; j++) {
			free_neuron ( &(net->layers[i][j]) );	// first free the neuron
			}
		free ( net->outputs[i] );
		free ( net->layers[i] );
		}

	// then free the input and expected output array
	free ( net->in );
	free ( net->ex_output );
	return 1;
	}


/* print_ann: This is the function to print entire ann in a good way. 
   Takes a pointer to an ann.

   Returns 1 on success and 0 on failure */

int print_ann (ann *net) {
	int i,j;

	if( net == NULL) {
		printf("Null pointer passed: ann = %p\n",net);
		return 0;
		}

	
	printf("Number of inputs to the network = %d\n",net->num_in);
	printf("Current input to the network:\n");
	for (i=0; i < net->num_in; i++)
		printf("%f ",net->in[i]);
	printf("\n\n");

	printf("Number of layers = %d\n\n---\n",net->num_layers);
	
	for (i=0; i < net->num_layers; i++) {
		printf("Layer %d\n",i+1);
		for (j=0; j < net->num_neurons[i]; j++) {
			printf("Neuron %d\n",j+1);
			print_neuron( &(net->layers[i][j]) );
			}

		printf("\nOutput of Layer %d\n",i+1);
		for (j=0; j < net->num_neurons[i]; j++)
			printf("%f ",net->outputs[i][j]);
		printf("\n--- \n\n");
		}

	printf("Expected output of the network:\n");
	for (i=0; i < net->num_neurons[net->num_layers-1]; i++)
		printf("%f ",net->ex_output[i]);
	printf("\n--- \n\n");
	return 1;
	}




/* err_backpropogation: This function takes a pointer to an ANN and propogates the input through
   the network. It then propogates the error backwards to update the weights of all the neurons
   in the ANN.

   Returns 1 on success and 0 on failure */

int err_backpropogation (ann * net) {
	int i;

	if(! net) {
		printf("Null pointer passed: ann = %p\n",net);
		return 0;
		}

	/* first thing to do is to find delta for output layer. The delta is given for output as:
			delta = g' * (t - y)	
	as given on http://www.willamette.edu/~gorr/classes/cs449/backprop.html. g' is 1 in our case */


	int last_layer_index = net->num_layers - 1;

	float * delta = (float *) malloc ( sizeof(float) * net->num_neurons[last_layer_index] );

	if (! delta ) {
		printf("Error allocating memory for delta array\n");
		return 0;
		}

	for (i = 0; i < net->num_neurons[last_layer_index]; i++) {
		delta[i] = net->ex_output[i] - net->outputs[last_layer_index][i];
		}


	/* now for each layer of neurons, we have to calculate new deltas and update the weights */
	for (i = net->num_layers - 1; i > 0; i--) {

		// i here is the previous layer (closer to input) hence we have to use (i-1) for indexing
		float * delta1 = (float *) malloc ( sizeof(float) * net->num_neurons[i-1] );
		if ( ! delta1 ) {
			printf("Error allocating memory for delta-1 array\n");
			return 0;
			}
		
		// now compute the delta for this layer (closer to the input)
		int j;
		for (j = 0; j < net->num_neurons[i-1]; j++) {
			
			/* for each neuron in this layer, we have to compute delta as
					delta^i_j = sum ( delta^i+1_j * weight^i+1_j )			*/
			float delta_sum = 0.0;
			int k;

			for (k=0; k < net->num_neurons[i]; k++) {
				delta_sum += delta[k] * net->layers[i][k].weights[j] ;
				}

			// take derivative of tanh if needed
			if (net->layers[i-1][j].act == tanh_activation) {
				float der = ( 1 - pow(net->outputs[i-1][j],2) );
				delta_sum *= der;
			}
			delta1[j] = delta_sum;
			

			/* now we have to update the weights of the layer i+1 (index i) */
			for (k=0; k < net->num_neurons[i]; k++) {
				net->layers[i][k].weights[j] += net->eta * delta[k] * net->outputs[i-1][j];
				net->layers[i][k].bias_wt += net->eta * delta[k] * net->layers[i][k].bias;
				}

			}


		// now that delta is calculated, switch the pointers such that delta1 is new delta
		free (delta);
		delta = delta1;
		delta1 = NULL;
		}

		// now we still have to update the weights for the layer closest to the input
		for (i=0; i < net->num_neurons[0]; i++) {
			int j;
			for (j=0; j < net->num_in; j++) 
				net->layers[0][i].weights[j] += net->eta * delta[i] * net->in[j];
				net->layers[0][i].bias_wt += net->eta * delta[i] * net->layers[0][i].bias;
			}

	free(delta);
	return 1;
	}



/* fwd_propogation: This function propogates the input through the neural network.
   It takes a pointer to a neural network and updates the outputs of all the neurons
   in the network

   Returns 1 on success and 0 on failure */

int fwd_propogation (ann * net) {
	int i,j;

	if (! net) {
		printf("Null pointer passed: ann = %p\n",net);
		return 0;
		}

	/* iterate over all the layers */

	for (i=0; i < net->num_layers; i++) {
		float * in;
		
		if (i == 0) {
			in = net->in;
			} else {
			in = net->outputs[i-1];
			}
		
		/* now update the outputs of all the neurons */
		for (j=0; j < net->num_neurons[i]; j++) {
			
			/* update the output of this selected neuron */
			perceptron_update_output( &net->layers[i][j], in);
			//perceptron_update_output( &net->layers[i][j], in, step_activation);
			net->outputs[i][j] = net->layers[i][j].output;
			} 
		}

	return 1;
	}






float linear_activation (float output) {
	return output;
	}

float bipolar_step_activation (float output) {
	if (output > 0)
		return 1;
	else
		return -1;
	}

float step_activation (float output) {
	return (output > 0);
	}

float tanh_activation (float output) {
	return tanh(output);
	}
