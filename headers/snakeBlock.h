#pragma once
#ifndef UNICODE
#define UNICODE
#endif

#include "block.h"

class snakeBlock : public block
{
public:
enum direction
{
    up,
    right,
    down,
    left,
    ammount,
    invalid
};
private:
enum state
{
    tile,
    snake,
    apple
};
snakeBlock::direction forward;
snakeBlock::direction backward;
snakeBlock::state currentState;
public:
snakeBlock(ID2D1HwndRenderTarget* renderTarget, block::location setLocation, RECT screenSize, block::style myStyle = block::style::boarder);
void resize(block::location newLocation, RECT screenSize);
void setSnake(snakeBlock::direction startingDirection);
void setSnake();
void killSnake();
void setApple();
void eatApple();
void setDirection(snakeBlock::direction newDirection);
bool testSnake();
bool testApple();
snakeBlock::direction getDirection();
void moveForward(POINT& blockLocaton);
void moveBackward(POINT& blockLocaton);
private:
void moveDirection(POINT& blockLocation, snakeBlock::direction direction);
};