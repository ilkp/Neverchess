#pragma once

struct MoveData
{
	bool shortCastle = false;
	bool longCastle = false;
	bool doublePawnMove = false;
	unsigned char xStart;
	unsigned char yStart;
	unsigned char xEnd;
	unsigned char yEnd;
};
