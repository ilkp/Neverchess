#include "BoardState.h"
#include "PieceCode.h"
#include "MoveData.h"
#include "Network.h"
#include "Layer.h"
#include <math.h>
#include <algorithm>
#include <iostream>
#include <random>
#include <fstream>
#include <time.h>
#include <chrono>
#include <string>


namespace BoardState
{
	void BoardManager::process(BoardStateData& boardStateData, AnnUtilities::Network& network, int evaluationDepth, int maxTurns)
	{
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		int turn = 0;
		AlphaBetaEvaluation eval;
		//std::vector<MoveData> randomMoves;
		//genRawMoves(boardStateData, randomMoves);
		//filterMoves(boardStateData, randomMoves);
		//playMove(boardStateData, randomMoves[rand() % randomMoves.size()]);
		//randomMoves.clear();
		//genRawMoves(boardStateData, randomMoves);
		//filterMoves(boardStateData, randomMoves);
		//playMove(boardStateData, randomMoves[rand() % randomMoves.size()]);

		while (turn < maxTurns)
		{
			eval = alphaBeta(boardStateData, network, evaluationDepth, -1000.0f, 1000.0f);
			alphaBetaHistory.push(eval);
			if (eval.whiteWin || eval.blackWin)
			{
				break;
			}
			increasePositionMap(boardStateData, eval.move);
			playMove(boardStateData, eval.move);
			//std::cout << turn << " " << (turn%2 == 0 ? "white" : "black") << "\t(" << eval.move.xStart << ", " << eval.move.yStart << ") -> (" << eval.move.xEnd << ", " << eval.move.yEnd << ")\t"
			//	<< eval.evaluatedValue << std::endl;
			//printBoard(boardStateData);
			if (hashPositions.at(zobrishHash(boardStateData)) > 2 || turn > maxTurns)
			{
				std::cout << "Draw by repetition" << std::endl;
				alphaBetaHistory.top().blackWin = false;
				alphaBetaHistory.top().whiteWin = false;
				alphaBetaHistory.top().draw = true;
				break;
			}
			++turn;
		}
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "turn: " << turn << ", elapsed time: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count()
			<< ", draw = " << alphaBetaHistory.top().draw << ", white win = " << alphaBetaHistory.top().whiteWin << ", black win = " << alphaBetaHistory.top().blackWin << std::endl;
		printBoard(boardStateData);
	}

	AlphaBetaEvaluation BoardManager::alphaBeta(BoardStateData& boardStateData, AnnUtilities::Network& network, int depth, float alpha, float beta)
	{
		AlphaBetaEvaluation evaluation;
		evaluation.boardStateData = boardStateData;
		std::vector<MoveData> moves;
		genRawMoves(boardStateData, moves);
		std::vector<BoardStateData> newStates = filterMoves(boardStateData, moves);
		if (moves.size() == 0)
		{
			evaluate(boardStateData, network, evaluation, true);
			return evaluation;
		}
		if (depth <= 0)
		{
			evaluate(boardStateData, network, evaluation, false);
			return evaluation;
		}
		BoardStateData temp;
		float value;
		int i = 0;
		evaluation.move = moves[0];
		float abValue;
		if (!boardStateData._turn) // white always minimizing player
		{
			value = 1000.0f;
			evaluation.evaluatedValue = value;
			for (; i < moves.size(); ++i)
			{
				abValue = alphaBeta(newStates[i], network, depth - 1, alpha, beta).evaluatedValue;
				if (abValue < value)
				{
					value = abValue;
					evaluation.evaluatedValue = value;
					evaluation.move = moves[i];
				}
				beta = std::min(beta, value);
				if (alpha >= beta)
				{
					break;
				}
			}
		}
		else // black always maximizing player
		{
			value = -1000.0f;
			evaluation.evaluatedValue = value;
			for (; i < moves.size(); ++i)
			{
				abValue = alphaBeta(newStates[i], network, depth - 1, alpha, beta).evaluatedValue;
				if (abValue > value)
				{
					value = abValue;
					evaluation.evaluatedValue = value;
					evaluation.move = moves[i];
				}
				alpha = std::max(alpha, value);
				if (alpha >= beta)
				{
					break;
				}
			}
		}
		return evaluation;
	}

