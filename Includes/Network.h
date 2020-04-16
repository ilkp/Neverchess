#pragma once



namespace AnnUtilities
{
	class Layer;
	struct InputData;

	class Network
	{
	private:

	public:
		Layer* _inputLayer = nullptr;
		Layer* _outputLayer = nullptr;
		Network();
		~Network();
		void propagateForward();
		void propagateBackward(const float* const labels);
		void Init(const int inputSize, const int hiddenSize, const int outputSize, const int hiddenLayers,
			float(*activationFuncHiddenL)(float), float(*derivativeFuncHiddenL)(float), float(*activationFuncOutputL)(float), float(*derivativeFuncOutputL)(float));
		void Epoch(const InputData* const inputData, const int inputSize, const float learningRate);
		void update(const int batchSize, const float learningRate);
		float* Test(const float* const inputData);
		void Clean();
	};
}