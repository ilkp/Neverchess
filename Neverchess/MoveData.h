#pragma once
#include <bitset>

struct MoveData
{
	PieceCode upgrade = PieceCode::EMPTY;
	bool shortCastle = false;
	bool longCastle = false;
	bool doublePawnMove = false;
	bool enPassant = false;
	int xStart = -1;
	int yStart = -1;
	int xEnd = -1;
	int yEnd = -1;
};