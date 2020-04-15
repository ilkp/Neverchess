#pragma once

#include <bitset>
#include <vector>
#include <Network.h>
#include <Layer.h>
#include <InputData.h>
#include <queue>
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
	};

	struct AlphaBetaEvaluation
	{
		BoardStateData boardStateData;
		MoveData move;
		float evaluatedValue;
		bool whiteWin = false;
		bool blackWin = false;
	};

	class BoardManager
	{
	private:
		std::queue<AlphaBetaEvaluation> alphaBetaHistory;
		std::vector<AnnUtilities::InputData> inputData;

		void setANNInput						(BoardStateData& boardStateData, AnnUtilities::Layer* inputLayer);
		std::vector<BoardStateData> filterMoves	(const BoardStateData& boardStateData, std::vector<MoveData>& moves);
		void findKing							(const PieceCode pieces[], bool turn, int* pos);
		void playMove							(BoardStateData& boardStateData, const MoveData& move);
		void genRawMoves						(const BoardStateData& boardStateData, std::vector<MoveData>& moves);
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
		void genRawPieceMoves					(const BoardStateData& boardStateData, std::vector<MoveData>& moves, int x, int y);
		void printBoard							(BoardStateData& boardStateData);

	public:
		void process							(BoardStateData& boardStateData, AnnUtilities::Network& network, int evaluationDepth, int maxTurns);
		void evaluate							(BoardStateData& boardStateData, AnnUtilities::Network& network, AlphaBetaEvaluation& eval, bool noMoves);
		void initBoardStateDataPieces			(PieceCode pieces[]);
		void placePiece							(PieceCode pieces[], PieceCode pieceCode, int x, int y);
		AlphaBetaEvaluation alphaBeta			(BoardStateData& boardStateData, AnnUtilities::Network& network, int depth, float alpha, float beta);
		std::vector<MoveData> getMoves			(const BoardStateData& boardStateData);
	};
}