#include <ANNetwork.h>
#include <iostream>
#include <Layer.h>
#include <Functions.h>
#include <chrono>
#include <exception>
#include <fstream>
#include <exception>

#include "BoardState.h"
#include "MoveData.h"


int main()
{
	//srand(time(NULL));
	AnnUtilities::ANNetwork ann;
	BoardState::BoardManager manager;
	BoardState::BoardStateData board;

	AnnUtilities::ANNSettings annSettings;
	annSettings._hiddenActicationFunction = AnnUtilities::ACTFUNC::TANH;
	annSettings._outputActicationFunction = AnnUtilities::ACTFUNC::SIGMOID;
	annSettings._inputSize = BoardState::ANN_INPUT_LENGTH;
	annSettings._hiddenSize = 900;
	annSettings._outputSize = 1;
	annSettings._numberOfHiddenLayers = 3;
	annSettings._learningRate = 0.1f;
	annSettings._momentum = 0.0f;
	ann._settings = annSettings;

	ann.Init();
	manager.calculateZobristValues();

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	try
	{
		int i = 0;
		for (; i < 100; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, ann, 2, 1000);
			manager.train(ann);
			manager.reset();
		}
		manager.exportANN(ann, "ann100.ann");

		for (; i < 500; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, ann, 2, 1000);
			manager.train(ann);
			manager.reset();
		}
		manager.exportANN(ann, "ann500.ann");

		for (; i < 2500; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, ann, 2, 1000);
			manager.train(ann);
			manager.reset();
		}
		manager.exportANN(ann, "ann2500.ann");

		for (; i < 12500; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, ann, 2, 1000);
			manager.train(ann);
			manager.reset();
		}
		manager.exportANN(ann, "ann10000.ann");
	}
	catch (std::exception e)
	{
		std::ofstream crashDump;
		crashDump.open("../../crashdump.txt");
		crashDump << e.what();
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	std::cout << "Finished training in " << std::chrono::duration_cast<std::chrono::minutes>(end - begin).count() << " minutes" << std::endl;


	int exit;
	std::cin >> exit;
	return 0;
}
