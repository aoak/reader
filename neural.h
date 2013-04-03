/*_____________________________________________________________________________
neural.h
    This is the header file for giving the basic functionality of the neural
	networks.
	The file contains the structure definitions for a single perceptron as well
	as multilayer feed-forward perceptron network. The functionality provided
	includes following:

		initialize_neuron:		 	Initializes the neuron structure members
		free_neuron:				Frees members of the neuron structure
		set_neuron:					Set parameters of neuron
		print_neuron:				Prints parameters of a neuron
		perceptron_update:			Updates the weights of the single perceptron 
									according to perceptron learning rule
		perceptron_update_output	Calculates and updates the output of the
									perceptron

		initialize_ann:				Initializes the ANN structure
		free_ann:					Frees members of the ANN structure and neurons
									in it.
		print_ann:					Prints parameters of an ANN
		err_backpropogation:		Updates the weights of a multilayered feed-
									forward perceptron network using error back-
									propogation algorithm

_______________________________________________________________________________
Author: 
    Aniket V. Oak
Date Created:
    02-Mar-2013
_______________________________________________________________________________*/


#ifndef _NEURAL_GUARD
#define _NEURAL_GUARD

/*___________________ single neuron and related functions ___________________ */


/* ____________________ activation functions for neurons _____________________ */


typedef float (* activation) (float);

float linear_activation (float);
float step_activation (float);
float bipolar_step_activation (float);
float tanh_activation (float);


/* Structure of a neuron. 
   Basically, we need a set of weighted inputs. We do this by having a set of
   weights. The inputs will be multiplied by these weights.

   Then we need an output, and a bias. Both of which will be just a float. */
   
typedef struct {
	float * weights;	/* weights have to be number. for now lets try to make it
				dynamic. If I cant pull it off, I will make if static putting
				an upper limit on inputs a neuron can have. */
	int num_in;		// number of inputs (weights)
	float output;	
	float bias;		/* bias. This can be done using one of the inputs, but for 
				ease and clarity, lets make it separate */
	float bias_wt;		// weight for the bias

	float ex_output;	/* expected output of the neuron. I am not totally sure
				whether this should be here - in this structure, but it seems
				most convinient and proper at this moment. */
	activation act;
	} neuron;

/* ____________________ general functions for single neurons _____________________ */

/* initialize_neuron: This is a function to initialize a neuron. It should take a
   pointer to a neuron, and number of inputs it will have. It should then initialize
   output to 0 and bias to random value and allocate an array for weights
   according to the number of inputs.
  
   NOTE: This does not set the weights of a neuron to particular value. It sets them
         to random thing. The right function for the former will be set_neuron

   Success will be returned as 1 and failure as 0 */

int initialize_neuron (neuron *, int, activation);


/* free_neuron: This function will free the memory used for weights of the neuron.
   It will take a neuron pointer and free its weights.

   Will return 1 on success and 0 on failure

   NOTE: This will not delete the neuron as neurons will be declared in user program
   and output and bias will still be on the stack. This function is just there to 
   free the momory chunk allocated by initialize_neuron so that we don't have leakage */

int free_neuron(neuron *);

/* set_neuron: This function will take an array of floats and initialize them as weights
   of the given neuron. It also takes a bias weight and initializes the bias of the neuron.
   The weights are initialized by copying instead of just pointing to the same array. 

   Returns 1 on success and 0 on failure */

int set_neuron (neuron *, float *, float, activation);


/* print_neuron: This function prints the perceptron parameters in a good way
   Takes a pointer to a neuron.

   Returns 1 on success and 0 on failure */

int print_neuron (neuron *);

/* perceptron_update: This function takes a neuron pointer, the parameter eta and a 
   pointer to an input vector. It then updates the weights of the neuron according
   to the perceptron learning algorithm.

   NOTE: for perceptron learning algorithm, see page 6, sidebar in 
   ``Artificial Neural Networks: A tutorial'', IEEE Computer 1996 by
   Anil Jain, Jianchang Mao, 

   Returns 1 on successful update and 0 on failure */

int perceptron_update (neuron *,float *,float);

/* perceptron_update_output: This function takes a vector of inputs and a pointer
   to a neuron. It then calculates the output of the neuron.

   Returns 1 on success and 0 on failure 

   NOTE: This uses step activation function for now. We might want to add support
   for custom activation functions later */

int perceptron_update_output (neuron *,float *);//,activation);




/* ____________________ network of neurons and related functions _____________________ */


#define MAXLAYERS 3

typedef struct {

	int num_layers;			// first thing we need is number of layers
	int num_in;				// number of inputs to the network 
	float * in;				// and array for it
	float eta;				// learning rate for this network
	float * ex_output;		// expected output vector

	int num_neurons[MAXLAYERS];		// array for number of neurons in each layer
	float * outputs[MAXLAYERS];		// array for output of each layer
	neuron * layers[MAXLAYERS];		// array of neuron layers

	} ann;




/* initialize_ann: this function takes a pointer to an ANN and initializes it in the 
   sense that it allocates and initializes all the arrays in following manner:

   It takes pointer to an ann, learning rate, number of inputs to the network and number of layers. It also 
   needs to have an array of length (number_of_layers) specifying how many neurons in each layer.

   It will then allocate arrays to the inputs and the number of neurons in each layer. It will also
   initiate the arrays as appropriate. All neurons in layer i-1 are connected to those in layer i. 

   Returns 1 on success and 0 on failure */


int initialize_ann (ann *, float, int, int, int *);


/* free_ann: This function takes a pointer to an ANN and frees all the memory allocated for it.
   The things on stack will still remain 

   Returns 1 on success and 0 on failure */

int free_ann (ann *);


/* print_ann: This is the function to print entire ann in a good way. 
   Takes a pointer to an ann.

   Returns 1 on success and 0 on failure */

int print_ann (ann *);


/* err_backpropogation: This function takes a pointer to an ANN and propogates the input through
   the network. It then propogates the error backwards to update the weights of all the neurons
   in the ANN.

   Returns 1 on success and 0 on failure */

int err_backpropogation (ann *);


/* fwd_propogation: This function propogates the input through the neural network.
   It takes a pointer to a neural network and updates the outputs of all the neurons
   in the network

   Returns 1 on success and 0 on failure */

int fwd_propogation (ann *);















#endif
