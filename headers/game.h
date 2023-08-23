#pragma once
#ifndef UNICODE
#define UNICODE
#endif 

#include "block.h"
#include "textBox.h"

#include "helpers.h"
#include "time.h"

#include "songManager.h"

#include <random>

class game
{
public:
struct currentBlocks {
	int x;
	int y;
	int type;
	int turn;
};
// the drop down blocks you see
struct ghostBlocks {
	int x = 0;
	int y = 0;
	int type = 15;
};
struct leaderboard {
	wchar_t name[20];
	int score;
	wchar_t scoreText[10];
};
struct gameMetrics
{
	wchar_t name[20];
	int score;
	wchar_t scoreText[10];
	int level;
	wchar_t levelText[10];
	int lines;
	wchar_t linesText[10];
};
private:
static const block::location gameSquareBoundary;
public:
game();
~game();

};