#pragma once
#ifndef UNICODE
#define UNICODE
#endif 

#include "block.h"
#include "textBox.h"
#include "snakeBlock.h"
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
	int time;
	wchar_t timeText[10];
};
private:
textBox scoreLabel;
textBox scoreDisplay;
textBox timeLabel;
textBox timeDisplay;
gameMetrics currentMetrics;
float runningTime;
int blocksWidth;
int blocksHeight;
snakeBlock* renderScreenBlocks[25][25];
static const block::location gameSquareBoundary;
RECT currentScreenSize;
Time countTime;
float count;
bool keys[0xffff];
POINT head;
POINT tail;
int snakeSize;
int numApples;
float speed;
bool dead;
bool win;
public:
game(ID2D1HwndRenderTarget* renderTarget, IDWriteFactory* pDWriteFactory, RECT screenSize, wchar_t playerName[20], int width, int height, float speed);
~game();
void render();
void gameLoop();
void keyDown(wchar_t inputChar);
void onKey(wchar_t inputChar);
void resize(RECT newScreen);
bool testDeath();
bool testWin();
void resetDeltaTime();
void populateMetrics(game::gameMetrics& populatedMetrics);
private:
void moveSnake();
void checkBoundaries();
void calculateApple();
};