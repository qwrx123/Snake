#ifndef UNICODE
#define UNICODE
#endif 

#include "../headers/scene.h"
#include "../headers/block.h"
#include "../headers/textBox.h"
#include "../headers/switchingTextBox.h"
#include "../headers/cyclingTextBox.h"
#include "../headers/scalingTextButton.h"
#include "../headers/warningTextButton.h"
#include "../headers/selectingTextButton.h"
#include "../headers/inputTextBox.h"
#include "../headers/autoCyclingTextBox.h"
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <iostream>
#include <fstream>
#pragma comment(lib, "Shell32.lib")

scene::scene(HWND hwnd)
	:renderSize(0), renderables(NULL), whenChange(scene::screen::invalid), settings({0}), snakeGame(NULL)
{
    parentHwnd = hwnd;

	HRESULT hr = DWriteCreateFactory(
		DWRITE_FACTORY_TYPE_SHARED,
		__uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(&pDWriteFactory)
	);
    hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);

    if (SUCCEEDED(hr))
    {
        RECT rc;
        GetClientRect(parentHwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(rc.right, rc.bottom);

        pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(), 
            D2D1::HwndRenderTargetProperties(parentHwnd, size),
            &pRenderTarget);

		wcsncpy(settings.name, L"Player", 20);
		initializeLeaderboard();
		changeScene(scene::screen::Title);
    }
}

scene::~scene()
{
    pD2DFactory->Release();
    pRenderTarget->Release();
	clearScene();
}

bool scene::render()
{
    pRenderTarget->BeginDraw();
	for (int i = 0; i < renderSize; i++) {		
		renderables[i]->render();
	}
	if (snakeGame && !gamePaused)
	{
		if(snakeGame->testDeath())
		{
			whenChange = scene::screen::Death;
		}
		if (snakeGame->testWin())
		{
			whenChange = scene::screen::win;
		}
		snakeGame->gameLoop();
		snakeGame->render();
	}
    pRenderTarget->EndDraw();
	checkScene();
    return true;
}

bool scene::resize(RECT size)
{
    D2D1_SIZE_U newSize = D2D1::SizeU(size.right, size.bottom);
	for (int i = 0; i < renderSize; i++) {
		renderables[i]->resize(size);
	}
	if (snakeGame)
	{
		snakeGame->resize(size);
	}
    return SUCCEEDED(pRenderTarget->Resize(newSize));
}

