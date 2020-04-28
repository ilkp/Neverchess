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
#include <thread>

#define HIGH_LABEL 1.0f
#define LOW_LABEL 0.0f

namespace BoardState
{
	void BoardManager::process(BoardStateData& boardStateData, AnnUtilities::Network& network, int evaluationDepth, int maxTurns)
	{
		std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
		int turn = 0;
		AlphaBetaEvaluation eval;

		while (turn < maxTurns)
		{
			eval = alphaBeta(boardStateData, network, evaluationDepth, -1000.0f, 1000.0f);
			alphaBetaHistory.push(eval);
			playMove(boardStateData, eval.move);
			checkWinner(boardStateData);
			if (whiteWin || blackWin)
			{
				break;
			}
			//std::cout << "(" << eval.move.xStart << ", " << eval.move.yStart << ") -> (" << eval.move.xEnd << ", " << eval.move.yEnd << ")" << std::endl;
			//printBoard(boardStateData);
			if (positionAppeared(boardStateData) > 2)
			{
				std::cout << "Draw by repetition" << std::endl;
				blackWin = false;
				whiteWin = false;
				break;
			}
			++turn;
		}
		std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
		std::cout << "turn: " << turn << ", elapsed time: " << std::chrono::duration_cast<std::chrono::seconds>(end - begin).count()
			<< ", white win = " << whiteWin << ", black win = " << blackWin << std::endl;
		printBoard(boardStateData);
	}