	void BoardManager::initBoardStateDataPieces(PieceCode pieces[])
	{
		for (int x = 0; x < BOARD_LENGTH; ++x)
		{
			pieces[BOARD_LENGTH + x] = PieceCode::W_PAWN;
			pieces[6 * BOARD_LENGTH + x] = PieceCode::B_PAWN;
		}

		pieces[0] = PieceCode::W_ROOK;
		pieces[1] = PieceCode::W_KNIGHT;
		pieces[2] = PieceCode::W_BISHOP;
		pieces[3] = PieceCode::W_QUEEN;
		pieces[4] = PieceCode::W_KING;
		pieces[5] = PieceCode::W_BISHOP;
		pieces[6] = PieceCode::W_KNIGHT;
		pieces[7] = PieceCode::W_ROOK;

		pieces[7 * BOARD_LENGTH + 0] = PieceCode::B_ROOK;
		pieces[7 * BOARD_LENGTH + 1] = PieceCode::B_KNIGHT;
		pieces[7 * BOARD_LENGTH + 2] = PieceCode::B_BISHOP;
		pieces[7 * BOARD_LENGTH + 3] = PieceCode::B_QUEEN;
		pieces[7 * BOARD_LENGTH + 4] = PieceCode::B_KING;
		pieces[7 * BOARD_LENGTH + 5] = PieceCode::B_BISHOP;
		pieces[7 * BOARD_LENGTH + 6] = PieceCode::B_KNIGHT;
		pieces[7 * BOARD_LENGTH + 7] = PieceCode::B_ROOK;
	}

	void BoardManager::resetBoardStateData(BoardStateData& boardStateDate)
	{
		for (int y = 2; y < BOARD_LENGTH - 2; ++y)
		{
			for (int x = 0; x < BOARD_LENGTH; ++x)
			{
				boardStateDate._pieces[y * BOARD_LENGTH + x] = PieceCode::EMPTY;
			}
		}

		for (int x = 0; x < BOARD_LENGTH; ++x)
		{
			boardStateDate._pieces[BOARD_LENGTH + x] = PieceCode::W_PAWN;
			boardStateDate._pieces[6 * BOARD_LENGTH + x] = PieceCode::B_PAWN;
		}

		boardStateDate._pieces[0] = PieceCode::W_ROOK;
		boardStateDate._pieces[1] = PieceCode::W_KNIGHT;
		boardStateDate._pieces[2] = PieceCode::W_BISHOP;
		boardStateDate._pieces[3] = PieceCode::W_QUEEN;
		boardStateDate._pieces[4] = PieceCode::W_KING;
		boardStateDate._pieces[5] = PieceCode::W_BISHOP;
		boardStateDate._pieces[6] = PieceCode::W_KNIGHT;
		boardStateDate._pieces[7] = PieceCode::W_ROOK;

		boardStateDate._pieces[7 * BOARD_LENGTH + 0] = PieceCode::B_ROOK;
		boardStateDate._pieces[7 * BOARD_LENGTH + 1] = PieceCode::B_KNIGHT;
		boardStateDate._pieces[7 * BOARD_LENGTH + 2] = PieceCode::B_BISHOP;
		boardStateDate._pieces[7 * BOARD_LENGTH + 3] = PieceCode::B_QUEEN;
		boardStateDate._pieces[7 * BOARD_LENGTH + 4] = PieceCode::B_KING;
		boardStateDate._pieces[7 * BOARD_LENGTH + 5] = PieceCode::B_BISHOP;
		boardStateDate._pieces[7 * BOARD_LENGTH + 6] = PieceCode::B_KNIGHT;
		boardStateDate._pieces[7 * BOARD_LENGTH + 7] = PieceCode::B_ROOK;

		boardStateDate._turn = 0;
		boardStateDate._kingMoved[0] = false;
		boardStateDate._kingMoved[1] = false;
		boardStateDate._kRookMoved[0] = false;
		boardStateDate._kRookMoved[1] = false;
		boardStateDate._qRookMoved[0] = false;
		boardStateDate._qRookMoved[1] = false;
		boardStateDate._enPassant = -1;
	}

	void BoardManager::calculateZobristValues()
	{
		std::random_device rd;
		std::mt19937_64 e2(rd());
		std::uniform_int_distribution<long long int> dist(std::llround(std::pow(2, 61)), std::llround(std::pow(2, 62)));
		long long int v;
		for (int i = 0; i < BOARD_LENGTH * BOARD_LENGTH * 12; ++i)
		{
			do
			{
				v = dist(e2);
			} while (zobristValueExists(v));
			zobristPieceValues[i] = v;
		}
	}

	void BoardManager::exportANN(AnnUtilities::Network& network, std::string fileName)
	{
		int hiddenLayers = 0;
		AnnUtilities::Layer* l = network._inputLayer->_nextLayer;
		while (l != network._outputLayer)
		{
			++hiddenLayers;
			l = l->_nextLayer;
		}
		std::ofstream file;
		file.open(fileName, std::ios_base::app);
		file << network._inputLayer->_layerSize << "\n";
		file << network._inputLayer->_nextLayer->_layerSize << "\n";
		file << network._outputLayer->_layerSize << "\n";
		file << hiddenLayers << "\n";
		l = network._inputLayer->_nextLayer;
		for (int i = 0; i < l->_layerSize; ++i)
		{
			for (int j = 0; j < network._inputLayer->_layerSize; ++j)
			{
				file << l->_weights[i * network._inputLayer->_layerSize + j] << "\n";
			}
		}
		l = l->_nextLayer;
		while (l != network._outputLayer)
		{
			for (int i = 0; i < network._inputLayer->_layerSize; ++i)
			{
				for (int j = 0; j < network._inputLayer->_layerSize; ++j)
				{
					file << l->_weights[i * network._inputLayer->_layerSize + j] << "\n";
				}
			}
			l = l->_nextLayer;
		}
		for (int i = 0; i < network._outputLayer->_layerSize; ++i)
		{
			for (int j = 0; j < network._outputLayer->_prevLayer->_layerSize; ++j)
			{
				file << network._outputLayer->_weights[i * network._outputLayer->_prevLayer->_layerSize + j] << "\n";
			}
		}
		l = network._inputLayer->_nextLayer;
		while (l != nullptr)
		{
			for (int i = 0; i < l->_layerSize; ++i)
			{
				file << l->_biases[i] << "\n";
			}
			l = l->_nextLayer;
		}
		file.close();
	}

