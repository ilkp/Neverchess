#pragma once

#include <vector>
#include <Network.h>
#include <Layer.h>
#include <queue>
#include <unordered_map>
#include <string>
#include "PieceCode.h"
#include "MoveData.h"

enum class PieceCode;

namespace BoardState
{
	static const int PIECE_CODE_LENGTH = 7;
	static const int BOARD_LENGTH = 8;
	static const int ANN_INPUT_LENGTH = BOARD_LENGTH * BOARD_LENGTH * PIECE_CODE_LENGTH
		+ 1 // turn
		+ 2 // white castles possible
		+ 2 // black castles possible
		+ 8; // en passant column

	struct BoardStateData
	{
		PieceCode _pieces[BOARD_LENGTH * BOARD_LENGTH] = { PieceCode::EMPTY };
		bool _turn = 0;
		bool _kingMoved[2] = { false, false };
		bool _kRookMoved[2] = { false, false };
		bool _qRookMoved[2] = { false, false };
		int _enPassant = -1;

		void copy(const BoardStateData& rhs)
		{
			for (int y = 0; y < BOARD_LENGTH; ++y)
			{
				for (int x = 0; x < BOARD_LENGTH; ++x)
				{
					_pieces[y * BOARD_LENGTH + x] = rhs._pieces[y * BOARD_LENGTH + x];
				}
			}
			_turn = rhs._turn;
			_enPassant = rhs._enPassant;
			_kingMoved[0] = rhs._kingMoved[0];
			_kingMoved[1] = rhs._kingMoved[1];
			_kRookMoved[0] = rhs._kRookMoved[0];
			_kRookMoved[1] = rhs._kRookMoved[1];
			_qRookMoved[0] = rhs._qRookMoved[0];
			_qRookMoved[1] = rhs._qRookMoved[1];
		}

		bool operator==(const BoardStateData& other)
		{
			for (int y = 0; y < BOARD_LENGTH; ++y)
			{
				for (int x = 0; x < BOARD_LENGTH; ++x)
				{
					if (_pieces[y * BOARD_LENGTH + x] != other._pieces[y * BOARD_LENGTH + x])
					{
						return false;
					}
				}
			}
			return _turn == other._turn
				&& _kingMoved[0] == other._kingMoved[0] && _kingMoved[1] == other._kingMoved[1]
				&& _kRookMoved[0] == other._kRookMoved[0] && _kRookMoved[1] == other._kRookMoved[1]
				&& _qRookMoved[0] == other._qRookMoved[0] && _qRookMoved[1] == other._qRookMoved[1]
				&& _enPassant == other._enPassant;
		}
	};

	struct AlphaBetaEvaluation
	{
		MoveData move;
		float evaluatedValue;
	};

	class BoardManager
	{
	private:
		unsigned long int zobristPieceValues[BOARD_LENGTH * BOARD_LENGTH * 12] = { 0 };
		unsigned long int zobristTurnValues[2] = { 0 };
		unsigned long int zobristKingMovedValues[2] = { 0 };
		unsigned long int zobristQRookMovedValues[2] = { 0 };
		unsigned long int zobristKRookMovedValues[2] = { 0 };
		unsigned long int zobristEnPassantValues[9] = { 0 };
		std::queue<AlphaBetaEvaluation> alphaBetaHistory;
		std::unordered_map<unsigned long int, int> hashPositions;
		std::unordered_map<unsigned long int, AlphaBetaEvaluation> boardEvaluations;
		int availableThreads = 0;
		bool whiteWin = false;
		bool blackWin = false;

		void setANNInput						(const BoardStateData& boardStateData, AnnUtilities::Layer* inputLayer);
		int positionAppeared					(const BoardStateData& boardStateData);
		void findKing							(const PieceCode pieces[], bool turn, int* pos);
		void playMove							(BoardStateData& boardStateData, const MoveData& move);
		std::vector<BoardStateData> filterMoves	(const BoardStateData& boardStateData, std::vector<MoveData>& moves);
		unsigned long int zobristHash			(const BoardStateData& boardStateData);
		bool zobristValueExists					(unsigned long int v);
		void checkWinner						(const BoardStateData& boardStateData);
		void printBoard							(const BoardStateData& boardStateData) const;

