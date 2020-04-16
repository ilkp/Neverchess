
public class ANNetwork
{
	private ANNLayer _inputLayer;
	private ANNLayer _outputLayer;

	public void ReadANN(string file)
	{
		string[] lines = System.IO.File.ReadAllLines(file);
		int line = 0;
		int inputSize = int.Parse(lines[line]);
		int hiddenSize = int.Parse(lines[++line]);
		int outputSize = int.Parse(lines[++line]);
		int nHiddenLayers = int.Parse(lines[++line]);

		_inputLayer = new ANNLayer(inputSize);
		_inputLayer._previousLayer = null;
		ANNLayer[] hiddenLayers = new ANNLayer[nHiddenLayers];
		hiddenLayers[0] = new ANNLayer(hiddenSize);
		hiddenLayers[0]._previousLayer = _inputLayer;
		for (int i = 1; i < nHiddenLayers; ++i)
		{
			hiddenLayers[i] = new ANNLayer(hiddenSize, hiddenLayers[i - 1]);
			hiddenLayers[i - 1]._nextLayer = hiddenLayers[i];
		}
		_outputLayer = new ANNLayer(outputSize, hiddenLayers[hiddenLayers.Length - 1]);
		_outputLayer._nextLayer = null;
		hiddenLayers[hiddenLayers.Length - 1] = _outputLayer;

		ANNLayer l = _inputLayer._nextLayer;
		for (int i = 0; i < hiddenSize; ++i)
		{
			for (int j = 0; j < inputSize; ++j)
			{
				l._weights[i * inputSize + j] = float.Parse(lines[++line]);
			}
		}
		l = l._nextLayer;
		while (l != _outputLayer)
		{
			for (int i = 0; i < hiddenSize; ++i)
			{
				for (int j = 0; j < hiddenSize; ++j)
				{
					l._weights[i * hiddenSize + j] = float.Parse(lines[++line]);
				}
			}
			l = l._nextLayer;
		}
		for (int i = 0; i < outputSize; ++i)
		{
			for (int j = 0; j < hiddenSize; ++j)
			{
				_outputLayer._weights[i * hiddenSize + j] = float.Parse(lines[++line]);
			}
		}
		l = _inputLayer._nextLayer;
		while (l != null)
		{
			for (int i = 0; i < l._layerSize; ++i)
			{
				l._biases[i] = float.Parse(lines[++line]);
			}
			l = l._nextLayer;
		}
	}

	private float ReLu(float x)
	{
		if (x < 0.0f)
		{
			return 0;
		}
		return x;
	}

	private float dRelu(float x)
	{
		if (x < 0.0f)
		{
			return 0.0f;
		}
		return 1.0f;
	}

	private float Sigmoid(float x)
	{
		return 1.0f / (1 + UnityEngine.Mathf.Exp(-x));
	}

	private float DSigomid(float x)
	{
		return Sigmoid(x) * (1 - Sigmoid(x));
	}
}
