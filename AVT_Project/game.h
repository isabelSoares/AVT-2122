#pragma once

const int START_LIFES = 5;
const int WINDOW_SIZE_FOR_TEXT = 1952640;
const float MIN_TEXT_SCALE = 0.366720;

enum class MyGameState { Running, Paused, Lost, Restart };

class MyGame {

public:

	int gameTime = 0;
	int gameScore = 0;
	int lifes = START_LIFES;
	MyGameState state = MyGameState::Running;

	void pauseGame();
	void resumeGame();
	void restartGame();

	void loseLife();

	// Update Scene
	void renderHUD(VSShaderLib& shaderText, float window_width, float window_height);
};