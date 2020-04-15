#include <math.h>

namespace AnnUtilities
{
	float sigmoid(float x)
	{
		return 1.0f / (1 + exp(-x));
	}

	float dSigmoid(float x)
	{
		return sigmoid(x) * (1 - sigmoid(x));
	}

	float reLu(float x)
	{
		if (x < 0.0f)
		{
			return 0;
		}
		return x;
	}

	float dReLu(float x)
	{
		if (x < 0.0f)
		{
			return 0;
		}
		return 1;
	}
}