bool scene::changeScene(scene::screen newScreen)
{
	if (renderables)
	{
		clearScene();
	}
	RECT rc;
    GetClientRect(parentHwnd, &rc);

	switch (newScreen)
	{
		case scene::screen::Title:
		{
			renderSize = 22;
			renderables = new block*[renderSize];

			clickableSize = 7;
			clickables = new clickableTextBox*[clickableSize];

			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			//leaderboard
			const wchar_t* leaderboardTitles[3] = {L"Top Scores Slow", L"Top Scores Medium", L"Top Scores Fast"};
			renderables[2] = new autoCyclingTextBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, leaderboardTitles, 3, (const int*)&settings.gameSpeed, settings.gameSpeed);
			for (int i = 0; i < 5; i++) {
				const wchar_t* tempArrayNames[3] = {currentLeaderboard[i].name[0], currentLeaderboard[i].name[1], currentLeaderboard[i].name[2]};
				const wchar_t* tempArrayScores[3] = {currentLeaderboard[i].scoreText[0], currentLeaderboard[i].scoreText[1], currentLeaderboard[i].scoreText[2]};
				renderables[2*i + 3] = new autoCyclingTextBox(pRenderTarget, block::location{(float)(0.4 + 0.05*i), 0.25, 0.25, 0.05}, rc, pDWriteFactory, tempArrayNames, 3, (const int*)&settings.gameSpeed, settings.gameSpeed, block::style::boarderless);
				renderables[2*i + 4] = new autoCyclingTextBox(pRenderTarget, block::location{(float)(0.4 + 0.05*i), 0.5, 0.25, 0.05}, rc, pDWriteFactory, tempArrayScores, 3, (const int*)&settings.gameSpeed, settings.gameSpeed, block::style::boarderless);
			}
			renderables[13] = new block(pRenderTarget, block::location{0.4, 0.25, 0.5, 0.25}, rc, block::style::onlyBoarder);
			//endLeaderboard
			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.7, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Play"
				, &whenChange, scene::screen::Game);
			renderables[14] = (block*)clickables[0];
			//player name button
			clickables[1] = new inputTextBox(pRenderTarget, block::location{0.85, 0.25, 0.5, 0.1}, rc, pDWriteFactory, settings.name, playerNameSize);
			renderables[15] = clickables[1];
			//levelSelect
			clickables[2] = new selectingTextButton(pRenderTarget, block::location{0.2, 0.05, 0.15, 0.15}, rc, pDWriteFactory, L"Board Size", settings.startLevel, 10, 24, &settings.startLevel);
			renderables[16] = clickables[2];
			//endLevelSelect
			//deleteLeaderboard
			clickables[3] = new warningTextBox(pRenderTarget, block::location{0.2, 0.8, 0.15, 0.15}, rc, pDWriteFactory, L"Delete\nLeaderboard", &willDelete);
			renderables[17] = clickables[3];
			//endDeleteLeaderboard
			//effectAdjustment
			clickables[4] = new scalingTextButton(pRenderTarget, block::location{0.45, 0.05, 0.15, 0.2}, rc, pDWriteFactory, L"Effect Volume", &settings.effectVolume, settings.effectVolume);
			renderables[18] = clickables[4];
			//changeSong
			renderables[19] = new textBox(pRenderTarget, block::location{0.5, 0.8, 0.15, 0.1}, rc, pDWriteFactory, L"Change Speed");
			const wchar_t* boxList[3] = {L"Slow", L"Medium", L"Fast"};
			clickables[5] = new cyclingTextBox(pRenderTarget, block::location{0.6, 0.8, 0.15, 0.05}, rc, pDWriteFactory, boxList, ARRAYSIZE(boxList), (int*)&settings.gameSpeed, (int)settings.gameSpeed);
			renderables[20] = clickables[5];
			//controlButton
			clickables[6] = new switchingTextBox(pRenderTarget, block::location{0.7, 0.8, 0.15, 0.1}, rc, pDWriteFactory, L"Controls",
				&whenChange, scene::screen::Control);
			renderables[21] = clickables[6];
			resize(rc);
			break;
		}
		case scene::screen::Control:
		{
			renderSize = 6;
			renderables = new block*[renderSize];

			clickableSize = 1;
			clickables = new clickableTextBox*[clickableSize];

			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Tetris");
			renderables[2] = new textBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, L"Controls");
			renderables[3] = new textBox(pRenderTarget, block::location{0.4, 0.25, 0.5, 0.35}, rc, pDWriteFactory, L"Move Up - W or ↑\nMove Left - A or ←\nMove Right - D or →\nMove Down - S or ↓\nPause Game - P", block::boarderless);
			renderables[4] = new block(pRenderTarget, block::location{0.4, 0.25, 0.5, 0.35}, rc, block::style::onlyBoarder);

			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.8, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Back", &whenChange, scene::screen::Title);
			renderables[5] = clickables[0];
			resize(rc);
			break;
		}
		case scene::screen::Game:
		{
			if (!snakeGame)
			{
				snakeGame = new game(pRenderTarget, pDWriteFactory, rc, settings.name, settings.startLevel, settings.startLevel, settings.gameSpeed);
			}
			snakeGame->resetDeltaTime();
			renderSize = 2;
			renderables = new block*[renderSize];
			clickableSize = 0;
			clickables = new  clickableTextBox*[1];

			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			

			gamePaused = false;
			resize(rc);
			break;
		}
		case scene::screen::Pause:
		{
			snakeGame->populateMetrics(currentMetrics);
			renderSize = 10;
			renderables = new block*[renderSize];
			clickableSize = 1;
			clickables = new  clickableTextBox*[clickableSize];
			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			renderables[2] = new textBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, L"Paused");
			renderables[3] = new textBox(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.1}, rc, pDWriteFactory, currentMetrics.name, block::style::boarderless);
			renderables[4] = new textBox(pRenderTarget, block::location{0.5, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Score", block::style::boarderless);
			renderables[5] = new textBox(pRenderTarget, block::location{0.55, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.scoreText, block::style::boarderless);
			renderables[6] = new textBox(pRenderTarget, block::location{0.6, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Time", block::style::boarderless);
			renderables[7] = new textBox(pRenderTarget, block::location{0.65, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.timeText, block::style::boarderless);
			renderables[8] = new block(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.35}, rc, block::style::onlyBoarder);
			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.8, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Back", &whenChange, scene::screen::Game);
			renderables[9] = clickables[0];
			resize(rc);
			break;
		}
		case scene::screen::Death:
		{
			snakeGame->populateMetrics(currentMetrics);
			calculateLeaderboard();
			delete snakeGame;
			snakeGame = nullptr;
			renderSize = 10;
			renderables = new block*[renderSize];
			clickableSize = 1;
			clickables = new  clickableTextBox*[clickableSize];
			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			renderables[2] = new textBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, L"You Died");
			renderables[3] = new textBox(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.1}, rc, pDWriteFactory, currentMetrics.name, block::style::boarderless);
			renderables[4] = new textBox(pRenderTarget, block::location{0.5, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Score", block::style::boarderless);
			renderables[5] = new textBox(pRenderTarget, block::location{0.55, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.scoreText, block::style::boarderless);
			renderables[6] = new textBox(pRenderTarget, block::location{0.6, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Time", block::style::boarderless);
			renderables[7] = new textBox(pRenderTarget, block::location{0.65, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.timeText, block::style::boarderless);
			renderables[8] = new block(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.35}, rc, block::style::onlyBoarder);
			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.8, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Back", &whenChange, scene::screen::Title);
			renderables[9] = clickables[0];
			resize(rc);
			break;
		}
		case scene::screen::win:
		{
			snakeGame->populateMetrics(currentMetrics);
			calculateLeaderboard();
			delete snakeGame;
			snakeGame = nullptr;
			renderSize = 10;
			renderables = new block*[renderSize];
			clickableSize = 1;
			clickables = new  clickableTextBox*[clickableSize];
			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			renderables[2] = new textBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, L"You Win");
			renderables[3] = new textBox(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.1}, rc, pDWriteFactory, currentMetrics.name, block::style::boarderless);
			renderables[4] = new textBox(pRenderTarget, block::location{0.5, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Score", block::style::boarderless);
			renderables[5] = new textBox(pRenderTarget, block::location{0.55, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.scoreText, block::style::boarderless);
			renderables[6] = new textBox(pRenderTarget, block::location{0.6, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Time", block::style::boarderless);
			renderables[7] = new textBox(pRenderTarget, block::location{0.65, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.timeText, block::style::boarderless);
			renderables[8] = new block(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.35}, rc, block::style::onlyBoarder);
			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.8, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Back", &whenChange, scene::screen::Title);
			renderables[9] = clickables[0];
			resize(rc);
			break;
		}
		default:
		break;
	}
	return true;
}

bool scene::clearScene()
{
	for (int i = 0; i < renderSize; i++) {
		delete renderables[i];
	}
	delete[] renderables;
	renderables = NULL;
	renderSize = 0;
	delete[] clickables;
	clickables = NULL;
	clickableSize = 0;
	return true;
}

void scene::onMove(D2D1_POINT_2F moved)
{
	if (!clickables)
	{
		return;
	}

	for (int i = 0; i < clickableSize; i++)
	{
		clickables[i]->onMove(moved);
	}
}

void scene::onClick(D2D1_POINT_2F clicked)
{
	if (!clickables)
	{
		return;
	}

	for (int i = 0; i < clickableSize; i++)
	{
		clickables[i]->onClick(clicked);
	}
}

void scene::onCapture(D2D1_POINT_2F capture)
{
	if (!clickables)
	{
		return;
	}

	for (int i = 0; i < clickableSize; i++)
	{
		clickables[i]->onCapture(capture);
	}
}

void scene::checkScene()
{
	if (whenChange == scene::screen::invalid)
	{
		return;
	}

	changeScene(whenChange);

	whenChange = scene::screen::invalid;
}

void scene::onKey(wchar_t inputChar)
{
	if ((wchar_t)inputChar == L'P' || (wchar_t)inputChar == L'p')
	{
		gamePaused = true;
		whenChange = scene::screen::Pause;
	}
	if (!clickables)
	{
		return;
	}

	for (int i = 0; i < clickableSize; i++)
	{
		clickables[i]->onKey(inputChar);
	}
}

void scene::keyUp(wchar_t inputChar)
{
	if (snakeGame && !gamePaused)
	{
		snakeGame->onKey(inputChar);
	}
}

void scene::keyDown(wchar_t inputChar)
{
	if (snakeGame && !gamePaused)
	{
		snakeGame->keyDown(inputChar);
	}
}

void scene::calculateLeaderboard()
{
	int pos;
	bool isBigger = false;
	for (int i = 4; i >= 0; i--)
	{
		if (currentMetrics.score > currentLeaderboard[i].score[currentMetrics.speed])
		{
     		isBigger = true;
			pos = i;
		}
	}
	if (isBigger == true)
	{
		for (int i = 3; i >= pos; i--)
		{
			currentLeaderboard[i + 1].score[currentMetrics.speed] = currentLeaderboard[i].score[currentMetrics.speed];
			wcsncpy(currentLeaderboard[i+1].name[currentMetrics.speed], currentLeaderboard[i].name[currentMetrics.speed], 20);
			wcsncpy(currentLeaderboard[i+1].scoreText[currentMetrics.speed], currentLeaderboard[i].scoreText[currentMetrics.speed], 10);
		}
		currentLeaderboard[pos].score[currentMetrics.speed] = currentMetrics.score;
		wcsncpy(currentLeaderboard[pos].scoreText[currentMetrics.speed], currentMetrics.scoreText, 10);
		wcsncpy(currentLeaderboard[pos].name[currentMetrics.speed], currentMetrics.name, 20);
	}
}

void scene::initializeSettings()
{

}

void scene::saveSettings()
{

}

void scene::initializeLeaderboard()
{
	for (int i = 0; i < 5; i++)
	{
		wcsncpy(currentLeaderboard[i].name[0], L"Player1", 20);
		currentLeaderboard[i].score[0] = 0;
		wcsncpy(currentLeaderboard[i].scoreText[0], L"0", 10);
		wcsncpy(currentLeaderboard[i].name[1], L"Player2", 20);
		currentLeaderboard[i].score[1] = 0;
		wcsncpy(currentLeaderboard[i].scoreText[1], L"0", 10);
		wcsncpy(currentLeaderboard[i].name[2], L"Player3", 20);
		currentLeaderboard[i].score[2] = 0;
		wcsncpy(currentLeaderboard[i].scoreText[2], L"0", 10);
	}
}

void scene::saveLeaderboard()
{

}

void scene::deleteLeaderboard()
{

}