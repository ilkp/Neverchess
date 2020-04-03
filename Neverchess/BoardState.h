#pragma once

#include <bitset>
#include <vector>

struct MoveData;
enum class PieceCode;

namespace BoardState
{
	static const unsigned char PIECE_CODE_LENGTH = 7;
	static const unsigned char BOARD_LENGTH = 8;
	static const unsigned char BOARD_LENGTH_SQUARE = BOARD_SIDE_LENGTH * BOARD_SIDE_LENGTH;

	struct BoardStateData
	{
		PieceCode _pieces[BOARD_LENGTH_SQUARE];
		bool _turn = 0;
		bool _kingInCheck[2] = { false, false };
		bool _kingMoved[2] = { false, false };
		bool _kRookMoved[2] = { false, false };
		bool _qRookMoved[2] = { false, false };
		char _enPassant = -1;
	};

	class BoardManager
	{
	private:
		bool shortCastleAvailable(const BoardStateData& boardStateData);
		bool longCastleAvailable(const BoardStateData& boardStateData);
		bool squareThreatened(const BoardStateData& boardStateData, unsigned char x, unsigned char y);

	public:
		void move(BoardStateData& boardState, const MoveData& move);
		std::vector<MoveData> genRawMoves(const BoardStateData& boardStateData);
		void genRawPieceMoves(const BoardStateData& boardStateData, std::vector<MoveData>& moves, unsigned char x, unsigned char y);
	};
}