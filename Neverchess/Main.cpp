#include <math.h>
#include <Network.h>
#include <iostream>
#include <bitset>
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
	network.Init(BoardState::ANN_INPUT_LENGTH, 500, 1, 3, AnnUtilities::sigmoid, AnnUtilities::sigmoid, AnnUtilities::dSigmoid, AnnUtilities::dSigmoid);
	BoardState::BoardManager manager;
	BoardState::BoardStateData board;
	manager.initBoardStateDataPieces(board._pieces);
	//board._turn = true;
	//board._kingMoved[0] = true;
	//board._kingMoved[1] = true;
	//manager.placePiece(board._pieces, PieceCode::W_KING, 4, 0);
	//manager.placePiece(board._pieces, PieceCode::B_KING, 0, 7);
	//manager.placePiece(board._pieces, PieceCode::W_ROOK, 7, 0);
	//manager.placePiece(board._pieces, PieceCode::W_QUEEN, 1, 5);

	manager.process(board, network, 2, 10000);
	//BoardState::AlphaBetaEvaluation eval = manager.alphaBeta(board, network, 2, 0, 1);

	//std::cout << eval.blackWin << "\n" << eval.whiteWin << std::endl;
	return 0;
}
