#pragma once

const int START_LIFES = 5;

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