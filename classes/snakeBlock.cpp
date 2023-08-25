#include "../headers/snakeBlock.h"

snakeBlock::snakeBlock(ID2D1HwndRenderTarget* renderTarget, block::location setLocation, RECT screenSize, block::style myStyle)
    :block(renderTarget, setLocation, screenSize, myStyle), currentState(snakeBlock::state::tile)
{

}

void snakeBlock::resize(block::location newLocation, RECT screenSize)
{
    coordinates = newLocation;
    block::resize(screenSize);
}

void snakeBlock::setSnake(snakeBlock::direction startingDirection)
{
	forward = startingDirection;
	backward = (snakeBlock::direction)((startingDirection+2)%snakeBlock::direction::ammount);
	block::changeColor(block::colors::green);
	currentState = state::snake;
}

void snakeBlock::setSnake()
{
	block::changeColor(block::colors::green);
	currentState = state::snake;
}

void snakeBlock::killSnake()
{
	currentState = state::tile;
	block::changeColor(block::colors::white);
}

void snakeBlock::setApple()
{
	currentState = state::apple;
	block::changeColor(block::colors::red);
}

void snakeBlock::eatApple()
{
	currentState = state::tile;
	block::changeColor(block::colors::white);
}

void snakeBlock::setDirection(snakeBlock::direction newDirection)
{
	if (newDirection == backward)
	{
		return;
	}
	forward = newDirection;
}

snakeBlock::direction snakeBlock::getDirection()
{
    return forward;
}

void snakeBlock::moveForward(POINT& blockLocation)
{
	moveDirection(blockLocation, forward);
}

void snakeBlock::moveBackward(POINT& blockLocation)
{
	moveDirection(blockLocation, backward);
}

void snakeBlock::moveDirection(POINT& blockLocation, snakeBlock::direction direction)
{
    switch (direction)
    {
        case direction::up:
        {
            blockLocation.y--;
			break;
        }
		case direction::right:
		{
			blockLocation.x++;
			break;
		}
		case direction::down:
		{
			blockLocation.y++;
			break;
		}
		case direction::left:
		{
			blockLocation.x--;
			break;
		}
    }
}

bool snakeBlock::testSnake()
{
	return currentState == state::snake;
}

bool snakeBlock::testApple()
{
	return currentState == state::apple;
}