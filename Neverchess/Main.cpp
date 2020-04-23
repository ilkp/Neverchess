#include <Network.h>
#include <iostream>
#include <Layer.h>
#include <Functions.h>
#include <chrono>
#include <exception>
#include <fstream>

#include "BoardState.h"
#include "MoveData.h"

#define HIDDEN_SIZE 500
#define OUTPUT_SIZE 1
#define H_LAYERS 3

int main(int argc, char* argv[])
{
	srand(time(NULL));
	AnnUtilities::Network network;
	network.Init(BoardState::ANN_INPUT_LENGTH, 900, 1, 3, AnnUtilities::sigmoid, AnnUtilities::dSigmoid, AnnUtilities::sigmoid, AnnUtilities::dSigmoid);
	BoardState::BoardManager manager;
	BoardState::BoardStateData board;
	manager.calculateZobristValues();

	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	try
	{
		for (int i = 0; i < 500; ++i)
		{
			std::cout << i << "th game" << std::endl;
			manager.resetBoardStateData(board);
			manager.process(board, network, 2, 1000);
			manager.train(network);
			manager.reset();
		}
	}
	catch (std::exception e)
	{
		std::ofstream crashDump;
		crashDump.open("CRASH_DUMP.TXT");
		crashDump << e.what();
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	std::cout << "Finished training in " << std::chrono::duration_cast<std::chrono::minutes>(end - begin).count() << " minutes" << std::endl;

	manager.exportANN(network, "testANN.ann");

	int exit;
	std::cin >> exit;
	return 0;
}
