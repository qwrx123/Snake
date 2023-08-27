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
			renderSize = 23;
			renderables = new block*[renderSize];

			clickableSize = 8;
			clickables = new clickableTextBox*[clickableSize];

			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			//leaderboard
			renderables[2] = new textBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, L"Top Scores");
			for (int i = 0; i < 5; i++) {
				renderables[2*i + 3] = new textBox(pRenderTarget, block::location{(float)(0.4 + 0.05*i), 0.25, 0.25, 0.05}, rc, pDWriteFactory, L"Player", block::style::boarderless);
				renderables[2*i + 4] = new textBox(pRenderTarget, block::location{(float)(0.4 + 0.05*i), 0.5, 0.25, 0.05}, rc, pDWriteFactory, L"0", block::style::boarderless);
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
			//volumeAdjustment
			clickables[4] = new scalingTextButton(pRenderTarget, block::location{0.4, 0.05, 0.15, 0.2}, rc, pDWriteFactory, L"Sound Volume", &settings.soundVolume, settings.soundVolume);
			renderables[18] = clickables[4];
			//effectAdjustment
			clickables[5] = new scalingTextButton(pRenderTarget, block::location{0.6, 0.05, 0.15, 0.2}, rc, pDWriteFactory, L"Effect Volume", &settings.effectVolume, settings.effectVolume);
			renderables[19] = clickables[5];
			//changeSong
			renderables[20] = new textBox(pRenderTarget, block::location{0.5, 0.8, 0.15, 0.1}, rc, pDWriteFactory, L"Change Song");
			const wchar_t* boxList[3] = {L"Number 1", L"Number 2", L"Number 3"};
			clickables[6] = new cyclingTextBox(pRenderTarget, block::location{0.6, 0.8, 0.15, 0.05}, rc, pDWriteFactory, boxList, ARRAYSIZE(boxList), &settings.songChoice, settings.songChoice);
			renderables[21] = clickables[6];
			//controlButton
			clickables[7] = new switchingTextBox(pRenderTarget, block::location{0.7, 0.8, 0.15, 0.1}, rc, pDWriteFactory, L"Controls",
				&whenChange, scene::screen::Control);
			renderables[22] = clickables[7];
			resize(rc);
			break;
		}
		case scene::screen::Control:
		{
			renderSize = 3;
			renderables = new block*[renderSize];

			clickableSize = 1;
			clickables = new clickableTextBox*[clickableSize];

			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");

			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.8, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Back", &whenChange, scene::screen::Title);
			renderables[2] = clickables[0];
			resize(rc);
			break;
		}
		case scene::screen::Game:
		{
			if (!snakeGame)
			{
				snakeGame = new game(pRenderTarget, pDWriteFactory, rc, settings.name, settings.startLevel, settings.startLevel, 0.5f);
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
			renderables[3] = new textBox(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.name, block::style::boarderless);
			renderables[4] = new textBox(pRenderTarget, block::location{0.45, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Score", block::style::boarderless);
			renderables[5] = new textBox(pRenderTarget, block::location{0.5, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.scoreText, block::style::boarderless);
			renderables[6] = new textBox(pRenderTarget, block::location{0.55, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Time", block::style::boarderless);
			renderables[7] = new textBox(pRenderTarget, block::location{0.6, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.timeText, block::style::boarderless);
			renderables[8] = new block(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.35}, rc, block::style::onlyBoarder);
			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.8, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Back", &whenChange, scene::screen::Game);
			renderables[9] = clickables[0];
			resize(rc);
			break;
		}
		case scene::screen::Death:
		{
			snakeGame->populateMetrics(currentMetrics);
			delete snakeGame;
			snakeGame = nullptr;
			renderSize = 10;
			renderables = new block*[renderSize];
			clickableSize = 1;
			clickables = new  clickableTextBox*[clickableSize];
			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			renderables[2] = new textBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, L"You Died");
			renderables[3] = new textBox(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.name, block::style::boarderless);
			renderables[4] = new textBox(pRenderTarget, block::location{0.45, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Score", block::style::boarderless);
			renderables[5] = new textBox(pRenderTarget, block::location{0.5, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.scoreText, block::style::boarderless);
			renderables[6] = new textBox(pRenderTarget, block::location{0.55, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Time", block::style::boarderless);
			renderables[7] = new textBox(pRenderTarget, block::location{0.6, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.timeText, block::style::boarderless);
			renderables[8] = new block(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.35}, rc, block::style::onlyBoarder);
			clickables[0] = new switchingTextBox(pRenderTarget, block::location{0.8, 0.4, 0.2, 0.1}, rc, pDWriteFactory, L"Back", &whenChange, scene::screen::Title);
			renderables[9] = clickables[0];
			resize(rc);
			break;
		}
		case scene::screen::win:
		{
			snakeGame->populateMetrics(currentMetrics);
			delete snakeGame;
			snakeGame = nullptr;
			renderSize = 10;
			renderables = new block*[renderSize];
			clickableSize = 1;
			clickables = new  clickableTextBox*[clickableSize];
			renderables[0] = new block(pRenderTarget, block::location{0.0, 0.0, 1.0, 1.0}, rc, block::style::boarderless);
			renderables[1] = new textBox(pRenderTarget, block::location{0.05, 0.3, 0.4, 0.15}, rc, pDWriteFactory, L"Snake");
			renderables[2] = new textBox(pRenderTarget, block::location{0.275, 0.35, 0.3, 0.1}, rc, pDWriteFactory, L"You Win");
			renderables[3] = new textBox(pRenderTarget, block::location{0.4, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.name, block::style::boarderless);
			renderables[4] = new textBox(pRenderTarget, block::location{0.45, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Score", block::style::boarderless);
			renderables[5] = new textBox(pRenderTarget, block::location{0.5, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.scoreText, block::style::boarderless);
			renderables[6] = new textBox(pRenderTarget, block::location{0.55, 0.35, 0.3, 0.05}, rc, pDWriteFactory, L"Time", block::style::boarderless);
			renderables[7] = new textBox(pRenderTarget, block::location{0.6, 0.35, 0.3, 0.05}, rc, pDWriteFactory, currentMetrics.timeText, block::style::boarderless);
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