	void BoardManager::placePiece(PieceCode pieces[], PieceCode pieceCode, int x, int y)
	{
		pieces[y * BOARD_LENGTH + x] = pieceCode;
	}

	void BoardManager::evaluate(BoardStateData& boardStateData, AnnUtilities::Network& network, AlphaBetaEvaluation& evaluation, bool noMoves)
	{
		if (noMoves)
		{
			if (boardStateData._turn)
			{
				evaluation.whiteWin = true;
				evaluation.evaluatedValue = 0.0f;
			}
			else
			{
				evaluation.blackWin = true;
				evaluation.evaluatedValue = 1.0f;
			}
		}
		else
		{
			setANNInput(boardStateData, network._inputLayer);
			network.propagateForward();
			evaluation.evaluatedValue = network._outputLayer->getOutput()[0];
		}
	}

	void BoardManager::train(AnnUtilities::Network& ann)
	{
		int size = alphaBetaHistory.size();
		float labelWhite;
		float labelBlack;
		float label;
		AlphaBetaEvaluation eval = alphaBetaHistory.top();
		if (eval.whiteWin)
		{
			labelWhite = 0.0f;
			labelBlack = 0.0f;
		}
		else if (eval.blackWin)
		{
			labelWhite = 1.0f;
			labelBlack = 1.0f;
		}
		else
		{
			labelWhite = 1.0f;
			labelBlack = 0.0f;
		}

		for (int i = 0; i < size; ++i)
		{
			eval = alphaBetaHistory.top();
			setANNInput(eval.boardStateData, ann._inputLayer);
			ann.propagateForward();
			if (eval.boardStateData._turn)
			{
				label = labelBlack - (labelBlack - 0.5f) * ((i + 1.0f) / (size + 1.0f));
			}
			else
			{
				label = labelWhite - (labelWhite - 0.5f) * ((i + 1.0f) / (size + 1.0f));
			}
			ann.propagateBackward(&label);
			alphaBetaHistory.pop();
		}
		ann.update(size, 0.2f);
	}

	void BoardManager::setANNInput(BoardStateData& boardStateData, AnnUtilities::Layer* inputLayer)
	{
		float input[ANN_INPUT_LENGTH] = { 0 };
		input[0] = boardStateData._turn;
		if (!boardStateData._kingMoved[0])
		{
			if (!boardStateData._qRookMoved[0])
			{
				input[1] = 1;
			}
			if (!boardStateData._kRookMoved[0])
			{
				input[2] = 1;
			}
		}
		if (!boardStateData._kingMoved[1])
		{
			if (!boardStateData._qRookMoved[1])
			{
				input[3] = 1;
			}
			if (!boardStateData._kRookMoved[1])
			{
				input[4] = 1;
			}
		}
		if (boardStateData._enPassant != -1)
		{
			int epMask = 1 << boardStateData._enPassant;
			for (int i = 0; i < 8; ++i)
			{
				input[5 + 1] = epMask >> i == 1;
			}
		}

		for (int y = 0; y < BOARD_LENGTH; ++y)
		{
			for (int x = 0; x < BOARD_LENGTH; ++x)
			{
				for (int i = 0; i < PIECE_CODE_LENGTH; ++i)
				{
					input[13 + (y * BOARD_LENGTH + x) * PIECE_CODE_LENGTH + i]
						= float(((int)boardStateData._pieces[y * BOARD_LENGTH + x] >> i) & 1);
				}
			}
		}
		inputLayer->setOutputs(input);
	}

