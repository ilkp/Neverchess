#pragma once

namespace AnnUtilities
{
	class Layer
	{
	private:
		float* _deltaWeights = nullptr;
		float* _deltaBiases = nullptr;
		float* _outputs = nullptr;
		float* _error = nullptr;
		float (*_activationFunction)(float);
		float (*_derivativeFunction)(float);

	public:
		int _layerSize;
		Layer* _prevLayer = nullptr;
		Layer* _nextLayer = nullptr;
		float* _biases = nullptr;
		float* _weights = nullptr;
		Layer(Layer* previousLayer, int layerSize, float(*activationFunction)(float), float(*derivativeFunction)(float));
		~Layer();

		void propagationForward();
		void propagationBackward();
		void calculateDelta();
		void update(const float learningRate, const int epochs);

		void setNextLayer(Layer* nextLayer) { _nextLayer = nextLayer; }
		void setError(const int node, const float correctOutput) { _error[node] = _outputs[node] - correctOutput; }
		void setOutputs(const float* const outputs) { for (int i = 0; i < _layerSize; ++i) { _outputs[i] = outputs[i]; }; }

		float* getOutput() const { return _outputs; }
		float getOutput(const int node) const { return _outputs[node]; }
	};
}