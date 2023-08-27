#include "../headers/game.h"
#include "../headers/scene.h"

const block::location game::gameSquareBoundary = {0.21, 0.3, 0.4, 0.74};

game::game(ID2D1HwndRenderTarget* renderTarget, IDWriteFactory* pDWriteFactory, RECT screenSize, wchar_t playerName[20], int width, int height, float speed)
	:count(0), blocksWidth(width), blocksHeight(height), speed(speed), dead(false), win(false), snakeSize(0), numApples(0), runningTime(0),
	scoreLabel(renderTarget, {0.55, 0.05, 0.15, 0.05}, screenSize, pDWriteFactory, L"Score"), 
    scoreDisplay(renderTarget, {0.6, 0.05, 0.15, 0.05}, screenSize, pDWriteFactory, L"0"),
	timeLabel(renderTarget, {0.65, 0.05, 0.15, 0.05}, screenSize, pDWriteFactory, L"Time"), 
    timeDisplay(renderTarget, {0.7, 0.05, 0.15, 0.05}, screenSize, pDWriteFactory, L"0")
{
	wcsncpy(currentMetrics.name, playerName, 20);
	for (int i = 0; i < blocksWidth; i++)
	{
		for(int j = 0; j < blocksHeight; j++)
		{
			renderScreenBlocks[i][j] = new snakeBlock(renderTarget, {0.1, 0.1, 0.1, 0.1}, screenSize);
		}
	}

	renderScreenBlocks[0][blocksHeight/2]->setTail(snakeBlock::direction::right);
	renderScreenBlocks[1][blocksHeight/2]->setSnake(snakeBlock::direction::right);
	renderScreenBlocks[2][blocksHeight/2]->setSnake(snakeBlock::direction::right);
	renderScreenBlocks[3][blocksHeight/2]->setHead(snakeBlock::direction::right);
	snakeSize = 4;
	currentMetrics.score = 0;
	helper::intToText(currentMetrics.scoreText, 9, currentMetrics.score);
	currentMetrics.time = 0;
	helper::intToText(currentMetrics.timeText, 9, currentMetrics.time);
	calculateApple();

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
	runningTime += deltaTime;
	if (count <= 0)
	{
		moveSnake();
		count += speed;
	}

	if (keys[L'W'] || keys[VK_UP])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::up);
	}
	if (keys[L'D'] || keys[VK_RIGHT])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::right);
	}
	if (keys[L'S'] || keys[VK_DOWN])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::down);
	}
	if (keys[L'A'] || keys[VK_LEFT])
	{
		renderScreenBlocks[head.x][head.y]->setDirection(snakeBlock::direction::left);
	}
	if ((int)runningTime > currentMetrics.time)
	{
		currentMetrics.time = (int)runningTime;
		helper::intToText(currentMetrics.timeText, 9, currentMetrics.time);
		timeDisplay.changeText(currentMetrics.timeText);
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
	scoreLabel.render();
    scoreDisplay.render();
	timeLabel.render();
    timeDisplay.render();
}

bool game::testDeath()
{
	return dead;
}

bool game::testWin()
{
	return win;
}

void game::resetDeltaTime()
{
	countTime.calculateDeltaTime();
	countTime.calculateDeltaTime();
}

void game::populateMetrics(game::gameMetrics& populatedMetrics)
{
	populatedMetrics = currentMetrics;
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
	scoreLabel.resize(newScreen);
    scoreDisplay.resize(newScreen);
	timeLabel.resize(newScreen);
    timeDisplay.resize(newScreen);
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
	renderScreenBlocks[tail.x][tail.y]->setTail();
	snakeBlock::direction currentDirection = renderScreenBlocks[head.x][head.y]->getDirection();
	checkBoundaries();
	if (dead) 
	{
		renderScreenBlocks[tail.x][tail.y]->setSnake();
		renderScreenBlocks[tail.x][tail.y]->moveBackward(tail);
		renderScreenBlocks[tail.x][tail.y]->setTail();
		return;
	}
	renderScreenBlocks[head.x][head.y]->setSnake();
	renderScreenBlocks[head.x][head.y]->moveForward(head);
	if (renderScreenBlocks[head.x][head.y]->testSnake())
	{
		renderScreenBlocks[tail.x][tail.y]->setSnake();
		renderScreenBlocks[tail.x][tail.y]->moveBackward(tail);
		renderScreenBlocks[tail.x][tail.y]->setTail();
		dead = true;
		return;
	}
	if (renderScreenBlocks[head.x][head.y]->testApple())
	{
		eaten = true;
		renderScreenBlocks[head.x][head.y]->eatApple();
		renderScreenBlocks[tail.x][tail.y]->setSnake();
		renderScreenBlocks[tail.x][tail.y]->moveBackward(tail);
		renderScreenBlocks[tail.x][tail.y]->setTail();
		snakeSize++;
		currentMetrics.score++;
		if (snakeSize == blocksWidth * blocksHeight)
		{
			win = true;
		}
		helper::intToText(currentMetrics.scoreText, 9, currentMetrics.score);
		scoreDisplay.changeText(currentMetrics.scoreText);
		numApples--;
	}
	renderScreenBlocks[head.x][head.y]->setHead(currentDirection);
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

void game::calculateApple() 
{
	if (numApples + snakeSize >= blocksWidth * blocksHeight)
	{
		return;
	}
	numApples++;
	std::random_device rd;  //Will be used to obtain a seed for the random number engine
	std::mt19937 gen(rd()); //Standard mersenne_twister_engine seeded with rd()
	std::uniform_int_distribution<> distribW(0, blocksWidth-1);
	std::uniform_int_distribution<> distribH(0, blocksHeight-1);
	int appleX = distribW(gen);
	int appleY = distribH(gen);

	while (renderScreenBlocks[appleX][appleY]->testSnake() || renderScreenBlocks[appleX][appleY]->testApple())
	{
		appleX = distribW(gen);
		appleY = distribH(gen);
	}

	renderScreenBlocks[appleX][appleY]->setApple();
}