	std::vector<BoardStateData> BoardManager::filterMoves(const BoardStateData& boardStateData, std::vector<MoveData>& moves)
	{
		std::vector<BoardStateData> newStates;
		BoardStateData temp;
		PieceCode kingCode = boardStateData._turn ? PieceCode::B_KING : PieceCode::W_KING;
		bool kingThreatened;
		int kingPos[2];
		int kingPosMoved[2];
		findKing(boardStateData._pieces, boardStateData._turn, kingPos);
		for (auto it = moves.begin(); it != moves.end();)
		{
			temp = boardStateData;
			playMove(temp, *it);
			if (temp._pieces[kingPos[0] + kingPos[1] * BOARD_LENGTH] == kingCode)
			{
				kingThreatened = squareThreatened(temp._pieces, temp._turn, kingPos[0], kingPos[1]);
			}
			else
			{
				findKing(temp._pieces, boardStateData._turn, kingPosMoved);
				kingThreatened = squareThreatened(temp._pieces, temp._turn, kingPosMoved[0], kingPosMoved[1]);
			}
			if (kingThreatened)
			{
				it = moves.erase(it);
			}
			else
			{
				newStates.push_back(temp);
				++it;
			}
		}
		return newStates;
	}

	long int BoardManager::zobrishHash(BoardStateData& const boardStateData)
	{
		long int h = 0;
		int piece;
		for (int i = 0; i < BOARD_LENGTH * BOARD_LENGTH; ++i)
		{
			if (boardStateData._pieces[i] != PieceCode::EMPTY)
			{
				piece = (int)boardStateData._pieces[i];
				h = h ^ zobristPieceValues[i + piece];
			}
		}
		return h;
	}

	void BoardManager::findKing(const PieceCode pieces[], bool turn, int* pos)
	{
		PieceCode king = turn ? PieceCode::B_KING : PieceCode::W_KING;
		for (int y = 0; y < BOARD_LENGTH; ++y)
		{
			for (int x = 0; x < BOARD_LENGTH; ++x)
			{
				if (pieces[y * BOARD_LENGTH + x] == king)
				{
					pos[0] = x;
					pos[1] = y;
				}
			}
		}
	}

	void BoardManager::playMove(BoardStateData& boardStateData, const MoveData& move)
	{
		boardStateData._enPassant = -1;
		if (move.enPassant)
		{
			boardStateData._pieces[move.yStart * BOARD_LENGTH + move.xEnd] = PieceCode::EMPTY;
		}
		else if (move.doublePawnMove)
		{
			boardStateData._enPassant = move.xStart;
		}
		if (move.longCastle)
		{
			int y = boardStateData._turn ? BOARD_LENGTH - 1 : 0;
			boardStateData._pieces[y * BOARD_LENGTH + 2] = boardStateData._pieces[y * BOARD_LENGTH + 4];
			boardStateData._pieces[y * BOARD_LENGTH + 3] = boardStateData._pieces[y * BOARD_LENGTH];
			boardStateData._pieces[y * BOARD_LENGTH + 4] = PieceCode::EMPTY;
			boardStateData._pieces[y * BOARD_LENGTH] = PieceCode::EMPTY;
			boardStateData._kingMoved[boardStateData._turn] = true;
			boardStateData._qRookMoved[boardStateData._turn] = true;
		}
		else if (move.shortCastle)
		{
			int y = boardStateData._turn ? BOARD_LENGTH - 1 : 0;
			boardStateData._pieces[y * BOARD_LENGTH + 6] = boardStateData._pieces[y * BOARD_LENGTH + 4];
			boardStateData._pieces[y * BOARD_LENGTH + 5] = boardStateData._pieces[y * BOARD_LENGTH + 7];
			boardStateData._pieces[y * BOARD_LENGTH + 4] = PieceCode::EMPTY;
			boardStateData._pieces[y * BOARD_LENGTH + 7] = PieceCode::EMPTY;
			boardStateData._kingMoved[boardStateData._turn] = true;
			boardStateData._kRookMoved[boardStateData._turn] = true;
		}
		else
		{
			if (boardStateData._pieces[move.yStart * BOARD_LENGTH + move.xStart] == PieceCode::W_KING || boardStateData._pieces[move.yStart * BOARD_LENGTH + move.xStart] == PieceCode::B_KING)
			{
				boardStateData._kingMoved[boardStateData._turn] = true;
			}
			boardStateData._pieces[move.yEnd * BOARD_LENGTH + move.xEnd] = boardStateData._pieces[move.yStart * BOARD_LENGTH + move.xStart];
			boardStateData._pieces[move.yStart * BOARD_LENGTH + move.xStart] = PieceCode::EMPTY;
			if (move.upgrade != PieceCode::EMPTY)
			{
				boardStateData._pieces[move.yEnd * BOARD_LENGTH + move.xEnd] = move.upgrade;
			}
		}
		boardStateData._turn = !boardStateData._turn;
	}

	void BoardManager::genRawMoves(const BoardStateData& boardStateData, std::vector<MoveData>& moves)
	{
		for (int y = 0; y < BOARD_LENGTH; ++y)
		{
			for (int x = 0; x < BOARD_LENGTH; ++x)
			{
				auto& piece = boardStateData._pieces[y * BOARD_LENGTH + x];
				if (piece == PieceCode::EMPTY)
				{
					continue;
				}
				if (((int)piece >> (PIECE_CODE_LENGTH - 1)) != boardStateData._turn)
				{
					continue;
				}
				genRawPieceMoves(boardStateData, moves, x, y);
			}
		}
	}

