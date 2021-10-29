#include <string>
#include <assert.h>
#include <stdlib.h>
#include <vector>

// include GLEW to access OpenGL 3.3 functions
#include <GL/glew.h>

// GLUT is the toolkit to interface with the OS
#include <GL/freeglut.h>
#include "AVTmathLib.h"

#include "VSShaderlib.h"
#include "avtFreeType.h"

#include "geometry.h"

#include "game.h"

#define G_PI       3.14159265358979323846f

// ============================ AUXILIARY FUNCTIONS ============================

std::string get_number_lifes_line(int MAX_LIFES, int lifes) {

	std::string line = "";
	for (int i = 0; i < lifes; i++) line = line + "$alive$";
	for (int i = 0; i < MAX_LIFES - lifes; i++) line = line + "$dead$";

	return line;
}

// ============================ =================== ============================


void MyGame::pauseGame() { state = MyGameState::Paused; }
void MyGame::resumeGame() { state = MyGameState::Running; }

void MyGame::restartGame() {

	if (state != MyGameState::Paused && state != MyGameState::Lost) return;

	state = MyGameState::Restart;
	// Restart attributes
	gameTime = 0;
	gameScore = 0;
	lifes = START_LIFES;
}

void MyGame::loseLife() {

	lastAngle = CAR_INITIAL_ANGLE;
	lastValidAngle = CAR_INITIAL_ANGLE;

	if (lifes > 0) lifes = lifes - 1;
	if (lifes == 0) state = MyGameState::Lost;
}

void MyGame::update(MyVec3 carPosition) {

	if (state != MyGameState::Running) return;

	float dot = 1 * carPosition.x + 0 * carPosition.z;
	float det = 1 * carPosition.z - 0 * carPosition.x;
	double angleRadians = atan2(det, dot);
	double angleDegrees = fmod((angleRadians * 180 / G_PI) + 360, 360);

	float movementAngle = angleDegrees - lastAngle;
	float movementPointsAngle = angleDegrees - lastValidAngle;

	// lastAngle = 355		angleDegrees = 5
	// lastAngle = 5		angleDegrees = 10

	// Front Movement
	if (movementAngle > 0.0f || (movementAngle < 0.0f && fabs(movementAngle) > 180.0f)) {
		if (movementPointsAngle > 0.0f || (movementPointsAngle < 0.0f && fabs(movementPointsAngle) > 180.0f)) {
			lastValidAngle = angleDegrees;
			gameScore = gameScore + fabs(movementPointsAngle);
		}
	}

	lastAngle = angleDegrees;
}

void MyGame::renderHUD(VSShaderLib& shaderText, float window_width, float window_height) {

	glBindTexture(GL_TEXTURE_2D, 0);
	//Render text (bitmap fonts)
	glDisable(GL_DEPTH_TEST);
	//the glyph contains background colors and non-transparent for the actual character pixels. So we use the blending
	int m_viewport[4];
	glGetIntegerv(GL_VIEWPORT, m_viewport);

	//viewer looking down at  negative z direction
	pushMatrix(MODEL);
	loadIdentity(MODEL);
	pushMatrix(PROJECTION);
	loadIdentity(PROJECTION);
	pushMatrix(VIEW);
	loadIdentity(VIEW);

	int window_size = window_width * window_height;
	float text_scale = 0.7f * (float(window_size) / float(WINDOW_SIZE_FOR_TEXT));
	if (text_scale < MIN_TEXT_SCALE) text_scale = MIN_TEXT_SCALE;

	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	RenderText(shaderText, get_number_lifes_line(START_LIFES, lifes), 0.02 * window_width, 0.95 * window_height, text_scale, 0.2f, 0.2f, 0.2f);
	std::string score_line = "$coin$ " + std::to_string((int) gameScore);
	RenderText(shaderText, score_line, 0.02 * window_width, 0.95 * window_height - 65.0f * text_scale, text_scale, 0.2, 0.2f, 0.2f);

	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);

	glEnable(GL_DEPTH_TEST);
}