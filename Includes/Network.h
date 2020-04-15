#pragma once



namespace AnnUtilities
{
	class Layer;
	struct InputData;

	class Network
	{
	private:
		void propagateBackward(const float* const labels);

	public:
		Layer* _inputLayer = nullptr;
		Layer* _outputLayer = nullptr;
		Network();
		~Network();
		void propagateForward();
		void Init(const int inputSize, const int hiddenSize, const int outputSize, const int hiddenLayers,
			float(*activationFuncHiddenL)(float), float(*activationFuncOutputL)(float), float(*derivativeFuncHiddenL)(float), float(*derivativeFuncOutputL)(float));
		void Epoch(const InputData* const inputData, const int inputSize, const float learningRate);
		float* Test(const float* const inputData);
		void Clean();
	};
}