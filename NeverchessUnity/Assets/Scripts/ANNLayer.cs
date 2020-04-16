using System;

public class ANNLayer
{
	public int _layerSize;
	public float[] _weights;
	public float[] _biases;
	public float[] _outputs;
	public Func<float, float> _activationFunction;
	public ANNLayer _previousLayer;
	public ANNLayer _nextLayer;

	public ANNLayer(int layerSize)
	{
		_layerSize = layerSize;
		_outputs = new float[layerSize];
	}

	public ANNLayer(int layerSize, ANNLayer previousLayer)
	{
		_previousLayer = previousLayer;
		_layerSize = layerSize;
		_weights = new float[previousLayer._layerSize * layerSize];
		_biases = new float[layerSize];
		_outputs = new float[layerSize];
	}

	public void PropagateForward()
	{

	}
}