	std::vector<MoveData> BoardManager::getMoves(const BoardStateData& boardStateData)
	{
		std::vector<MoveData> moves;
		genRawMoves(boardStateData, moves);
		filterMoves(boardStateData, moves);
		return moves;
	}

	void BoardManager::reset()
	{
		for (int i = 0; i < alphaBetaHistory.size(); ++i)
		{
			alphaBetaHistory.pop();
		}
		hashPositions.clear();
	}

	void BoardManager::genRawPieceMoves(const BoardStateData& boardStateData, std::vector<MoveData>& moves, int x, int y)
	{
		switch (boardStateData._pieces[y * BOARD_LENGTH + x])
		{
		case PieceCode::EMPTY:
			break;
		case PieceCode::W_KING: case PieceCode::B_KING:
			genRawMovesKing(moves, boardStateData, x, y);
			break;
		case PieceCode::W_QUEEN: case PieceCode::B_QUEEN:
			genRawMovesQueen(moves, boardStateData._pieces, boardStateData._turn, x, y);
			break;
		case PieceCode::W_ROOK: case PieceCode::B_ROOK:
			genRawMovesRook(moves, boardStateData._pieces, boardStateData._turn, x, y);
			break;
		case PieceCode::W_BISHOP: case PieceCode::B_BISHOP:
			genRawMovesBishop(moves, boardStateData._pieces, boardStateData._turn, x, y);
			break;
		case PieceCode::W_KNIGHT: case PieceCode::B_KNIGHT:
			genRawMovesKnight(moves, boardStateData._pieces, boardStateData._turn, x, y);
			break;
		case PieceCode::W_PAWN: case PieceCode::B_PAWN:
			genRawMovesPawn(moves, boardStateData._pieces, boardStateData._enPassant, boardStateData._turn, x, y);
			break;
		}
	}

	void BoardManager::printBoard(BoardStateData& boardStateData)
	{
		for (int y = BOARD_LENGTH - 1; y > -1; --y)
		{
			for (int x = 0; x < BOARD_LENGTH; ++x)
			{
				std::cout << "[";
				switch (boardStateData._pieces[y * BOARD_LENGTH + x])
				{
				case PieceCode::EMPTY:
					break;
				case PieceCode::B_KING:
					std::cout << "BK";
					break;
				case PieceCode::B_QUEEN:
					std::cout << "BQ";
					break;
				case PieceCode::B_ROOK:
					std::cout << "BR";
					break;
				case PieceCode::B_BISHOP:
					std::cout << "BB";
					break;
				case PieceCode::B_KNIGHT:
					std::cout << "Bk";
					break;
				case PieceCode::B_PAWN:
					std::cout << "BP";
					break;
				case PieceCode::W_KING:
					std::cout << "WK";
					break;
				case PieceCode::W_QUEEN:
					std::cout << "WQ";
					break;
				case PieceCode::W_ROOK:
					std::cout << "WR";
					break;
				case PieceCode::W_BISHOP:
					std::cout << "WB";
					break;
				case PieceCode::W_KNIGHT:
					std::cout << "Wk";
					break;
				case PieceCode::W_PAWN:
					std::cout << "WP";
					break;
				}
				std::cout << "]\t";
			}
			std::cout << "\n";
		}
		std::cout << "\n";
	}

	bool BoardManager::zobristValueExists(long int v)
	{
		for (int i = 0; i < BOARD_LENGTH * BOARD_LENGTH * 12; ++i)
		{
			if (zobristPieceValues[i] == v)
			{
				return true;
			}
		}
		return false;
	}

	void BoardManager::increasePositionMap(BoardStateData& boardStateData, MoveData& move)
	{
		BoardStateData temp = boardStateData;
		playMove(temp, move);
		long int h = zobrishHash(temp);
		if (hashPositions.find(h) == hashPositions.end())
		{
			hashPositions.insert({h, 1});
		}
		else
		{
			++hashPositions.at(h);
		}
	}

	void BoardManager::genRawMovesKing(std::vector<MoveData>& moves, const BoardStateData& boardStateData, int pieceX, int pieceY)
	{
		for (int y = -1; y < 2; ++y)
		{
			for (int x = -1; x < 2; ++x)
			{
				if (!(x == 0 && y == 0)
					&& pieceX + x >= 0
					&& pieceX + x < BOARD_LENGTH
					&& pieceY + y >= 0
					&& pieceY + y < BOARD_LENGTH
					&& (boardStateData._pieces[(pieceY + y) * BOARD_LENGTH + pieceX + x] == PieceCode::EMPTY
						|| ((int)boardStateData._pieces[(pieceY + y) * BOARD_LENGTH + pieceX + x] >> (PIECE_CODE_LENGTH - 1)) != boardStateData._turn)
					)
				{
					MoveData move;
					move.xStart = pieceX;
					move.yStart = pieceY;
					move.xEnd = pieceX + x;
					move.yEnd = pieceY + y;
					moves.push_back(std::move(move));
				}
			}
		}
		// Castles
		if (shortCastleAvailable(boardStateData))
		{
			MoveData sc;
			sc.shortCastle = true;
			moves.push_back(std::move(sc));
		}
		if (longCastleAvailable(boardStateData))
		{
			MoveData lc;
			lc.longCastle = true;
			moves.push_back(std::move(lc));
		}
	}

