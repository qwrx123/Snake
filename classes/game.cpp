#include "../headers/game.h"
#include "../headers/scene.h"

const block::location game::gameSquareBoundary = {0.21, 0.3, 0.4, 0.74};

game::game(ID2D1HwndRenderTarget* renderTarget, IDWriteFactory* pDWriteFactory, RECT screenSize, int width, int height, float speed)
	:count(0), blocksWidth(width), blocksHeight(height), speed(speed), dead(false)
{
	for (int i = 0; i < blocksWidth; i++)
	{
		for(int j = 0; j < blocksHeight; j++)
		{
			renderScreenBlocks[i][j] = new snakeBlock(renderTarget, {0.1, 0.1, 0.1, 0.1}, screenSize);
		}
	}

	renderScreenBlocks[0][blocksHeight/2]->setSnake(snakeBlock::direction::right);
	renderScreenBlocks[1][blocksHeight/2]->setSnake(snakeBlock::direction::right);
	renderScreenBlocks[2][blocksHeight/2]->setSnake(snakeBlock::direction::right);
	renderScreenBlocks[3][blocksHeight/2]->setSnake(snakeBlock::direction::right);
	renderScreenBlocks[6][2]->setApple();
	head = {3, blocksHeight/2};
	tail = {0, blocksHeight/2};
}

game::~game()
{

}

void game::gameLoop()
{
	countTime.calculateDeltaTime();
	float deltaTime = countTime.getDeltaTime();
	count -= deltaTime;

	if (count <= 0)
	{
		moveSnake();
		count = speed;
	}

	if (keys[L'W'])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::up);
	}
	if (keys[L'D'])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::right);
	}
	if (keys[L'S'])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::down);
	}
	if (keys[L'A'])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::left);
	}
}

void game::render()
{
	for (int i = 0; i < blocksWidth; i++)
	{
		for(int j = 0; j < blocksHeight; j++)
		{
			renderScreenBlocks[i][j]->render();
		}
	}

}

bool game::testDeath()
{
	return false;
}

void game::resize(RECT newScreen)
{
	currentScreenSize = newScreen;
	float blockSize = (newScreen.right * gameSquareBoundary.width)/(float)blocksWidth < (newScreen.bottom * gameSquareBoundary.height)/(float)blocksHeight ? (newScreen.right * gameSquareBoundary.width)/(float)blocksWidth : (newScreen.bottom * gameSquareBoundary.height)/(float)blocksHeight;
	D2D1_POINT_2F middle = {(newScreen.right * gameSquareBoundary.width)/2.0f + newScreen.right * gameSquareBoundary.left, (newScreen.bottom * gameSquareBoundary.height)/2.0f + newScreen.bottom * gameSquareBoundary.top};
	for (int i = 0; i < blocksWidth; i++)
	{
		for(int j = 0; j < blocksHeight; j++)
		{
			renderScreenBlocks[i][j]->resize({((middle.y - blockSize*(blocksHeight/2.0f)) + j*blockSize)/newScreen.bottom, ((middle.x - blockSize*(blocksWidth/2.0f)) + i*blockSize)/newScreen.right, blockSize/newScreen.right, blockSize/newScreen.bottom}, newScreen);
		}
	}

}

void game::onKey(wchar_t inputChar)
{
	keys[inputChar] = false;
}

void game::keyDown(wchar_t inputChar)
{
	keys[inputChar] = true;
}

void game::moveSnake()
{
	bool eaten = false;
	renderScreenBlocks[tail.x][tail.y]->killSnake();
	renderScreenBlocks[tail.x][tail.y]->moveForward(tail);
	snakeBlock::direction currentDirection = renderScreenBlocks[head.x][head.y]->getDirection();
	checkBoundaries();
	if (dead) 
	{
		renderScreenBlocks[tail.x][tail.y]->moveBackward(tail);
		renderScreenBlocks[tail.x][tail.y]->setSnake();
		return;
	}
	renderScreenBlocks[head.x][head.y]->moveForward(head);
	if (renderScreenBlocks[head.x][head.y]->testSnake())
	{
		renderScreenBlocks[tail.x][tail.y]->moveBackward(tail);
		renderScreenBlocks[tail.x][tail.y]->setSnake();
		dead = true;
		return;
	}
	if (renderScreenBlocks[head.x][head.y]->testApple())
	{
		eaten = true;
		renderScreenBlocks[head.x][head.y]->eatApple();
		renderScreenBlocks[tail.x][tail.y]->moveBackward(tail);
		renderScreenBlocks[tail.x][tail.y]->setSnake();
	}
	renderScreenBlocks[head.x][head.y]->setSnake(currentDirection);
	if (eaten)
	{
		calculateApple();
	}
}

void game::checkBoundaries()
{
	POINT testMove = head;
	renderScreenBlocks[testMove.x][testMove.y]->moveForward(testMove);
	if (testMove.x < 0 || testMove.x >= blocksWidth || testMove.y < 0 || testMove.y >= blocksHeight)
	{
		dead = true;
	}
}

void game::calculateApple() {
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> distribW(0, blocksWidth-1);
	std::uniform_int_distribution<> distribH(0, blocksHeight-1);
	int appleX = distribW(gen);
	int appleY = distribH(gen);

	while (renderScreenBlocks[appleX][appleY]->testSnake())
	{
		appleX = distribW(gen);
		appleY = distribH(gen);
	}

	renderScreenBlocks[appleX][appleY]->setApple();
}