#include <math.h>
#include <Network.h>
#include <iostream>
#include <bitset>

#include "BoardState.h"
#include "MoveData.h"
#include <Layer.h>

#define INPUT_SIZE 448
#define HIDDEN_SIZE 500
#define OUTPUT_SIZE 2
#define H_LAYERS 3

float sigmoid(float x)
{
	return 1.0f / (1 + exp(-x));
}

float dSigmoid(float x)
{
	return sigmoid(x) * (1 - sigmoid(x));
}

float relu(float x)
{
	if (x < 0.0f)
	{
		return 0;
	}
	return x;
}

float dRelu(float x)
{
	if (x < 0.0f)
	{
		return 0;
	}
	return 1;
}

int main()
{
	//Network ann;
	//ann.Init(INPUT_SIZE, HIDDEN_SIZE, OUTPUT_SIZE, H_LAYERS, relu, sigmoid, dRelu, dSigmoid);
}
