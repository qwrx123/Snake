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
    head,
    tail,
    apple
};
snakeBlock::direction forward;
snakeBlock::direction backward;
snakeBlock::state currentState;
D2D1_POINT_2F frontLinePoints[2];
D2D1_POINT_2F backLinePoints[2];
D2D1_POINT_2F eyePoints[2];
D2D1_POINT_2F mouthPoints[2];
public:
snakeBlock(ID2D1HwndRenderTarget* renderTarget, block::location setLocation, RECT screenSize, block::style myStyle = block::style::boarder);
void resize(block::location newLocation, RECT screenSize);
void setSnake(snakeBlock::direction startingDirection);
void setSnake();
void setHead(snakeBlock::direction startingDirection);
void setHead();
void setTail(snakeBlock::direction startingDirection);
void setTail();
void killSnake();
void setApple();
void eatApple();
bool setDirection(snakeBlock::direction newDirection);
bool testSnake();
bool testApple();
snakeBlock::direction getDirection();
void moveForward(POINT& blockLocaton);
void moveBackward(POINT& blockLocaton);
virtual bool render();
private:
void moveDirection(POINT& blockLocation, snakeBlock::direction direction);
void setLines();
};