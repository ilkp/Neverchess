#include "BoardState.h"
#include "PieceCode.h"
#include "MoveData.h"

namespace BoardState
{
	void BoardManager::move(BoardStateData& boardState, const MoveData& move)
	{

	}

	std::vector<MoveData> BoardManager::genRawMoves(const BoardStateData& boardStateData)
	{
		std::vector<MoveData> moves;
		for (unsigned char x = 0; x < BOARD_LENGTH; ++x)
		{
			for (unsigned char y = 0; y < BOARD_LENGTH; ++y)
			{
				auto& piece = boardStateData._pieces[y * BOARD_LENGTH + x];
				if (piece == PieceCode::EMPTY)
				{
					continue;
				}
				if ((piece >> PIECE_CODE_LENGTH - 1) != boardStateData._turn)
				{
					continue;
				}
				genRawPieceMoves(boardStateData, moves, x, y);
			}
		}
	}

	void BoardManager::genRawPieceMoves(const BoardStateData& boardStateData, std::vector<MoveData>& moves, unsigned char x, unsigned char y)
	{
		switch (boardStateData._pieces[y * BOARD_LENGTH + x])
		{
		case PieceCode::EMPTY:
			break;
		case PieceCode::W_KING: case PieceCode::B_KING:
			for (int i = -1; i < 2; ++i)
			{
				for (int j = -1; j < 2; ++j)
				{
					if (!(i == 0 && j == 0)
						&& x + i > 0
						&& x + i < BOARD_LENGTH - 1
						&& y + j > 0
						&& y + j < BOARD_LENGTH - 1
						&& (boardStateData._pieces[(y + j) * BOARD_LENGTH + x + i] == PieceCode::EMPTY
							|| (boardStateData._pieces[(y + j) * BOARD_LENGTH + x + i] >> PIECE_CODE_LENGTH - 1) != boardStateData._turn)
						)
					{
						MoveData move;
						move.xStart = x;
						move.yStart = y;
						move.xEnd = x + i;
						move.yEnd = y + j;
						moves.push_back(std::move(move));
					}
				}
			}
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
			break;
		case PieceCode::W_QUEEN: case PieceCode::B_QUEEN:
			break;
		case PieceCode::W_ROOK: case PieceCode::B_ROOK:
			break;
		case PieceCode::W_BISHOP: case PieceCode::B_BISHOP:
			break;
		case PieceCode::W_KNIGHT: case PieceCode::B_KNIGHT:
			break;
		case PieceCode::W_PAWN: case PieceCode::B_PAWN:
			break;
		}
	}

	bool BoardManager::shortCastleAvailable(const BoardStateData& boardStateData)
	{
		unsigned char row = (boardStateData._turn ? 0 : BOARD_LENGTH - 1) * BOARD_LENGTH;
		return !boardStateData._kingMoved[boardStateData._turn]
			&& !boardStateData._kRookMoved[boardStateData._turn]
			&& boardStateData._pieces[row + 5] == PieceCode::EMPTY
			&& boardStateData._pieces[row + 6] == PieceCode::EMPTY
			&& !squareThreatened(boardStateData, 4, row)
			&& !squareThreatened(boardStateData, 5, row)
			&& !squareThreatened(boardStateData, 6, row);
	}

	bool BoardManager::longCastleAvailable(const BoardStateData& boardStateData)
	{
		unsigned char row = (boardStateData._turn ? 0 : BOARD_LENGTH - 1) * BOARD_LENGTH;
		return !boardStateData._kingMoved[boardStateData._turn]
			&& !boardStateData._qRookMoved[boardStateData._turn]
			&& boardStateData._pieces[row + 2] == PieceCode::EMPTY
			&& boardStateData._pieces[row + 3] == PieceCode::EMPTY
			&& !squareThreatened(boardStateData, 4, row)
			&& !squareThreatened(boardStateData, 2, row)
			&& !squareThreatened(boardStateData, 3, row);
	}

	bool BoardManager::squareThreatened(const BoardStateData& boardStateData, unsigned char x, unsigned char y)
	{

	}
}