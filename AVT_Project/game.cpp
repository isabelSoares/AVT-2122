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

extern float mCompMatrix[COUNT_COMPUTED_MATRICES][16];
extern float mNormal3x3[9];

extern int timesToGenerateParticles;
extern const int TIME_PARTICLES = 200;

// ============================ AUXILIARY FUNCTIONS ============================

std::string get_number_lifes_line(int MAX_LIFES, int lifes) {

	std::string line = "";
	for (int i = 0; i < lifes; i++) line = line + "$alive$";
	for (int i = 0; i < MAX_LIFES - lifes; i++) line = line + "$dead$";

	return line;
}

void renderQuad(VSShaderLib& shaderText, float xpos, float ypos, float width, float height, float colorR, float colorG, float colorB, float colorAlpha) {

	unsigned int VAO, VBO;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	
	GLuint programIndex = shaderText.getProgramIndex();
	glUseProgram(programIndex);

	computeDerivedMatrix(PROJ_VIEW_MODEL);
	glUniformMatrix4fv(glGetUniformLocation(programIndex, "m_pvm"), 1, GL_FALSE, mCompMatrix[PROJ_VIEW_MODEL]);

	glUniform1i(glGetUniformLocation(programIndex, "geometry"), true);
	glUniform4f(glGetUniformLocation(programIndex, "geometryColor"), colorR, colorG, colorB, colorAlpha);
	glBindVertexArray(VAO);

	float vertices[6][4] = {
		{ xpos,         ypos + height,   0.0f, 0.0f },
		{ xpos,         ypos,            0.0f, 1.0f },
		{ xpos + width, ypos,            1.0f, 1.0f },

		{ xpos,         ypos + height,   0.0f, 0.0f },
		{ xpos + width, ypos,            1.0f, 1.0f },
		{ xpos + width, ypos + height,   1.0f, 0.0f }
	};

	// update content of VBO memory
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glBindVertexArray(0);
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

	// Front Movement
	if (movementAngle > 0.0f || (movementAngle < 0.0f && fabs(movementAngle) > 180.0f)) {
		if (movementPointsAngle > 0.0f || (movementPointsAngle < 0.0f && fabs(movementPointsAngle) > 180.0f)) {

			if (lastValidAngle < 155.0f && angleDegrees >= 155.0f) {
				timesToGenerateParticles = TIME_PARTICLES;
			}

			lastValidAngle = angleDegrees;
			gameScore = gameScore + fabs(movementPointsAngle);

		}
	}

	lastAngle = angleDegrees;
}

void MyGame::renderSquareScreen(VSShaderLib& shaderText, float window_width, float window_height) {

	float width, height, xpos, ypos;

	width = 1 * window_width;
	height = 1 * window_height;

	xpos = (window_width - width) / 2;
	ypos = (window_height - height) / 2;

	renderQuad(shaderText, xpos, ypos, width, height, 0.58f, 0.65f, 0.65f, 0.4);

	width = 0.5 * window_width;
	height = 0.5 * window_height;

	xpos = (window_width - width) / 2;
	ypos = (window_height - height) / 2;

	renderQuad(shaderText, xpos, ypos, width, height, 0.18f, 0.19f, 0.19f, 1);

	std::string title, subtitle;
	if (state == MyGameState::Paused) title = "PAUSED";
	else if (state == MyGameState::Lost) title = "! YOU LOST !";

	if (state == MyGameState::Paused) subtitle = "Press 'S' to Unpause and 'R' to Restart";
	else if (state == MyGameState::Lost) subtitle = "Press 'R' to Restart";

	float window_size = window_width * window_height;
	float text_scale = 0.7f * (float(window_size) / float(WINDOW_SIZE_FOR_TEXT));
	if (text_scale < MIN_TEXT_SCALE) text_scale = MIN_TEXT_SCALE;

	RenderText(shaderText, title, true, 0.5 * window_width, 0.6 * window_height, text_scale * 2.0f, 1.0f, 1.0f, 1.0f);
	RenderText(shaderText, subtitle, true, 0.5 * window_width, 0.45 * window_height, text_scale, 1.0f, 1.0f, 1.0f);
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
	RenderText(shaderText, get_number_lifes_line(START_LIFES, lifes), false, 0.02 * window_width, 0.95 * window_height, text_scale, 0.8f, 0.8f, 0.8f);
	std::string score_line = "$coin$ " + std::to_string((int) gameScore);
	RenderText(shaderText, score_line, false,0.02 * window_width, 0.95 * window_height - 65.0f * text_scale, text_scale, 0.8f, 0.8f, 0.8f);

	if (state != MyGameState::Running) renderSquareScreen(shaderText, window_width, window_height);

	popMatrix(PROJECTION);
	popMatrix(VIEW);
	popMatrix(MODEL);

	glEnable(GL_DEPTH_TEST);
}
