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
	:renderSize(0), renderables(NULL), whenChange(scene::screen::invalid), settings({0}), snakeGame(NULL),
	settingVersion(0), willDelete(false), manageSongs(hwnd)
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
		initializeSettings();
		initializeLeaderboard();
		manageSongs.changeEffectVolume(0);
		changeScene(scene::screen::Title);
    }
}

scene::~scene()
{
    pD2DFactory->Release();
    pRenderTarget->Release();
	clearScene();
	saveSettings();
	saveLeaderboard();
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
			manageSongs.switchSongType(songManager::songTypes::TITLE);
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
			clickables[2] = new selectingTextButton(pRenderTarget, block::location{0.2, 0.05, 0.15, 0.15}, rc, pDWriteFactory, L"Board Size", settings.boardSize, 10, 24, &settings.boardSize);
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
			manageSongs.switchSongType(songManager::songTypes::GAME);
			if (!snakeGame)
			{
				snakeGame = new game(pRenderTarget, pDWriteFactory, rc, settings.name, settings.boardSize, settings.boardSize, settings.gameSpeed, manageSongs);
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
			manageSongs.switchSongType(songManager::songTypes::NONE);
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
			manageSongs.switchSongType(songManager::songTypes::DEATH);
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
			manageSongs.switchSongType(songManager::songTypes::DEATH);
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
	manageSongs.changeSongVolume(100);
	manageSongs.changeEffectVolume(settings.effectVolume);
	if (willDelete)
	{
		deleteLeaderboard();
		willDelete = false;
		whenChange = scene::screen::Title;
	}
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
	memset(&settings, 0, sizeof(mySettings));
	wchar_t settingsPath[MAX_PATH];
	wchar_t settingsPathAppend[] = L"NGames\\Snake\\Settings";
	wchar_t settingsFileName[] = L"\\settings.bin";
	wchar_t settingsPathFile[MAX_PATH];
	int settingsPointer = 0;
	int tempPointer = 0;
	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_DEFAULT, settingsPathAppend, settingsPath);
	while (settingsPath[tempPointer] != NULL)
	{
		settingsPathFile[settingsPointer] = settingsPath[tempPointer];
		settingsPointer++;
		tempPointer++;
	}
	tempPointer = 0;
	while (settingsFileName[tempPointer] != NULL)
	{
		settingsPathFile[settingsPointer] = settingsFileName[tempPointer];
		settingsPointer++;
		tempPointer++;
	}
	settingsPathFile[settingsPointer] = NULL;
	std::ifstream myfile(settingsPathFile, std::ios::binary);
	if (myfile.is_open())
	{
		myfile.read((char*)&settings.version, 4);
		if (settings.version >= 0)
		{
			myfile.read((char*)&settings.effectVolume, 4);
			myfile.read((char*)&settings.boardSize, 4);
			myfile.read((char*)&settings.gameSpeed, 4);
			myfile.read((char*)&settings.name, 40);
		}
		myfile.close();
	}
	else
	{
		settings.effectVolume = 100;

		settings.boardSize = 10;

		settings.version = settingVersion;

		settings.gameSpeed = game::speeds::slow;

		wcsncpy(settings.name, L"Player", 20);

		std::ofstream myfile(settingsPathFile, std::ios::binary);
		if (myfile.is_open())
		{
			myfile.write((char*)&settings.version, 4);
			myfile.write((char*)&settings.effectVolume, 4);
			myfile.write((char*)&settings.boardSize, 4);
			myfile.write((char*)&settings.gameSpeed, 4);
			myfile.write((char*)&settings.name, 40);
			myfile.close();
		}
	}
}

void scene::saveSettings()
{
	wchar_t settingsPath[MAX_PATH];
	wchar_t settingsPathAppend[] = L"NGames\\Snake\\Settings";
	wchar_t settingsFileName[] = L"\\settings.bin";
	wchar_t settingsPathFile[MAX_PATH];
	int settingsPointer = 0;
	int tempPointer = 0;
	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_DEFAULT, settingsPathAppend, settingsPath);
	while (settingsPath[tempPointer] != NULL)
	{
		settingsPathFile[settingsPointer] = settingsPath[tempPointer];
		settingsPointer++;
		tempPointer++;
	}
	tempPointer = 0;
	while (settingsFileName[tempPointer] != NULL)
	{
		settingsPathFile[settingsPointer] = settingsFileName[tempPointer];
		settingsPointer++;
		tempPointer++;
	}
	settingsPathFile[settingsPointer] = NULL;
	std::ofstream myfile(settingsPathFile, std::ios::binary);
	if (myfile.is_open())
	{
		myfile.write((char*)&settingVersion, 4);
		myfile.write((char*)&settings.effectVolume, 4);
		myfile.write((char*)&settings.boardSize, 4);
		myfile.write((char*)&settings.gameSpeed, 4);
		myfile.write((char*)&settings.name, 40);
		myfile.close();
	}
}

