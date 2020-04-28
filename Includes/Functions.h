#pragma once

namespace AnnUtilities
{
	enum class ACTFUNC
	{
		SIGMOID,
		RELU,
		LEAKY_RELU,
		TANH,
	};

	float sigmoid(float x);

	float dSigmoid(float x);

	float relu(float x);

	float dRelu(float x);

	float leakyRelu(float x);

	float dLeakyRelu(float x);

	float hypTanh(float x);

	float dTanh(float x);
}