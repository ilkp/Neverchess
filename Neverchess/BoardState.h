#pragma once

#include <vector>
#include <Network.h>
#include <Layer.h>
#include <stack>
#include <unordered_map>
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

	struct BoardHash
	{
		std::size_t operator()(BoardStateData& const boardStateData) const noexcept
		{
			std::size_t h = 0;
			for (int i = 0; i < BOARD_LENGTH * BOARD_LENGTH; ++i)
			{
				if (i % 2 == 0)
				{
					h += std::hash<int>{}((int)boardStateData._pieces[i] >> 1);
				}
				else
				{
					h += std::hash<int>{}((int)boardStateData._pieces[i] << 1);
				}
			}
			return h
				^ std::hash<bool>()(boardStateData._turn)
				^ std::hash<bool>()(boardStateData._kingMoved[0])
				^ std::hash<bool>()(boardStateData._kingMoved[1])
				^ std::hash<bool>()(boardStateData._kRookMoved[0])
				^ std::hash<bool>()(boardStateData._kRookMoved[1])
				^ std::hash<bool>()(boardStateData._qRookMoved[0])
				^ std::hash<bool>()(boardStateData._qRookMoved[1])
				^ std::hash<int>()(boardStateData._enPassant);
		}
	};

	struct AlphaBetaEvaluation
	{
		BoardStateData boardStateData;
		MoveData move;
		float evaluatedValue;
		bool whiteWin = false;
		bool blackWin = false;
		bool draw = false;
	};

	class BoardManager
	{
	private:
		std::stack<AlphaBetaEvaluation> alphaBetaHistory;
		std::unordered_map<std::size_t, int> hashPositions;

		void setANNInput						(BoardStateData& boardStateData, AnnUtilities::Layer* inputLayer);
		void increasePositionMap				(BoardStateData& boardStateData, MoveData& move);
		void findKing							(const PieceCode pieces[], bool turn, int* pos);
		void playMove							(BoardStateData& boardStateData, const MoveData& move);
		std::vector<BoardStateData> filterMoves	(const BoardStateData& boardStateData, std::vector<MoveData>& moves);

		void genRawMoves						(const BoardStateData& boardStateData, std::vector<MoveData>& moves);
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

		void printBoard							(BoardStateData& boardStateData);

	public:
		void train								(AnnUtilities::Network& ann);
		void process							(BoardStateData& boardStateData, AnnUtilities::Network& network, int evaluationDepth, int maxTurns);
		void evaluate							(BoardStateData& boardStateData, AnnUtilities::Network& network, AlphaBetaEvaluation& eval, bool noMoves);
		void initBoardStateDataPieces			(PieceCode pieces[]);
		void placePiece							(PieceCode pieces[], PieceCode pieceCode, int x, int y);
		AlphaBetaEvaluation alphaBeta			(BoardStateData& boardStateData, AnnUtilities::Network& network, int depth, float alpha, float beta);
		std::vector<MoveData> getMoves			(const BoardStateData& boardStateData);
		void reset								();
		void resetBoardStateData				(BoardStateData& boardStateDate);
	};
}