void scene::initializeLeaderboard()
{
	memset(&currentLeaderboard, 0, sizeof(game::leaderboard));
	wchar_t leaderboardPath[MAX_PATH];
	wchar_t leaderboardPathAppend[] = L"NGames\\Snake\\leaderboard";
	wchar_t leaderboardFileName[] = L"\\leaderboard.bin";
	wchar_t leaderboardPathFile[MAX_PATH];
	int leaderboardPointer = 0;
	int tempPointer = 0;
	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_DEFAULT, leaderboardPathAppend, leaderboardPath);
	while (leaderboardPath[tempPointer] != NULL)
	{
		leaderboardPathFile[leaderboardPointer] = leaderboardPath[tempPointer];
		leaderboardPointer++;
		tempPointer++;
	}
	tempPointer = 0;
	while (leaderboardFileName[tempPointer] != NULL)
	{
		leaderboardPathFile[leaderboardPointer] = leaderboardFileName[tempPointer];
		leaderboardPointer++;
		tempPointer++;
	}
	leaderboardPathFile[leaderboardPointer] = NULL;
	std::ifstream myfile(leaderboardPathFile, std::ios::binary);
	if (myfile.is_open())
	{
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					myfile.read((char*)&currentLeaderboard[j].name[i], 40);
					myfile.read((char*)&currentLeaderboard[j].score[i], 4);
					myfile.read((char*)&currentLeaderboard[j].scoreText[i], 20);
				}
			}
			myfile.close();
	}
	else
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				wcsncpy(currentLeaderboard[j].name[i], L"Player", 20);
				currentLeaderboard[j].score[i] = 0;
				wcsncpy(currentLeaderboard[j].scoreText[i], L"0", 10);
			}
		}
		std::ofstream myfile(leaderboardPathFile, std::ios::binary);
		if (myfile.is_open())
		{
			for (int i = 0; i < 3; i++)
			{
				for (int j = 0; j < 5; j++)
				{
					myfile.write((char*)&currentLeaderboard[j].name[i], 40);
					myfile.write((char*)&currentLeaderboard[j].score[i], 4);
					myfile.write((char*)&currentLeaderboard[j].scoreText[i], 20);
				}
			}
			myfile.close();
		}
	}
}

void scene::saveLeaderboard()
{
	wchar_t leaderboardPath[MAX_PATH];
	wchar_t leaderboardPathAppend[] = L"NGames\\Snake\\leaderboard";
	wchar_t leaderboardFileName[] = L"\\leaderboard.bin";
	wchar_t leaderboardPathFile[MAX_PATH];
	int leaderboardPointer = 0;
	int tempPointer = 0;
	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_DEFAULT, leaderboardPathAppend, leaderboardPath);
	while (leaderboardPath[tempPointer] != NULL)
	{
		leaderboardPathFile[leaderboardPointer] = leaderboardPath[tempPointer];
		leaderboardPointer++;
		tempPointer++;
	}
	tempPointer = 0;
	while (leaderboardFileName[tempPointer] != NULL)
	{
		leaderboardPathFile[leaderboardPointer] = leaderboardFileName[tempPointer];
		leaderboardPointer++;
		tempPointer++;
	}
	leaderboardPathFile[leaderboardPointer] = NULL;
	std::ofstream myfile(leaderboardPathFile, std::ios::binary);
	if (myfile.is_open())
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				myfile.write((char*)&currentLeaderboard[j].name[i], 40);
				myfile.write((char*)&currentLeaderboard[j].score[i], 4);
				myfile.write((char*)&currentLeaderboard[j].scoreText[i], 20);
			}
		}
		myfile.close();
	}
}

void scene::deleteLeaderboard()
{
	wchar_t leaderboardPath[MAX_PATH];
	wchar_t leaderboardPathAppend[] = L"NGames\\Snake\\leaderboard";
	wchar_t leaderboardFileName[] = L"\\leaderboard.bin";
	wchar_t leaderboardPathFile[MAX_PATH];
	int leaderboardPointer = 0;
	int tempPointer = 0;
	SHGetFolderPathAndSubDir(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_DEFAULT, leaderboardPathAppend, leaderboardPath);
	while (leaderboardPath[tempPointer] != NULL)
	{
		leaderboardPathFile[leaderboardPointer] = leaderboardPath[tempPointer];
		leaderboardPointer++;
		tempPointer++;
	}
	tempPointer = 0;
	while (leaderboardFileName[tempPointer] != NULL)
	{
		leaderboardPathFile[leaderboardPointer] = leaderboardFileName[tempPointer];
		leaderboardPointer++;
		tempPointer++;
	}
	leaderboardPathFile[leaderboardPointer] = NULL;
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			wcsncpy(currentLeaderboard[j].name[i], L"Player", 20);
			currentLeaderboard[j].score[i] = 0;
			wcsncpy(currentLeaderboard[j].scoreText[i], L"0", 10);
		}
	}
	std::ofstream myfile(leaderboardPathFile, std::ios::binary);
	if (myfile.is_open())
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 5; j++)
			{
				myfile.write((char*)&currentLeaderboard[j].name[i], 40);
				myfile.write((char*)&currentLeaderboard[j].score[i], 4);
				myfile.write((char*)&currentLeaderboard[j].scoreText[i], 20);
			}
		}
		myfile.close();
	}
}