#pragma once

const int START_LIFES = 5;
const int WINDOW_SIZE_FOR_TEXT = 1952640;
const float MIN_TEXT_SCALE = 0.366720;

const float CAR_INITIAL_ANGLE = 180.0f;
const float MINIMUM_MOVEMENT_ANGLE = 0.05f;

enum class MyGameState { Running, Paused, Lost, Restart };

class MyGame {

public:

	int gameTime = 0;
	float gameScore = 0;
	int lifes = START_LIFES;

	bool clicked_something = false;
	MyGameState state = MyGameState::Running;

	// Keep track of car
	float lastValidAngle = CAR_INITIAL_ANGLE;
	float lastAngle = CAR_INITIAL_ANGLE;

	void pauseGame();
	void resumeGame();
	void restartGame();
	void clicked();

	void loseLife();

	void update(MyVec3 carPosition);

	// Update Scene
	void renderHUD(VSShaderLib& shaderText, float window_width, float window_height);

private:
	void renderSquareScreen(VSShaderLib& shaderText, float window_width, float window_height);
};