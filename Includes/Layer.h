#pragma once
#include "Functions.h"

namespace AnnUtilities
{
	class Layer
	{
	private:
		AnnUtilities::ACTFUNC _actfunc;
		float* _deltaWeights = nullptr;
		float* _deltaBiases = nullptr;
		float* _error = nullptr;
		void calculateError();
		void calculateDerivative();
		void calculateDelta();

	public:
		int _layerSize;
		float* _outputs = nullptr;
		Layer* _prevLayer = nullptr;
		Layer* _nextLayer = nullptr;
		float* _biases = nullptr;
		float* _weights = nullptr;
		Layer(Layer* previousLayer, int layerSize, AnnUtilities::ACTFUNC actfunc);
		~Layer();

		void propagateForward();
		void propagateBackward();
		void propagateBackward(const float* const error);
		void update(const float learningRate, const int epochs);

		void setNextLayer(Layer* nextLayer) { _nextLayer = nextLayer; }
		void setOutputs(const float* const outputs) { for (int i = 0; i < _layerSize; ++i) { _outputs[i] = outputs[i]; }; }

		float* getOutput() const { return _outputs; }
		float getOutput(const int node) const { return _outputs[node]; }
	};
}