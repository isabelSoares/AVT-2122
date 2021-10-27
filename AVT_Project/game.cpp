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

#include "game.h"

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

	if (lifes == 0) state = MyGameState::Lost;
	else lifes = lifes - 1;
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

	ortho(m_viewport[0], m_viewport[0] + m_viewport[2] - 1, m_viewport[1], m_viewport[1] + m_viewport[3] - 1, -1, 1);
	std::string lifes_line = "You have '" + std::to_string(lifes) + "' lifes left!";
	RenderText(shaderText, lifes_line, 0.02 * window_width, 0.95 * window_height, 0.7f, 0.2f, 0.2f, 0.2f);
	std::string score_line = "You have '" + std::to_string(gameScore) + "' points!";
	RenderText(shaderText, score_line, 0.02 * window_width, 0.91 * window_height, 0.7f, 0.2, 0.2f, 0.2f);

	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);

	glEnable(GL_DEPTH_TEST);
}