	void BoardManager::genRawMovesQueen(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY)
	{
		genMovesDir(moves, pieces, -1, 0, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 0, 1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 1, 0, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 0, -1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, -1, -1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, -1, 1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 1, 1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 1, -1, turn, pieceX, pieceY);
	}

	void BoardManager::genRawMovesBishop(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY)
	{
		genMovesDir(moves, pieces, -1, -1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, -1, 1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 1, 1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 1, -1, turn, pieceX, pieceY);
	}

	void BoardManager::genRawMovesRook(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY)
	{
		genMovesDir(moves, pieces, -1, 0, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 0, 1, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 1, 0, turn, pieceX, pieceY);
		genMovesDir(moves, pieces, 0, -1, turn, pieceX, pieceY);
	}

	void BoardManager::genRawMovesKnight(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY)
	{
		int x = pieceX - 2;
		int y = pieceY - 1;
		if (x > -1 && y > -1 && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
		y = pieceY + 1;
		if (x > -1 && y < BOARD_LENGTH && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
		x = pieceX + 2;
		if (x < BOARD_LENGTH && y < BOARD_LENGTH && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
		y = pieceY - 1;
		if (x < BOARD_LENGTH && y < -1 && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
		x = pieceX - 1;
		y = pieceY - 2;
		if (x > -1 && y > -1 && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
		x = pieceX + 1;
		if (x < BOARD_LENGTH && y > -1 && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
		y = pieceY + 2;
		if (x < BOARD_LENGTH && y < BOARD_LENGTH && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
		x = pieceX - 1;
		if (x > -1 && y < BOARD_LENGTH && (pieces[y * BOARD_LENGTH + x] == PieceCode::EMPTY || (int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn))
		{
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
		}
	}

	void BoardManager::genRawMovesPawn(std::vector<MoveData>& moves, const PieceCode pieces[], char enPassant, bool turn, int pieceX, int pieceY)
	{
		int y = pieceY + (turn ? -1 : 1);
		if (pieces[y * BOARD_LENGTH + pieceX] == PieceCode::EMPTY)
		{
			// regular move
			MoveData moveBse;
			moveBse.xStart = pieceX;
			moveBse.yStart = pieceY;
			moveBse.xEnd = pieceX;
			moveBse.yEnd = y;
			if ((turn && y == 0) || (!turn && y == BOARD_LENGTH - 1))
			{
				// upgrade if at the end of the board
				moveBse.upgrade = turn ? PieceCode::B_QUEEN : PieceCode::W_QUEEN;
				MoveData upgradeToKnight;
				upgradeToKnight.xStart = pieceX;
				upgradeToKnight.yStart = pieceY;
				upgradeToKnight.xEnd = pieceX;
				upgradeToKnight.yEnd = y;
				upgradeToKnight.upgrade = turn ? PieceCode::B_KNIGHT : PieceCode::W_KNIGHT;
				moves.push_back(std::move(upgradeToKnight));
			}
			moves.push_back(std::move(moveBse));
			if (!turn && pieceY == 1 && pieces[(pieceY + 2) * BOARD_LENGTH + pieceX] == PieceCode::EMPTY)
			{
				// double move white
				MoveData moveDouble;
				moveDouble.xStart = pieceX;
				moveDouble.yStart = pieceY;
				moveDouble.xEnd = pieceX;
				moveDouble.yEnd = y + 1;
				moveDouble.doublePawnMove = true;
				moves.push_back(std::move(moveDouble));
			}
			else if (turn && pieceY == BOARD_LENGTH - 2 && pieces[(pieceY - 2) * BOARD_LENGTH + pieceX] == PieceCode::EMPTY)
			{
				// double move black
				MoveData moveDouble;
				moveDouble.xStart = pieceX;
				moveDouble.yStart = pieceY;
				moveDouble.xEnd = pieceX;
				moveDouble.yEnd = y - 1;
				moveDouble.doublePawnMove = true;
				moves.push_back(std::move(moveDouble));
			}
		}
		if (pieceX > 0 && pieces[y * BOARD_LENGTH + pieceX - 1] != PieceCode::EMPTY && (int)pieces[y * BOARD_LENGTH + pieceX - 1] >> (PIECE_CODE_LENGTH - 1) != turn)
		{
			// take 
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = pieceX - 1;
			move.yEnd = y;
			if ((turn && y == 0) || (!turn && y == BOARD_LENGTH - 1))
			{
				// upgrade if at the end of the board
				move.upgrade = turn ? PieceCode::B_QUEEN : PieceCode::W_QUEEN;
				MoveData upgradeToKnight;
				upgradeToKnight.xStart = pieceX;
				upgradeToKnight.yStart = pieceY;
				upgradeToKnight.xEnd = pieceX - 1;
				upgradeToKnight.yEnd = y;
				upgradeToKnight.upgrade = turn ? PieceCode::B_KNIGHT : PieceCode::W_KNIGHT;
				moves.push_back(std::move(upgradeToKnight));
			}
			moves.push_back(std::move(move));
		}
		if (pieceX < BOARD_LENGTH - 1 && pieces[y * BOARD_LENGTH + pieceX + 1] != PieceCode::EMPTY && (int)pieces[y * BOARD_LENGTH + pieceX + 1] >> (PIECE_CODE_LENGTH - 1) != turn)
		{
			// take
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = pieceX + 1;
			move.yEnd = y;
			if ((turn && y == 0) || (!turn && y == BOARD_LENGTH - 1))
			{
				// upgrade if at the end of the board
				move.upgrade = turn ? PieceCode::B_QUEEN : PieceCode::W_QUEEN;
				MoveData upgradeToKnight;
				upgradeToKnight.xStart = pieceX;
				upgradeToKnight.yStart = pieceY;
				upgradeToKnight.xEnd = pieceX + 1;
				upgradeToKnight.yEnd = y;
				upgradeToKnight.upgrade = turn ? PieceCode::B_KNIGHT : PieceCode::W_KNIGHT;
				moves.push_back(std::move(upgradeToKnight));
			}
			moves.push_back(std::move(move));
		}
		if (enPassant != -1)
		{
			if (turn && pieceY == 3 && (pieceX == enPassant - 1 || pieceX == enPassant + 1))
			{
				MoveData move;
				move.xStart = pieceX;
				move.yStart = pieceY;
				move.xEnd = enPassant;
				move.yEnd = y;
				move.enPassant = true;
				moves.push_back(std::move(move));
			}
			else if (!turn && pieceY == 4 && (pieceX == enPassant - 1 || pieceX == enPassant + 1))
			{
				MoveData move;
				move.xStart = pieceX;
				move.yStart = pieceY;
				move.xEnd = enPassant;
				move.yEnd = y;
				move.enPassant = true;
				moves.push_back(std::move(move));
			}
		}
	}

	void BoardManager::genMovesDir(std::vector<MoveData>& moves, const PieceCode pieces[], int xDir, int yDir, bool turn, int pieceX, int pieceY)
	{
		int x = pieceX + xDir;
		int y = pieceY + yDir;
		while (x > -1 && x < BOARD_LENGTH && y > -1 && y < BOARD_LENGTH)
		{
			if (pieces[y * BOARD_LENGTH + x] != PieceCode::EMPTY)
			{
				if ((int)pieces[y * BOARD_LENGTH + x] >> (PIECE_CODE_LENGTH - 1) != turn)
				{
					MoveData lastMove;
					lastMove.xStart = pieceX;
					lastMove.yStart = pieceY;
					lastMove.xEnd = x;
					lastMove.yEnd = y;
					moves.push_back(std::move(lastMove));
				}
				break;
			}
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = x;
			move.yEnd = y;
			moves.push_back(std::move(move));
			x += xDir;
			y += yDir;
		}
	}

	bool BoardManager::shortCastleAvailable(const BoardStateData& boardStateData)
	{
		int row = boardStateData._turn ? BOARD_LENGTH - 1 : 0;
		return !boardStateData._kingMoved[boardStateData._turn]
			&& !boardStateData._kRookMoved[boardStateData._turn]
			&& boardStateData._pieces[row + 5] == PieceCode::EMPTY
			&& boardStateData._pieces[row + 6] == PieceCode::EMPTY
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 4, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 5, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 6, row);
	}

	bool BoardManager::longCastleAvailable(const BoardStateData& boardStateData)
	{
		int row = (boardStateData._turn ? 0 : BOARD_LENGTH - 1) * BOARD_LENGTH;
		return !boardStateData._kingMoved[boardStateData._turn]
			&& !boardStateData._qRookMoved[boardStateData._turn]
			&& boardStateData._pieces[row + 2] == PieceCode::EMPTY
			&& boardStateData._pieces[row + 3] == PieceCode::EMPTY
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 4, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 2, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 3, row);
	}

	bool BoardManager::squareThreatened(const PieceCode pieces[], bool turn, int x, int y)
	{
		for (int yBoard = 0; yBoard < BOARD_LENGTH; ++yBoard)
		{
			for (int xBoard = 0; xBoard < BOARD_LENGTH; ++xBoard)
			{
				if (pieces[yBoard * BOARD_LENGTH + xBoard] != PieceCode::EMPTY
					&& (int)pieces[yBoard * BOARD_LENGTH + xBoard] >> (PIECE_CODE_LENGTH - 1) == turn
					&& pieceCanThreatenSquare(pieces, pieces[yBoard * BOARD_LENGTH + xBoard], turn, xBoard, yBoard, x, y))
				{
					return true;
				}
			}
		}
		return false;
	}

	bool BoardManager::pieceCanThreatenSquare(const PieceCode pieces[], PieceCode piece, bool turn, int pieceX, int pieceY, int targetX, int targetY)
	{
		switch (piece)
		{
		case PieceCode::W_KING: case PieceCode::B_KING:
			return kingCanThreatenSquare(pieceX, pieceY, targetX, targetY);
			break;
		case PieceCode::W_QUEEN: case PieceCode::B_QUEEN:
			return queenCanThreatenSquare(pieces, pieceX, pieceY, targetX, targetY);
			break;
		case PieceCode::W_ROOK: case PieceCode::B_ROOK:
			return rookCanThreatenSquare(pieces, pieceX, pieceY, targetX, targetY);
			break;
		case PieceCode::W_BISHOP: case PieceCode::B_BISHOP:
			return bishopCanThreatenSquare(pieces, pieceX, pieceY, targetX, targetY);
			break;
		case PieceCode::W_KNIGHT: case PieceCode::B_KNIGHT:
			return knightCanThreatenSquare(pieces, pieceX, pieceY, targetX, targetY);
			break;
		case PieceCode::W_PAWN: case PieceCode::B_PAWN:
			return pawnCanThreatenSquare(turn, pieceX, pieceY, targetX, targetY);
			break;
		}
		return false;
	}

	bool BoardManager::kingCanThreatenSquare(int pieceX, int pieceY, int targetX, int targetY)
	{
		return abs(targetX - pieceX) <= 1 && abs(targetY - pieceY) <= 1;
	}

	bool BoardManager::queenCanThreatenSquare(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY)
	{
		if (!(targetX == pieceX || targetY == pieceY || (abs(targetX - pieceX) == abs(targetY - pieceY))))
		{
			return false;
		}
		int xDir = 0;
		int yDir = 0;
		if (targetX == pieceX)
		{
			yDir = (targetY - pieceY) / abs(targetY - pieceY);
		}
		else if (targetY == pieceY)
		{
			xDir = (targetX - pieceX) / abs(targetX - pieceX);
		}
		else
		{
			xDir = (targetX - pieceX) / abs(targetX - pieceX);
			yDir = (targetY - pieceY) / abs(targetY - pieceY);
		}
		int x = pieceX + xDir;
		int y = pieceY + yDir;
		while (!(x == targetX && y == targetY))
		{
			if (pieces[y * BOARD_LENGTH + x] != PieceCode::EMPTY)
			{
				return false;
			}
			x += xDir;
			y += yDir;
		}
		return true;
	}

	bool BoardManager::bishopCanThreatenSquare(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY)
	{
		if (abs(targetX - pieceX) != abs(targetY - pieceY))
		{
			return false;
		}
		int xDir = (targetX - pieceX) / abs(targetX - pieceX);
		int yDir = (targetY - pieceY) / abs(targetY - pieceY);
		int x = pieceX + xDir;
		int y = pieceY + yDir;
		while (!(x == targetX && y == targetY))
		{
			if (pieces[y * BOARD_LENGTH + x] != PieceCode::EMPTY)
			{
				return false;
			}
			x += xDir;
			y += yDir;
		}
		return true;
	}

	bool BoardManager::knightCanThreatenSquare(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY)
	{
		if (abs(targetX - pieceX) == 2 && abs(targetY - pieceY) == 1
			|| abs(targetX - pieceX) == 1 && abs(targetY - pieceY) == 2)
		{
			return true;
		}
		return false;
	}

	bool BoardManager::rookCanThreatenSquare(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY)
	{
		if (!(targetX == pieceX || targetY == pieceY))
		{
			return false;
		}
		int xDir = 0;
		int yDir = 0;
		if (targetX != pieceX)
		{
			xDir = (targetX - pieceX) / abs(targetX - pieceX);
		}
		if (targetY != pieceY)
		{
			yDir = (targetY - pieceY) / abs(targetY - pieceY);
		}
		int x = pieceX + xDir;
		int y = pieceY + yDir;
		while (!(x == targetX && y == targetY))
		{
			if (pieces[y * BOARD_LENGTH + x] != PieceCode::EMPTY)
			{
				return false;
			}
			x += xDir;
			y += yDir;
		}
		return true;
	}

	bool BoardManager::pawnCanThreatenSquare(int turn, int pieceX, int pieceY, int targetX, int targetY)
	{
		int yDir = turn ? -1 : 1;
		if (pieceY + yDir == targetY && abs(targetX - pieceX) == 1)
		{
			return true;
		}
		return false;
	}
}