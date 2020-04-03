#pragma once



namespace AnnUtilities
{
	class Layer;
	struct InputData;

	class Network
	{
	private:
		Layer* _inputLayer = nullptr;
		Layer* _outputLayer = nullptr;
		void propagateForward();
		void propagateBackward(const float* const labels);

	public:
		Network();
		~Network();
		void Init(const int inputSize, const int hiddenSize, const int outputSize, const int hiddenLayers,
			float(*activationH)(float), float(*activationO)(float), float(*derivativeH)(float), float(*derivativeO)(float));
		void Epoch(const InputData* const inputData, const int inputSize, const float learningRate);
		float* Test(const float* const inputData);
		void Clean();
	};
}