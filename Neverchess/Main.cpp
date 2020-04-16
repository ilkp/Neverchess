#include <Network.h>
#include <iostream>
#include <Layer.h>
#include <Functions.h>

#include "BoardState.h"
#include "MoveData.h"

#define HIDDEN_SIZE 500
#define OUTPUT_SIZE 1
#define H_LAYERS 3

int main()
{
	AnnUtilities::Network network;
	network.Init(BoardState::ANN_INPUT_LENGTH, 900, 1, 3, AnnUtilities::reLu, AnnUtilities::sigmoid, AnnUtilities::dReLu, AnnUtilities::dSigmoid);
	BoardState::BoardManager manager;
	BoardState::BoardStateData board;

	for (int i = 0; i < 1000; ++i)
	{
		std::cout << i << "th game" << std::endl;
		manager.resetBoardStateData(board);
		manager.process(board, network, 2, 1000);
		manager.train(network);
		manager.reset();
	}

	int exit;
	std::cin >> exit;
	return 0;
}
