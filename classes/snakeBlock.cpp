#include "../headers/snakeBlock.h"

snakeBlock::snakeBlock(ID2D1HwndRenderTarget* renderTarget, block::location setLocation, RECT screenSize, block::style myStyle)
    :block(renderTarget, setLocation, screenSize, myStyle), currentState(snakeBlock::state::tile)
{

}

void snakeBlock::resize(block::location newLocation, RECT screenSize)
{
    coordinates = newLocation;
    block::resize(screenSize);
	setLines();
}

bool snakeBlock::render()
{
	block::render();
	if (currentState == state::snake || currentState == state::tail)
	{
		pRenderTarget->DrawLine(frontLinePoints[0], frontLinePoints[1], block::paintColor[block::colors::black], 2.0);
	}
	if (currentState == state::snake || currentState == state::head)
	{
		pRenderTarget->DrawLine(backLinePoints[0], backLinePoints[1], block::paintColor[block::colors::black], 2.0);
	}
	if (currentState == state::head)
	{
		pRenderTarget->DrawEllipse({eyePoints[0], 1, 1}, block::paintColor[block::colors::black], 2.0);
		pRenderTarget->DrawEllipse({eyePoints[1], 1, 1}, block::paintColor[block::colors::black], 2.0);
		pRenderTarget->DrawLine(mouthPoints[0], mouthPoints[1], block::paintColor[block::colors::black], 2.0);
	}
	return true;
}

void snakeBlock::setSnake(snakeBlock::direction startingDirection)
{
	forward = startingDirection;
	backward = (snakeBlock::direction)((startingDirection+2)%snakeBlock::direction::ammount);
	block::changeColor(block::colors::green);
	currentState = state::snake;
	setLines();
}

void snakeBlock::setSnake()
{
	block::changeColor(block::colors::green);
	currentState = state::snake;
}

void snakeBlock::setHead(snakeBlock::direction startingDirection)
{
	setSnake(startingDirection);
	currentState = state::head;
}

void snakeBlock::setHead()
{
	setSnake();
	currentState = state::head;
}

void snakeBlock::setTail(snakeBlock::direction startingDirection)
{
	setSnake(startingDirection);
	currentState = state::tail;
}

void snakeBlock::setTail()
{
	setSnake();
	currentState = state::tail;
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
	setLines();
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

void snakeBlock::setLines()
{
	float halfBoxHeight = (drawnRect.bottom - drawnRect.top)/2;
	float halfBoxWidth = (drawnRect.right - drawnRect.left)/2;
	frontLinePoints[0] = {drawnRect.left + halfBoxWidth, drawnRect.top + halfBoxHeight};
	backLinePoints[0] = frontLinePoints[0];
	frontLinePoints[1] = frontLinePoints[0];
	backLinePoints[1] = frontLinePoints[0];
	eyePoints[0] = frontLinePoints[0];
	eyePoints[1] = frontLinePoints[1];
    switch (forward)
    {
        case direction::up:
        {
			frontLinePoints[1].y -= halfBoxHeight;
			eyePoints[0].y -= halfBoxHeight/2;
			eyePoints[0].x -= halfBoxHeight/2;
			eyePoints[1] = eyePoints[0];
			eyePoints[1].x += halfBoxHeight;
			mouthPoints[0] = eyePoints[0];
			mouthPoints[1] = eyePoints[1];
			mouthPoints[0].y -= halfBoxHeight/4;
			mouthPoints[1].y -= halfBoxHeight/4;
			mouthPoints[0].x += halfBoxHeight/4;
			mouthPoints[1].x -= halfBoxHeight/4;
			break;
        }
		case direction::right:
		{
			frontLinePoints[1].x += halfBoxWidth;
			eyePoints[0].x += halfBoxHeight/2;
			eyePoints[0].y -= halfBoxHeight/2;
			eyePoints[1] = eyePoints[0];
			eyePoints[1].y += halfBoxHeight;
			mouthPoints[0] = eyePoints[0];
			mouthPoints[1] = eyePoints[1];
			mouthPoints[0].y += halfBoxHeight/4;
			mouthPoints[1].y -= halfBoxHeight/4;
			mouthPoints[0].x += halfBoxHeight/4;
			mouthPoints[1].x += halfBoxHeight/4;
			break;
		}
		case direction::down:
		{
			frontLinePoints[1].y += halfBoxHeight;
			eyePoints[0].y += halfBoxHeight/2;
			eyePoints[0].x -= halfBoxHeight/2;
			eyePoints[1] = eyePoints[0];
			eyePoints[1].x += halfBoxHeight;
			mouthPoints[0] = eyePoints[0];
			mouthPoints[1] = eyePoints[1];
			mouthPoints[0].y += halfBoxHeight/4;
			mouthPoints[1].y += halfBoxHeight/4;
			mouthPoints[0].x += halfBoxHeight/4;
			mouthPoints[1].x -= halfBoxHeight/4;
			break;
		}
		case direction::left:
		{
			frontLinePoints[1].x -= halfBoxWidth;
			eyePoints[0].x -= halfBoxHeight/2;
			eyePoints[0].y -= halfBoxHeight/2;
			eyePoints[1] = eyePoints[0];
			eyePoints[1].y += halfBoxHeight;
			mouthPoints[0] = eyePoints[0];
			mouthPoints[1] = eyePoints[1];
			mouthPoints[0].y += halfBoxHeight/4;
			mouthPoints[1].y -= halfBoxHeight/4;
			mouthPoints[0].x -= halfBoxHeight/4;
			mouthPoints[1].x -= halfBoxHeight/4;
			break;
		}
    }
    switch (backward)
    {
        case direction::up:
        {
			backLinePoints[1].y -= halfBoxHeight;
			break;
        }
		case direction::right:
		{
			backLinePoints[1].x += halfBoxWidth;
			break;
		}
		case direction::down:
		{
			backLinePoints[1].y += halfBoxHeight;
			break;
		}
		case direction::left:
		{
			backLinePoints[1].x -= halfBoxWidth;
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