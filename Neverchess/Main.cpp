#include <Network.h>
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
	AnnUtilities::Network network;
	BoardState::BoardManager manager;
	BoardState::BoardStateData board;
	network.Init(BoardState::ANN_INPUT_LENGTH, 900, 1, 3, AnnUtilities::ACTFUNC::TANH, AnnUtilities::ACTFUNC::SIGMOID);
	manager.calculateZobristValues();

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	try
	{
		int i = 0;
		for (; i < 100; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, network, 2, 1000);
			manager.train(network);
			manager.reset();
		}
		manager.exportANN(network, "ann100.ann");

		for (; i < 500; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, network, 2, 1000);
			manager.train(network);
			manager.reset();
		}
		manager.exportANN(network, "ann500.ann");

		for (; i < 2500; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, network, 2, 1000);
			manager.train(network);
			manager.reset();
		}
		manager.exportANN(network, "ann2500.ann");

		for (; i < 12500; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, network, 2, 1000);
			manager.train(network);
			manager.reset();
		}
		manager.exportANN(network, "ann10000.ann");
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