	AlphaBetaEvaluation BoardManager::alphaBeta(BoardStateData& boardStateData, AnnUtilities::Network& network, int depth, float alpha, float beta)
	{
		bool savePosition = false;
		unsigned long int zHash = zobristHash(boardStateData);
		auto transpVal = boardEvaluations.find(zHash);
		if (transpVal != boardEvaluations.end())
		{
			return transpVal->second;
		}
		else
		{
			savePosition = true;
		}

		std::vector<MoveData> moves = genRawMoves(boardStateData);
		std::vector<BoardStateData> newStates = filterMoves(boardStateData, moves);
		AlphaBetaEvaluation evaluation;

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

		float abValue;
		evaluation.move = moves[0];
		evaluation.evaluatedValue = boardStateData._turn ? -1000.0f : 1000.0f;

		for (unsigned int i = 0; i < moves.size(); ++i)
		{
			abValue = alphaBeta(newStates[i], network, depth - 1, alpha, beta).evaluatedValue;
			if (boardStateData._turn == 0)
			{
				if (abValue < evaluation.evaluatedValue)
				{
					evaluation.evaluatedValue = abValue;
					evaluation.move = moves[i];
				}
				beta = std::min(beta, evaluation.evaluatedValue);
			}
			else
			{
				if (abValue > evaluation.evaluatedValue)
				{
					evaluation.evaluatedValue = abValue;
					evaluation.move = moves[i];
				}
				alpha = std::max(alpha, evaluation.evaluatedValue);
			}
			if (alpha >= beta)
			{
				break;
			}
		}
		if (savePosition)
		{
			boardEvaluations.insert({ zHash, evaluation });
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
		std::mt19937 e2(rd());
		std::uniform_int_distribution<unsigned long int> dist(0, ULONG_MAX);
		unsigned long int v;
		for (int i = 0; i < BOARD_LENGTH * BOARD_LENGTH * 12; ++i)
		{
			do
			{
				v = dist(e2);
			} while (zobristValueExists(v));
			zobristPieceValues[i] = v;
		}
		for (int i = 0; i < BOARD_LENGTH + 1; ++i)
		{
			do
			{
				v = dist(e2);
			} while (zobristValueExists(v));
			zobristEnPassantValues[i] = v;
		}
		for (int i = 0; i < 2; ++i)
		{
			do
			{
				v = dist(e2);
			} while (zobristValueExists(v));
			zobristTurnValues[i] = v;
		}
		for (int i = 0; i < 2; ++i)
		{
			do
			{
				v = dist(e2);
			} while (zobristValueExists(v));
			zobristKingMovedValues[i] = v;
		}
		for (int i = 0; i < 2; ++i)
		{
			do
			{
				v = dist(e2);
			} while (zobristValueExists(v));
			zobristQRookMovedValues[i] = v;
		}
		for (int i = 0; i < 2; ++i)
		{
			do
			{
				v = dist(e2);
			} while (zobristValueExists(v));
			zobristKRookMovedValues[i] = v;
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
		// WRITE WEIGHTS
		l = network._inputLayer->_nextLayer;
		while (l != nullptr)
		{
			for (int i = 0; i < l->_layerSize; ++i)
			{
				for (int j = 0; j < l->_prevLayer->_layerSize; ++j)
				{
					file << l->_weights[i * l->_prevLayer->_layerSize + j] << "\n";
				}
			}
			l = l->_nextLayer;
		}
		// WRITE BIASES
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

	//AnnUtilities::Network BoardManager::importANN(std::string fileName)
	//{
		//AnnUtilities::Network ann;
		//std::ifstream file;
		//file.open(fileName);
		//std::string line;
		//std::getline(file, line);
		//int inputSize = std::stoi(line);
		//std::getline(file, line);
		//int hiddenSize = std::stoi(line);
		//std::getline(file, line);
		//int outputSize = std::stoi(line);
		//std::getline(file, line);
		//int nHiddenLayers = std::stoi(line);

		//_inputLayer = new ANNLayer(inputSize);
		//_inputLayer._previousLayer = null;
		//ANNLayer[] hiddenLayers = new ANNLayer[nHiddenLayers];
		//hiddenLayers[0] = new ANNLayer(hiddenSize);
		//hiddenLayers[0]._previousLayer = _inputLayer;
		//for (int i = 1; i < nHiddenLayers; ++i)
		//{
		//	hiddenLayers[i] = new ANNLayer(hiddenSize, hiddenLayers[i - 1]);
		//	hiddenLayers[i - 1]._nextLayer = hiddenLayers[i];
		//}
		//_outputLayer = new ANNLayer(outputSize, hiddenLayers[hiddenLayers.Length - 1]);
		//_outputLayer._nextLayer = null;
		//hiddenLayers[hiddenLayers.Length - 1] = _outputLayer;

		//ANNLayer l = _inputLayer._nextLayer;
		//for (int i = 0; i < hiddenSize; ++i)
		//{
		//	for (int j = 0; j < inputSize; ++j)
		//	{
		//		l._weights[i * inputSize + j] = float.Parse(lines[++line]);
		//	}
		//}
		//l = l._nextLayer;
		//while (l != _outputLayer)
		//{
		//	for (int i = 0; i < hiddenSize; ++i)
		//	{
		//		for (int j = 0; j < hiddenSize; ++j)
		//		{
		//			l._weights[i * hiddenSize + j] = float.Parse(lines[++line]);
		//		}
		//	}
		//	l = l._nextLayer;
		//}
		//for (int i = 0; i < outputSize; ++i)
		//{
		//	for (int j = 0; j < hiddenSize; ++j)
		//	{
		//		_outputLayer._weights[i * hiddenSize + j] = float.Parse(lines[++line]);
		//	}
		//}
		//l = _inputLayer._nextLayer;
		//while (l != null)
		//{
		//	for (int i = 0; i < l._layerSize; ++i)
		//	{
		//		l._biases[i] = float.Parse(lines[++line]);
		//	}
		//	l = l._nextLayer;
		//}
	//}

	void BoardManager::placePiece(PieceCode pieces[], PieceCode pieceCode, int x, int y)
	{
		pieces[y * BOARD_LENGTH + x] = pieceCode;
	}

	void BoardManager::evaluate(const BoardStateData& boardStateData, AnnUtilities::Network& network, AlphaBetaEvaluation& evaluation, bool noMoves)
	{
		if (noMoves)
		{
			if (boardStateData._turn)
			{
				evaluation.evaluatedValue = -1000.0f;
				evaluation.move.xStart = -1;
			}
			else
			{
				evaluation.evaluatedValue = 1000.0f;
				evaluation.move.xStart = -1;
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
		float label;
		float slope;
		float average = (HIGH_LABEL + LOW_LABEL) * 0.5f;
		int size = alphaBetaHistory.size();
		BoardStateData boardStateData;
		resetBoardStateData(boardStateData);
		if (whiteWin)
		{
			label = LOW_LABEL;
		}
		else if (blackWin)
		{
			label = HIGH_LABEL;
		}
		else
		{
			label = average;
		}
		slope = (label - average) / size;
		for (int i = 0; i < size; ++i)
		{
			label = slope * i + average;
			playMove(boardStateData, alphaBetaHistory.front().move);
			setANNInput(boardStateData, ann._inputLayer);
			ann.propagateForward();
			ann.propagateBackward(&label);
			alphaBetaHistory.pop();
		}
		ann.update(size, 0.2f);
	}

	void BoardManager::setANNInput(const BoardStateData& boardStateData, AnnUtilities::Layer* inputLayer)
	{
		int loc = -1;
		inputLayer->_outputs[++loc] = boardStateData._turn;
		if (!boardStateData._kingMoved[0])
		{
			if (!boardStateData._qRookMoved[0])
			{
				inputLayer->_outputs[++loc] = 1.0f;
			}
			else
			{
				inputLayer->_outputs[++loc] = 0.0f;
			}
			if (!boardStateData._kRookMoved[0])
			{
				inputLayer->_outputs[++loc] = 1.0f;
			}
			else
			{
				inputLayer->_outputs[++loc] = 0.0f;
			}
		}
		else
		{
			inputLayer->_outputs[++loc] = 0.0f;
			inputLayer->_outputs[++loc] = 0.0f;
		}
		if (!boardStateData._kingMoved[1])
		{
			if (!boardStateData._qRookMoved[1])
			{
				inputLayer->_outputs[++loc] = 1.0f;
			}
			else
			{
				inputLayer->_outputs[++loc] = 0.0f;
			}
			if (!boardStateData._kRookMoved[1])
			{
				inputLayer->_outputs[++loc] = 1.0f;
			}
			else
			{
				inputLayer->_outputs[++loc] = 0.0f;
			}
		}
		else
		{
			inputLayer->_outputs[++loc] = 0.0f;
			inputLayer->_outputs[++loc] = 0.0f;
		}
		if (boardStateData._enPassant != -1)
		{
			int epMask = 1 << boardStateData._enPassant;
			for (int i = 0; i < 8; ++i)
			{
				inputLayer->_outputs[++loc] = epMask >> i == 1;
			}
		}
		else
		{
			for (int i = 0; i < 8; ++i)
			{
				inputLayer->_outputs[++loc] = 0.0f;
			}
		}

		for (int y = 0; y < BOARD_LENGTH; ++y)
		{
			for (int x = 0; x < BOARD_LENGTH; ++x)
			{
				for (int i = 0; i < PIECE_CODE_LENGTH; ++i)
				{
					inputLayer->_outputs[++loc] = float(((int)boardStateData._pieces[y * BOARD_LENGTH + x] >> i) & 1);
				}
			}
		}
	}

	//Removes moves that leave the king threatened and returns a vector of new possible board states
	std::vector<BoardStateData> BoardManager::filterMoves(const BoardStateData& boardStateData, std::vector<MoveData>& moves)
	{
		std::vector<BoardStateData> newStates;
		BoardStateData temp;
		PieceCode kingCode = boardStateData._turn ? PieceCode::B_KING : PieceCode::W_KING;
		bool kingThreatened;
		int kingPos[2] = { 0, 0 };
		int kingPosMoved[2] = { 0, 0 };
		findKing(boardStateData._pieces, boardStateData._turn, kingPos);
		for (auto it = moves.begin(); it != moves.end();)
		{
			temp.copy(boardStateData);
			playMove(temp, *it);
			if (temp._pieces[kingPos[0] + kingPos[1] * BOARD_LENGTH] == kingCode)
			{
				kingThreatened = squareThreatened(temp._pieces, !temp._turn, kingPos[0], kingPos[1]);
			}
			else
			{
				findKing(temp._pieces, boardStateData._turn, kingPosMoved);
				kingThreatened = squareThreatened(temp._pieces, !temp._turn, kingPosMoved[0], kingPosMoved[1]);
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

	unsigned long int BoardManager::zobristHash(const BoardStateData& boardStateData)
	{
		unsigned long int h = 0;
		PieceCode piece;
		int intPiece = 0;
		h = h ^ zobristTurnValues[(int)boardStateData._turn];
		for (int i = 0; i < BOARD_LENGTH * BOARD_LENGTH; ++i)
		{
			if (boardStateData._pieces[i] != PieceCode::EMPTY)
			{
				piece = boardStateData._pieces[i];
				switch (piece)
				{
				case PieceCode::W_KING:
					intPiece = 0;
					break;
				case PieceCode::W_QUEEN:
					intPiece = 1;
					break;
				case PieceCode::W_PAWN:
					intPiece = 2;
					break;
				case PieceCode::W_KNIGHT:
					intPiece = 3;
					break;
				case PieceCode::W_BISHOP:
					intPiece = 4;
					break;
				case PieceCode::W_ROOK:
					intPiece = 5;
					break;
				case PieceCode::B_KING:
					intPiece = 6;
					break;
				case PieceCode::B_QUEEN:
					intPiece = 7;
					break;
				case PieceCode::B_PAWN:
					intPiece = 8;
					break;
				case PieceCode::B_KNIGHT:
					intPiece = 9;
					break;
				case PieceCode::B_BISHOP:
					intPiece = 10;
					break;
				case PieceCode::B_ROOK:
					intPiece = 11;
					break;
				}
				h = h ^ zobristPieceValues[(i * 12) + intPiece];
			}
		}
		for (int i = 0; i < BOARD_LENGTH + 1; ++i)
		{
			int enPassant = boardStateData._enPassant + 1;
			h = h ^ zobristEnPassantValues[enPassant];
		}
		h = h ^ zobristKingMovedValues[0];
		h = h ^ zobristKingMovedValues[1];
		h = h ^ zobristQRookMovedValues[0];
		h = h ^ zobristQRookMovedValues[1];
		h = h ^ zobristQRookMovedValues[0];
		h = h ^ zobristQRookMovedValues[1];

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
					return;
				}
			}
		}
		std::cout << "KING NOT FOUND" << std::endl;
		int x;
		std::cin >> x;
		pos[0] = -1;
		pos[1] = -1;
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
			PieceCode piece = boardStateData._pieces[move.yStart * BOARD_LENGTH + move.xStart];
			if (piece == PieceCode::W_KING || piece == PieceCode::B_KING)
			{
				boardStateData._kingMoved[boardStateData._turn] = true;
			}
			else if (piece == PieceCode::W_ROOK || piece == PieceCode::B_ROOK)
			{
				if (move.xStart == 0)
				{
					boardStateData._qRookMoved[boardStateData._turn] = true;
				}
				if (move.xStart == 7)
				{
					boardStateData._kRookMoved[boardStateData._turn] = true;
				}
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

	void BoardManager::reset()
	{
		for (unsigned int i = 0; i < alphaBetaHistory.size(); ++i)
		{
			alphaBetaHistory.pop();
		}
		while (!alphaBetaHistory.empty())
		{
			alphaBetaHistory.pop();
		}
		hashPositions.clear();
		boardEvaluations.clear();
		whiteWin = false;
		blackWin = false;
	}

	bool BoardManager::zobristValueExists(unsigned long int v)
	{
		for (int i = 0; i < BOARD_LENGTH * BOARD_LENGTH * 12; ++i)
		{
			if (zobristPieceValues[i] == v)
			{
				return true;
			}
		}
		for (int i = 0; i < BOARD_LENGTH + 1; ++i)
		{
			if (zobristEnPassantValues[i] == v)
			{
				return true;
			}
		}
		if (zobristTurnValues[0] == v || zobristTurnValues[1] == v)
		{
			return true;
		}
		if (zobristKingMovedValues[0] == v || zobristKingMovedValues[1] == v)
		{
			return true;
		}
		if (zobristQRookMovedValues[0] == v || zobristQRookMovedValues[1] == v)
		{
			return true;
		}
		if (zobristKRookMovedValues[0] == v || zobristKRookMovedValues[1] == v)
		{
			return true;
		}
		return false;
	}

	void BoardManager::checkWinner(const BoardStateData& boardStateData)
	{
		PieceCode king = boardStateData._turn ? PieceCode::B_KING : PieceCode::W_KING;
		int kingCoord[2];
		findKing(boardStateData._pieces, boardStateData._turn, kingCoord);
		std::vector<MoveData> moves = genRawMoves(boardStateData);
		filterMoves(boardStateData, moves);
		if (moves.size() == 0)
		{
			if (boardStateData._turn == 0)
			{
				blackWin = true;
			}
			else
			{
				whiteWin = true;
			}
		}
	}

	void BoardManager::printBoard(const BoardStateData& boardStateData) const
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

	int BoardManager::positionAppeared(const BoardStateData& boardStateData)
	{
		unsigned long int h = zobristHash(boardStateData);
		auto it = hashPositions.find(h);
		if (it == hashPositions.end())
		{
			hashPositions.insert({h, 1});
			return 1;
		}
		else
		{
			return ++it->second;
		}
	}

	std::vector<MoveData> BoardManager::genRawMoves(const BoardStateData& boardStateData)
	{
		std::vector<MoveData> moves;
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
		return moves;
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
			sc.xStart = 4;
			sc.xEnd = 6;
			sc.yStart = boardStateData._turn ? 7 : 0;
			sc.yEnd = boardStateData._turn ? 7 : 0;
			moves.push_back(std::move(sc));
		}
		if (longCastleAvailable(boardStateData))
		{
			MoveData lc;
			lc.longCastle = true;
			lc.xStart = 4;
			lc.xEnd = 2;
			lc.yStart = boardStateData._turn ? 7 : 0;
			lc.yEnd = boardStateData._turn ? 7 : 0;
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
		int yDir = turn ? -1 : 1;
		if (pieces[(pieceY + yDir) * BOARD_LENGTH + pieceX] == PieceCode::EMPTY)
		{
			// regular move
			MoveData baseMove;
			baseMove.xStart = pieceX;
			baseMove.yStart = pieceY;
			baseMove.xEnd = pieceX;
			baseMove.yEnd = pieceY + yDir;
			if ((turn && pieceY + yDir == 0) || (!turn && pieceY + yDir == BOARD_LENGTH - 1))
			{
				// upgrade if at the end of the board
				baseMove.upgrade = turn ? PieceCode::B_QUEEN : PieceCode::W_QUEEN;
				MoveData upgradeToKnight;
				upgradeToKnight.xStart = pieceX;
				upgradeToKnight.yStart = pieceY;
				upgradeToKnight.xEnd = pieceX;
				upgradeToKnight.yEnd = pieceY + yDir;
				upgradeToKnight.upgrade = turn ? PieceCode::B_KNIGHT : PieceCode::W_KNIGHT;
				moves.push_back(std::move(upgradeToKnight));
			}
			moves.push_back(std::move(baseMove));
			if ((!turn && pieceY == 1 || turn && pieceY == 6) && pieces[(pieceY + 2 * yDir) * BOARD_LENGTH + pieceX] == PieceCode::EMPTY)
			{
				// double move
				MoveData moveDouble;
				moveDouble.xStart = pieceX;
				moveDouble.yStart = pieceY;
				moveDouble.xEnd = pieceX;
				moveDouble.yEnd = pieceY + 2 * yDir;
				moveDouble.doublePawnMove = true;
				moves.push_back(std::move(moveDouble));
			}
		}
		if (pieceX > 0 && pieces[(pieceY + yDir) * BOARD_LENGTH + pieceX - 1] != PieceCode::EMPTY && ((int)pieces[(pieceY + yDir) * BOARD_LENGTH + pieceX - 1] >> (PIECE_CODE_LENGTH - 1)) != turn)
		{
			// take towards low x coord
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = pieceX - 1;
			move.yEnd = pieceY + yDir;
			if ((turn && pieceY + yDir == 0) || (!turn && pieceY + yDir == 7))
			{
				// upgrade if at the end of the board
				move.upgrade = turn ? PieceCode::B_QUEEN : PieceCode::W_QUEEN;
				MoveData upgradeToKnight;
				upgradeToKnight.xStart = pieceX;
				upgradeToKnight.yStart = pieceY;
				upgradeToKnight.xEnd = pieceX - 1;
				upgradeToKnight.yEnd = pieceY + yDir;
				upgradeToKnight.upgrade = turn ? PieceCode::B_KNIGHT : PieceCode::W_KNIGHT;
				moves.push_back(std::move(upgradeToKnight));
			}
			moves.push_back(std::move(move));
		}
		if (pieceX < BOARD_LENGTH - 1 && pieces[(pieceY + yDir) * BOARD_LENGTH + pieceX + 1] != PieceCode::EMPTY && ((int)pieces[(pieceY + yDir) * BOARD_LENGTH + pieceX + 1] >> (PIECE_CODE_LENGTH - 1)) != turn)
		{
			// take towards high x coord
			MoveData move;
			move.xStart = pieceX;
			move.yStart = pieceY;
			move.xEnd = pieceX + 1;
			move.yEnd = pieceY + yDir;
			if ((turn && pieceY + yDir == 0) || (!turn && pieceY + yDir == 7))
			{
				// upgrade if at the end of the board
				move.upgrade = turn ? PieceCode::B_QUEEN : PieceCode::W_QUEEN;
				MoveData upgradeToKnight;
				upgradeToKnight.xStart = pieceX;
				upgradeToKnight.yStart = pieceY;
				upgradeToKnight.xEnd = pieceX + 1;
				upgradeToKnight.yEnd = pieceY + yDir;
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
				move.yEnd = pieceY + yDir;
				move.enPassant = true;
				moves.push_back(std::move(move));
			}
			else if (!turn && pieceY == 4 && (pieceX == enPassant - 1 || pieceX == enPassant + 1))
			{
				MoveData move;
				move.xStart = pieceX;
				move.yStart = pieceY;
				move.xEnd = enPassant;
				move.yEnd = pieceY + yDir;
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
		int row = boardStateData._turn ? (BOARD_LENGTH - 1) : 0;
		return !boardStateData._kingMoved[boardStateData._turn]
			&& !boardStateData._kRookMoved[boardStateData._turn]
			&& boardStateData._pieces[row * BOARD_LENGTH + 5] == PieceCode::EMPTY
			&& boardStateData._pieces[row * BOARD_LENGTH + 6] == PieceCode::EMPTY
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 4, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 5, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 6, row);
	}

	bool BoardManager::longCastleAvailable(const BoardStateData& boardStateData)
	{
		int row = boardStateData._turn ? (BOARD_LENGTH - 1) : 0;
		return !boardStateData._kingMoved[boardStateData._turn]
			&& !boardStateData._qRookMoved[boardStateData._turn]
			&& boardStateData._pieces[row * BOARD_LENGTH + 1] == PieceCode::EMPTY
			&& boardStateData._pieces[row * BOARD_LENGTH + 2] == PieceCode::EMPTY
			&& boardStateData._pieces[row * BOARD_LENGTH + 3] == PieceCode::EMPTY
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 2, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 3, row)
			&& !squareThreatened(boardStateData._pieces, boardStateData._turn, 4, row);
	}

	bool BoardManager::squareThreatened(const PieceCode pieces[], bool turn, int x, int y)
	{
		for (int yBoard = 0; yBoard < BOARD_LENGTH; ++yBoard)
		{
			for (int xBoard = 0; xBoard < BOARD_LENGTH; ++xBoard)
			{
				if (pieces[yBoard * BOARD_LENGTH + xBoard] != PieceCode::EMPTY
					&& (int)pieces[yBoard * BOARD_LENGTH + xBoard] >> (PIECE_CODE_LENGTH - 1) != turn
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
		int yDir = turn ? 1 : -1;
		if (pieceY + yDir == targetY && abs(targetX - pieceX) == 1)
		{
			return true;
		}
		return false;
	}

	bool BoardManager::moveIsLegal(const BoardStateData& boardStateData, const MoveData move)
	{
		if (move.xStart < 0 || move.xStart >= BOARD_LENGTH || move.yStart < 0 || move.yStart >= BOARD_LENGTH
			|| move.xEnd < 0 || move.xEnd >= BOARD_LENGTH || move.yEnd < 0 || move.yEnd >= BOARD_LENGTH)
		{
			return false;
		}
		PieceCode pieceCode = boardStateData._pieces[move.yStart * BOARD_LENGTH + move.xStart];
		switch (pieceCode)
		{
		case PieceCode::W_KING:
		case PieceCode::B_KING:
			return moveIsLegalKing(move, boardStateData._pieces, boardStateData._turn, boardStateData._kingMoved, boardStateData._kRookMoved, boardStateData._qRookMoved);

		case PieceCode::W_QUEEN:
		case PieceCode::B_QUEEN:
			return moveIsLegalQueen(move, boardStateData._pieces, boardStateData._turn);

		case PieceCode::W_BISHOP:
		case PieceCode::B_BISHOP:
			return moveIsLegalBishop(move, boardStateData._pieces, boardStateData._turn);

		case PieceCode::W_ROOK:
		case PieceCode::B_ROOK:
			return moveIsLegalRook(move, boardStateData._pieces, boardStateData._turn);

		case PieceCode::W_KNIGHT:
		case PieceCode::B_KNIGHT:
			return moveIsLegalKnight(move, boardStateData._pieces, boardStateData._turn);

		case PieceCode::W_PAWN:
		case PieceCode::B_PAWN:
			return moveIsLegalPawn(move, boardStateData._pieces, boardStateData._turn, boardStateData._enPassant);
		}
		return false;
	}

	bool BoardManager::moveIsLegalKing(const MoveData& move, const PieceCode pieces[], bool turn, const bool kingMoved[], const bool kRookMoved[], const bool qRookMoved[])
	{
		int xStart = move.xStart;
		int yStart = move.yStart;
		int xEnd = move.xEnd;
		int yEnd = move.yEnd;
		int row = turn == 0 ? 0 : 7;
		// SHORT CASTLE
		if (move.shortCastle
			&& yStart == row
			&& yEnd == row
			&& xStart == 4
			&& (xEnd == xStart + 2)
			&& !kingMoved[turn]
			&& !kRookMoved[turn]
			&& pieces[row * BOARD_LENGTH + 5] == PieceCode::EMPTY
			&& pieces[row * BOARD_LENGTH + 6] == PieceCode::EMPTY
			&& !squareThreatened(pieces, turn, 4, row)
			&& !squareThreatened(pieces, turn, 5, row)
			&& !squareThreatened(pieces, turn, 6, row))
		{
			return true;
		}
		// LONG CASTLE
		else if (move.longCastle
			&& yStart == row
			&& yEnd == row
			&& xStart == 4
			&& xEnd == xStart - 2
			&& !kingMoved[turn]
			&& !qRookMoved[turn]
			&& pieces[row * BOARD_LENGTH + 1] == PieceCode::EMPTY
			&& pieces[row * BOARD_LENGTH + 2] == PieceCode::EMPTY
			&& pieces[row * BOARD_LENGTH + 3] == PieceCode::EMPTY
			&& !squareThreatened(pieces, turn, 2, row)
			&& !squareThreatened(pieces, turn, 3, row)
			&& !squareThreatened(pieces, turn, 4, row))
		{
			return true;
		}
		else
		{
			return (abs(xEnd - xStart) == 0 || abs(xEnd - xStart) == 1)
				&& (abs(yEnd - yStart) == 0 || abs(yEnd - yStart) == 1)
				&& ((pieces[yEnd * BOARD_LENGTH + xEnd] == PieceCode::EMPTY || ((int)pieces[yEnd * BOARD_LENGTH + xEnd] >> (PIECE_CODE_LENGTH - 1) != turn)));
		}
	}

	bool BoardManager::moveIsLegalQueen(const MoveData& move, const PieceCode pieces[], bool turn)
	{
		int xStart = move.xStart;
		int yStart = move.yStart;
		int xEnd = move.xEnd;
		int yEnd = move.yEnd;
		if (pieces[yEnd * BOARD_LENGTH + xEnd] != PieceCode::EMPTY && ((int)pieces[yEnd * BOARD_LENGTH + xEnd] >> (PIECE_CODE_LENGTH - 1)) == turn)
		{
			return false;
		}
		if (((xStart == xEnd && yStart != yEnd) || (xStart != xEnd && yStart == yEnd)) || (abs(xEnd - xStart) == abs(yEnd - yStart)))
		{
			return squaresAreEmpty(pieces, xStart, yStart, xEnd, yEnd);

		}
		return false;
	}

	bool BoardManager::moveIsLegalBishop(const MoveData& move, const PieceCode pieces[], bool turn)
	{
		int xStart = move.xStart;
		int yStart = move.yStart;
		int xEnd = move.xEnd;
		int yEnd = move.yEnd;
		if (pieces[yEnd * BOARD_LENGTH + xEnd] != PieceCode::EMPTY && ((int)pieces[yEnd * BOARD_LENGTH + xEnd] >> (PIECE_CODE_LENGTH - 1)) == turn)
		{
			return false;
		}
		if (abs(xEnd - xStart) == abs(yEnd - yStart))
		{
			return squaresAreEmpty(pieces, xStart, yStart, xEnd, yEnd);
		}
		return false;
	}

	bool BoardManager::moveIsLegalRook(const MoveData& move, const PieceCode pieces[], bool turn)
	{
		int xStart = move.xStart;
		int yStart = move.yStart;
		int xEnd = move.xEnd;
		int yEnd = move.yEnd;
		if (pieces[yEnd * BOARD_LENGTH + xEnd] != PieceCode::EMPTY && ((int)pieces[yEnd * BOARD_LENGTH + xEnd] >> (PIECE_CODE_LENGTH - 1)) == turn)
		{
			return false;
		}
		if (((xStart == xEnd) && (yStart != yEnd) || (xStart != xEnd) && (yStart == yEnd)))
		{
			return squaresAreEmpty(pieces, xStart, yStart, xEnd, yEnd);
		}
		return false;
	}

	bool BoardManager::moveIsLegalKnight(const MoveData& move, const PieceCode pieces[], bool turn)
	{
		int xStart = move.xStart;
		int yStart = move.yStart;
		int xEnd = move.xEnd;
		int yEnd = move.yEnd;
		if (pieces[yEnd * BOARD_LENGTH + xEnd] != PieceCode::EMPTY && ((int)pieces[yEnd * BOARD_LENGTH + xEnd] >> (PIECE_CODE_LENGTH - 1)) == turn)
		{
			return false;
		}
		if (xEnd == xStart + 2 || xEnd == xStart - 2)
		{
			if (yEnd == yStart + 1 || yEnd == yStart - 1)
			{
				return true;
			}
		}
		else if (xEnd == xStart + 1 || xEnd == xStart - 1)
		{
			if (yEnd == yStart + 2 || yEnd == yStart - 2)
			{
				return true;
			}
		}
		return false;
	}

	bool BoardManager::moveIsLegalPawn(const MoveData& move, const PieceCode pieces[], bool turn, int enPassant)
	{

		int xStart = move.xStart;
		int yStart = move.yStart;
		int xEnd = move.xEnd;
		int yEnd = move.yEnd;
		int dir = turn == 0 ? 1 : -1;
		// EN PASSANT
		if (move.enPassant
			&& xEnd == enPassant
			&& (xEnd == xStart + 1 || xEnd == xStart - 1)
			&& yEnd == yStart + dir
			&& move.yStart == (turn == 0 ? 4 : 3))
		{
			return true;
		}
		// DOUBLE MOVE
		else if (move.doublePawnMove
			&& yStart == (turn == 0 ? 1 : 6)
			&& xEnd == xStart
			&& yEnd == yStart + 2 * dir
			&& pieces[(yStart + dir) * BOARD_LENGTH + xStart] == PieceCode::EMPTY
			&& pieces[(yStart + 2 * dir) * BOARD_LENGTH + xStart] == PieceCode::EMPTY)
		{
			return true;
		}
		// TAKE
		else if ((xEnd == xStart + 1 || xEnd == xStart - 1)
			&& yEnd == yStart + dir
			&& pieces[yEnd * BOARD_LENGTH + xEnd] != PieceCode::EMPTY
			&& ((int)pieces[yEnd * BOARD_LENGTH + xEnd] >> (PIECE_CODE_LENGTH - 1)) != turn)
		{
			return true;
		}
		// NORMAL MOVE
		else if (xEnd == xStart && yEnd == yStart + dir
			&& pieces[yEnd * BOARD_LENGTH + xEnd] == PieceCode::EMPTY)
		{
			return true;
		}
		return false;
	}

	bool BoardManager::squaresAreEmpty(const PieceCode pieces[], int xStart, int yStart, int xEnd, int yEnd)
	{
		int xDir = 0;
		int yDir = 0;
		if (xEnd == xStart)
		{
			yDir = (yEnd - yStart) / abs(yEnd - yStart);
		}
		else if (yEnd == yStart)
		{
			xDir = (xEnd - xStart) / abs(xEnd - xStart);
		}
		else if (abs(xEnd - xStart) == abs(yEnd - yStart))
		{
			xDir = (xEnd - xStart) / abs(xEnd - xStart);
			yDir = (yEnd - yStart) / abs(yEnd - yStart);
		}
		else
		{
			return false;
		}
		int x = xStart + xDir;
		int y = yStart + yDir;
		while (x != xEnd || y != yEnd)
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
}