		std::vector<MoveData> genRawMoves		(const BoardStateData& boardStateData);
		void genRawPieceMoves					(const BoardStateData& boardStateData, std::vector<MoveData>& moves, int x, int y);
		void genRawMovesKing					(std::vector<MoveData>& moves, const BoardStateData& boardStateData, int pieceX, int pieceY);
		void genRawMovesQueen					(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY);
		void genRawMovesBishop					(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY);
		void genRawMovesRook					(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY);
		void genRawMovesKnight					(std::vector<MoveData>& moves, const PieceCode pieces[], bool turn, int pieceX, int pieceY);
		void genRawMovesPawn					(std::vector<MoveData>& moves, const PieceCode pieces[], char enPassant, bool turn, int pieceX, int pieceY);
		void genMovesDir						(std::vector<MoveData>& moves, const PieceCode pieces[], int xDir, int yDir, bool turn, int pieceX, int pieceY);

		bool shortCastleAvailable				(const BoardStateData& boardStateData);
		bool longCastleAvailable				(const BoardStateData& boardStateData);

		bool squareThreatened					(const PieceCode pieces[], bool turn, int x, int y);
		bool pieceCanThreatenSquare				(const PieceCode pieces[], PieceCode piece, bool turn, int pieceX, int pieceY, int targetX, int targetY);
		bool kingCanThreatenSquare				(int pieceX, int pieceY, int targetX, int targetY);
		bool queenCanThreatenSquare				(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY);
		bool bishopCanThreatenSquare			(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY);
		bool knightCanThreatenSquare			(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY);
		bool rookCanThreatenSquare				(const PieceCode pieces[], int pieceX, int pieceY, int targetX, int targetY);
		bool pawnCanThreatenSquare				(int turn, int pieceX, int pieceY, int targetX, int targetY);

		bool moveIsLegal						(const BoardStateData& boardStateData, const MoveData move);
		bool moveIsLegalKing					(const MoveData& move, const PieceCode pieces[], bool turn, const bool kingMoved[], const bool kRookMoved[], const bool qRookMoved[]);
		bool moveIsLegalQueen					(const MoveData& move, const PieceCode pieces[], bool turn);
		bool moveIsLegalRook					(const MoveData& move, const PieceCode pieces[], bool turn);
		bool moveIsLegalBishop					(const MoveData& move, const PieceCode pieces[], bool turn);
		bool moveIsLegalKnight					(const MoveData& move, const PieceCode pieces[], bool turn);
		bool moveIsLegalPawn					(const MoveData& move, const PieceCode pieces[], bool turn, int enPassant);
		bool squaresAreEmpty					(const PieceCode pieces[], int xStart, int yStart, int xEnd, int yEnd);

	public:
		void train								(AnnUtilities::Network& ann);
		void process							(BoardStateData& boardStateData, AnnUtilities::Network& network, int evaluationDepth, int maxTurns);
		void evaluate							(const BoardStateData& boardStateData, AnnUtilities::Network& network, AlphaBetaEvaluation& eval, bool noMoves);
		void initBoardStateDataPieces			(PieceCode pieces[]);
		void placePiece							(PieceCode pieces[], PieceCode pieceCode, int x, int y);
		AlphaBetaEvaluation alphaBeta			(BoardStateData& boardStateData, AnnUtilities::Network& network, int depth, float alpha, float beta);
		void reset								();
		void resetBoardStateData				(BoardStateData& boardStateDate);
		void calculateZobristValues				();
		void exportANN							(AnnUtilities::Network& network, std::string fileName);
		//AnnUtilities::Network importANN			(std::string fileName);
	};
}