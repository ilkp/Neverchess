#pragma once
#include "ANNSettings.h"


namespace AnnUtilities
{
	class Layer;
	struct InputData;

	class ANNetwork
	{
	private:

	public:
		AnnUtilities::ANNSettings _settings;
		Layer* _inputLayer = nullptr;
		Layer* _outputLayer = nullptr;
		ANNetwork();
		~ANNetwork();
		void propagateForward();
		void propagateBackward(const float* const labels);
		void Init();
		void Epoch(const InputData* const inputData, const int inputSize, const float learningRate);
		void update(const int batchSize);
		void Clean();
	};
}