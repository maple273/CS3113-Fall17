/*
CS-UY 3113
Geena Saji & Mary Qiu

Final Game Project: Bird Fight

Due December 13, 2017

Rules:
2 Player Game
Objective of Game: Player must catch a worm & successfully navigate to the birdhouse without getting hit by the flying birds guarding their worms.
-Player cannot enter the birdhouse without a worm. Player needs a worm to move on to the next level.
-If a player is hit, the opponent automatically gets 1 point and wins that level.

Level 1: Players can shoot the flying birds.
Level 2: Players can shoot the flying birds.
Level 3: Flying birds are immortal so players cannot shoot them down.

Game ends after Level 3. Player with the most points wins.

KEYS:
Player 1 controllers: Up, Down, Left, Right, Space to shoot
Player 2 controllers:  W,    S,    A,     D,     X to shoot
To exit the game: ESC 
*/
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>
#include <windows.h>
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;
using namespace std;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL, STATE_LEVEL_TWO, STATE_LEVEL_THREE, STATE_GAME_OVER };
Mix_Chunk* bulletShoot;
Mix_Chunk* shotEnemy;
Mix_Chunk* wormSound;
Mix_Chunk* doorSound;
bool playerLostL1 = false;
bool playerLostL2 = false;
bool playerLostL3 = false;
bool playerWonL1 = false;
bool playerWonL2 = false;
bool playerWonL3 = false;
bool player1WormL1_1 = false;
bool player2WormL1_2 = false;
bool player1WormL2_1 = false;
bool player2WormL2_2 = false;
bool player1WormL3_1 = false;
bool player2WormL3_2 = false;
float screenShakeValue = 0.0f;
float screenShakeSpeed = 0.05f;
float screenShakeIntensity = 0.05f;
class Vector3 {
public:
	Vector3() {};
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
	float x;
	float y;
	float z;
};

class SheetSprite {
public:
	SheetSprite() {};
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
	void Draw(ShaderProgram *program);
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};


class Entity {
public:
	Entity() {};
	Entity(SheetSprite sprite1, float x, float y, float z, float vel_x, float vel_y, float vel_z) : sprite(sprite1), position(x, y, z), velocity(vel_x, vel_y, vel_z) {}
	void Draw(ShaderProgram *program) {
		sprite.Draw(program);
	};
	Vector3 position;
	Vector3 velocity;
	Vector3 size;
	SheetSprite sprite;
	float score = 0.0f;
	float health = 0.0f; //0.0f=healthy, 1.0f=shot dead
};

void SheetSprite::Draw(ShaderProgram *program) {
	glBindTexture(GL_TEXTURE_2D, textureID);
	GLfloat texCoords[] = {
		u, v + height,
		u + width, v,
		u, v,
		u + width, v,
		u, v + height,
		u + width, v + height
	};
	float aspect = width / height;
	float vertices[] = {
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, 0.5f * size,
		0.5f * size * aspect, 0.5f * size,
		-0.5f * size * aspect, -0.5f * size,
		0.5f * size * aspect, -0.5f * size };

	//draw our arrays
	glBindTexture(GL_TEXTURE_2D, textureID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

GLuint LoadTexture(const char *filePath) {
	int w, h, comp;
	unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
	if (image == NULL) {
		cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}
	GLuint retTexture;
	glGenTextures(1, &retTexture);
	glBindTexture(GL_TEXTURE_2D, retTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	stbi_image_free(image);
	return retTexture;
}

vector<Entity*> createEnemies() {
	vector<Entity*> enemies;
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"player.png");
	SheetSprite enemySprite = SheetSprite(spriteSheetTexture, 87.0f / 256.0f, 125.0f / 512.0f, 31.0f / 256.0f, 39.0f / 512.0f, 0.5f);
	float xPosition = -3.0f;
	float yPosition = 3.0f;
	while (xPosition != -4.0f && yPosition != 0.0f) { //Push 18 enemy birds
		enemies.push_back(new Entity(enemySprite, xPosition, yPosition, 0.0, 0.5f, -0.3f, 0.0f));
		xPosition += 1.0f;
		if (xPosition == 3.0f) {
			xPosition = -3.0f;
			yPosition -= 1.0f;
		}
	}
	return enemies;
}
vector<Entity*> createEnemies2() {
	vector<Entity*> enemies2;
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"player.png");
	SheetSprite enemySprite = SheetSprite(spriteSheetTexture, 87.0f / 256.0f, 84.0f / 512.0f, 31.0f / 256.0f, 39.0f / 512.0f, 0.5f);
	float xPosition = -2.0f;
	float yPosition = 2.0f;
	while (xPosition != -3.0f && yPosition != 0.0f) { //Push 8 enemy birds
		enemies2.push_back(new Entity(enemySprite, xPosition, yPosition, 0.0, 0.5f, -0.5f, 0.0f));
		xPosition += 1.0f;
		if (xPosition == 2.0f) {
			xPosition = -2.0f;
			yPosition -= 1.0f;
		}
	}
	return enemies2;
}
vector<Entity*> createEnemies3() {
	vector<Entity*> enemies3;
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"player.png");
	SheetSprite enemySprite = SheetSprite(spriteSheetTexture, 120.0f / 256.0f, 125.0f / 512.0f, 31.0f / 256.0f, 39.0f / 512.0f, 1.0f);
	float xPosition = -3.0f;
	float yPosition = 3.0f;
	while (xPosition != -4.0f && yPosition != 0.0f) { //Push 18 enemy birds
		enemies3.push_back(new Entity(enemySprite, xPosition, yPosition, 0.0, 0.5f, -0.5f, 0.0f));
		xPosition += 1.0f;
		if (xPosition == 3.0f) {
			xPosition = -3.0f;
			yPosition -= 1.0f;
		}
	}
	return enemies3;
}
Entity createBullets(Entity *player) {
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
	SheetSprite bulletSprite = SheetSprite(spriteSheetTexture, 843.0f / 1024.0f, 977.0f / 1024.0f, 13.0f / 1024.0f, 37.0f / 1024.0f, 0.5f);
	Entity bullet1 = Entity(bulletSprite, player->position.x, player->position.y + 1.0f, 0.0f, 0.0f, 0.5f, 0.0f);
	Mix_PlayChannel(-1, bulletShoot, 0); //bullet shot sound 
	return bullet1;
}
vector<Entity*> createDoors() {
	vector<Entity*> doorV;
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"doors.png");
	SheetSprite doorSprite = SheetSprite(spriteSheetTexture, 72.0f / 1024.0f, 484.0f / 1024.0f, 61.0f / 1024.0f, 60.0f / 1024.0f, 1.0f);
	float xPosition = 0.0f;
	float yPosition = 4.5f;
	doorV.push_back(new Entity(doorSprite, xPosition, yPosition, 0.0, 0.0f, 0.0f, 0.0f));
	return doorV;
}
vector<Entity*> createWorms1() {
	vector<Entity*> wormsVec;
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"worms.png");
	SheetSprite wormSprite = SheetSprite(spriteSheetTexture, 0.0f / 128.0f, 0.0f / 128.0f, 27.0f / 128.0f, 23.0f / 128.0f, 0.5f);
	wormsVec.push_back(new Entity(wormSprite, -3.0f, 2.0f, 0.0, 0.0f, 0.0f, 0.0f)); //worm at (-3,2)
	wormsVec.push_back(new Entity(wormSprite, 4.0f, 3.0f, 0.0, 0.0f, 0.0f, 0.0f)); //worm2 at (4,3)
	return wormsVec;
}
vector<Entity*> createWorms2() {
	vector<Entity*> wormsVec;
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"worms.png");
	SheetSprite wormSprite = SheetSprite(spriteSheetTexture, 0.0f / 128.0f, 0.0f / 128.0f, 27.0f / 128.0f, 23.0f / 128.0f, 0.5f);
	wormsVec.push_back(new Entity(wormSprite, -2.0f, 3.0f, 0.0, 0.0f, 0.0f, 0.0f)); //worm at (-2,3)
	wormsVec.push_back(new Entity(wormSprite, 2.0f, 3.0f, 0.0, 0.0f, 0.0f, 0.0f)); //worm2 at (2,3)
	return wormsVec;
}
vector<Entity*> createWorms3() {
	vector<Entity*> wormsVec;
	GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"worms.png");
	SheetSprite wormSprite = SheetSprite(spriteSheetTexture, 0.0f / 128.0f, 0.0f / 128.0f, 27.0f / 128.0f, 23.0f / 128.0f, 0.5f);
	wormsVec.push_back(new Entity(wormSprite, 5.0f, 3.0f, 0.0, 0.0f, 0.0f, 0.0f)); //worm at (5,3)
	wormsVec.push_back(new Entity(wormSprite, 5.0f, -3.0f, 0.0, 0.0f, 0.0f, 0.0f)); //worm at (5,3)
	return wormsVec;
}
class GameState {
public:
	GameState() {}
	GameState(Entity *ptr, Entity *ptr2) {
		player = ptr;
		player2 = ptr2;
		enemies = createEnemies();
		doors = createDoors();
	}
	Entity *player;
	Entity *player2;
	vector<Entity*> enemies;
	vector<Entity*> bullets;
	vector<Entity*> doors;
	vector<Entity*> doors2 = createDoors();
	vector<Entity*> doors3 = createDoors();
	vector<Entity*> enemies2 = createEnemies2();
	vector<Entity*> enemies3 = createEnemies3();
	vector<Entity*> worms1 = createWorms1();
	vector<Entity*> worms2 = createWorms2();
	vector<Entity*> worms3 = createWorms3();
	void makeBulletVector(Entity *bullet1) {
		bullets.push_back(bullet1);
	}
};

void DrawText(ShaderProgram *program, int fontTexture, string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	for (int i = 0; i < text.size(); i++) {
		int spriteIndex = (int)text[i];
		float texture_x = (float)(spriteIndex % 16) / 16.0f;
		float texture_y = (float)(spriteIndex / 16) / 16.0f;

		vertexData.insert(vertexData.end(), {
			((size + spacing) * i) + (-0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (0.5f * size), -0.5f * size,
			((size + spacing) * i) + (0.5f * size), 0.5f * size,
			((size + spacing) * i) + (-0.5f * size), -0.5f * size,
		});
		texCoordData.insert(texCoordData.end(), {
			texture_x, texture_y,
			texture_x, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x + texture_size, texture_y + texture_size,
			texture_x + texture_size, texture_y,
			texture_x, texture_y + texture_size,
		});
	}
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}


void transform(float posX, float posY, ShaderProgram* program, Matrix& modelViewMatrix, Matrix& projectionMatrix) {
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(posX, posY, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
}

void drawGameScreen(GLuint background, float vertices[], float texCoords[], ShaderProgram* program) {
	glBindTexture(GL_TEXTURE_2D, background);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void drawBgSprite(ShaderProgram *program, Matrix &projectionMatrix, Matrix &modelViewMatrix, SheetSprite sprite, float posX, float posY) {
	Entity sprite1 = Entity(sprite, posX, posY, 0.0, 0.0f, 0.0f, 0.0f);
	transform(sprite1.position.x, sprite1.position.y, program, modelViewMatrix, projectionMatrix);
	sprite1.Draw(program);
}

GameMode updateGame(float elapsed, GameState state, SDL_Event* event, ShaderProgram* program, Matrix& modelViewMatrix, Matrix& projectionMatrix, bool done, GLuint fontText, float texCoords[], GLuint background) {

	while (!done) {
		while (SDL_PollEvent(event)) {
			if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glUseProgram(program->programID);
		glClearColor(0.0f, 0.3f, 0.0f, 0.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		float vertices0[] = { -8.0f, -6.0f, 8.0f, -6.0f, 8.0f, 7.0f, -8.0f, -6.0f, 8.0f, 7.0f, -8.0f, 7.0f };
		drawGameScreen(background, vertices0, texCoords, program);
				
		transform(-4.3f, 2.5f, program, modelViewMatrix, projectionMatrix);
		DrawText(program, fontText, "BIRD FIGHT", 1.0f, 0.0f);
		transform(-2.5f, -2.5f, program, modelViewMatrix, projectionMatrix);
		DrawText(program, fontText, "Press Enter", 0.5f, 0.0f);

		screenShakeValue += 0.2f;

		//Shakes screen for main menu
		//draws player1 on main menu screen
		modelViewMatrix.Identity();
		modelViewMatrix.Translate(state.player->position.x, state.player->position.y + 1.5f, state.player->position.z);
		modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		state.player->Draw(program);
		//draws player2 on main menu screen
		modelViewMatrix.Identity();
		modelViewMatrix.Translate(state.player2->position.x, state.player2->position.y + 1.5f, state.player2->position.z);
		modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		state.player2->Draw(program);

		const Uint8 *keys = SDL_GetKeyboardState(NULL);
		if (keys[SDL_SCANCODE_RETURN]) {
			return STATE_GAME_LEVEL;
		}
		if (keys[SDL_SCANCODE_ESCAPE]) {
			exit(0);
		}

		SDL_GL_SwapWindow(displayWindow);
	}
	return STATE_MAIN_MENU;
}
GameMode endGame(GameState &state) {

	if (playerLostL3 || playerWonL3) {
		return STATE_GAME_OVER; // go to end card
	}
	return STATE_LEVEL_THREE; //else stay at current level3

}
GameMode goToLevel2(GameState &state) {

	if (playerLostL1 || playerWonL1) {
		return STATE_LEVEL_TWO; // go to Level2
	}
	return STATE_GAME_LEVEL; //else stay at current Level 1
}
GameMode goToLevel3(GameState &state) {

	if (playerLostL2 || playerWonL2) {
		return STATE_LEVEL_THREE; // go to Level3
	}
	return STATE_LEVEL_TWO; //else stay at current level2

}

void displayScores(int score1, int score2, ShaderProgram* program, Matrix& modelViewMatrix, Matrix& projectionMatrix, GLuint fontText) {
	glUseProgram(program->programID);
	string scoreValue1 = to_string(score1);
	string scoreValue2 = to_string(score2);

	transform(-6.5f, 5.5f, program, modelViewMatrix, projectionMatrix);
	DrawText(program, fontText, "Blue Score: " + scoreValue1, 0.5f, -0.3f);
	transform(4.5f, 5.5f, program, modelViewMatrix, projectionMatrix);
	DrawText(program, fontText, "Red Score: " + scoreValue2, 0.5f, -0.3f);
}

//draw sprites for each of the levels
void Render(ShaderProgram *program, const GameState &state, Matrix &projectionMatrix, Matrix &modelViewMatrix, GLuint fontText, GLuint bgSpriteSheet) {
	glUseProgram(program->programID);
	displayScores((int)state.player->score, (int)state.player2->score, program, modelViewMatrix, projectionMatrix, fontText);

	SheetSprite cloudSprite = SheetSprite(bgSpriteSheet, 0.0f / 1024.0f, 0.0f / 512.0f, 515.0f / 1024.0f, 384.0f / 512.0f, 2.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, cloudSprite, -5.0f, 4.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, cloudSprite, 5.5f, 3.5f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, cloudSprite, 2.0f, -4.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, cloudSprite, -1.f, 1.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, cloudSprite, -6.0f, -1.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, cloudSprite, 4.0f, -1.5f);

	//positions player
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(state.player->position.x, state.player->position.y, state.player->position.z);

	//draws and shakes character
	screenShakeValue += 0.5f;
	modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	state.player->Draw(program);

	//draw player2
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(state.player2->position.x, state.player2->position.y, state.player2->position.z);
	//draws and shakes character
	modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	state.player2->Draw(program);

	//draw enemies
	for (size_t i = 0; i < state.enemies.size(); i++) {
		transform(state.enemies[i]->position.x, state.enemies[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.enemies[i]->Draw(program);
	}

	//draw bullets
	for (size_t i = 0; i < state.bullets.size(); i++) {
		transform(state.bullets[i]->position.x, state.bullets[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.bullets[i]->Draw(program);
	}

	//draw door 
	for (size_t i = 0; i < state.doors.size(); i++) {
		transform(state.doors[i]->position.x, state.doors[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.doors[i]->Draw(program);
	}
	//draw worms
	for (size_t i = 0; i < state.worms1.size(); i++) {
		transform(state.worms1[i]->position.x, state.worms1[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.worms1[i]->Draw(program);
	}
}
void Render2(ShaderProgram *program, const GameState &state, Matrix &projectionMatrix, Matrix &modelViewMatrix, GLuint fontText, GLuint bgSpriteSheet) {

	glUseProgram(program->programID);
	displayScores((int)state.player->score, (int)state.player2->score, program, modelViewMatrix, projectionMatrix, fontText);

	SheetSprite sunSprite = SheetSprite(bgSpriteSheet, 517.0f / 1024.0f, 0.0f / 512.0f, 300.0f / 1024.0f, 300.0f / 512.0f, 3.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, sunSprite, -5.0f, 4.0f);

	screenShakeValue += 0.5f;
	//positions player
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(state.player->position.x, state.player->position.y, state.player->position.z);
	//draws and shakes character
	modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	state.player->Draw(program);

	//draw player2
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(state.player2->position.x, state.player2->position.y, state.player2->position.z);
	//draws and shakes character
	modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	state.player2->Draw(program);

	//draw enemies
	for (size_t i = 0; i < state.enemies2.size(); i++) { 
		transform(state.enemies2[i]->position.x, state.enemies2[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.enemies2[i]->Draw(program);
	}
	//draw bullets
	for (size_t i = 0; i < state.bullets.size(); i++) {
		transform(state.bullets[i]->position.x, state.bullets[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.bullets[i]->Draw(program);
	}
	//draw door 
	for (size_t i = 0; i < state.doors2.size(); i++) {
		transform(state.doors2[i]->position.x, state.doors2[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.doors2[i]->Draw(program);
	}
	//draw worms
	for (size_t i = 0; i < state.worms2.size(); i++) {
		transform(state.worms2[i]->position.x, state.worms2[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.worms2[i]->Draw(program);
	}

}
void Render3(ShaderProgram *program, const GameState &state, Matrix &projectionMatrix, Matrix &modelViewMatrix, GLuint fontText, GLuint bgSpriteSheet) {

	glUseProgram(program->programID);
	displayScores((int)state.player->score, (int)state.player2->score, program, modelViewMatrix, projectionMatrix, fontText);

	SheetSprite starSprite = SheetSprite(bgSpriteSheet, 0.0f / 1024.0f, 386.0f / 512.0f, 31.0f / 1024.0f, 30.0f / 512.0f, 1.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, starSprite, -5.0f, 4.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, starSprite, 5.5f, 3.5f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, starSprite, 2.0f, -4.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, starSprite, -1.f, 1.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, starSprite, -6.0f, -1.0f);
	drawBgSprite(program, projectionMatrix, modelViewMatrix, starSprite, 4.0f, -1.5f);
	
	screenShakeValue += 0.5f;
	//positions player
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(state.player->position.x, state.player->position.y, state.player->position.z);
	//draws and shakes character
	
	modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	state.player->Draw(program);

	//draw player2
	modelViewMatrix.Identity();
	modelViewMatrix.Translate(state.player2->position.x, state.player2->position.y, state.player2->position.z);
	//draws and shakes character
	modelViewMatrix.Translate(0.0f, sin(screenShakeValue * screenShakeSpeed)* screenShakeIntensity, 0.0f);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelViewMatrix);
	state.player2->Draw(program);


	//draw immortal enemies
	for (size_t i = 0; i < state.enemies3.size(); i++) {
		transform(state.enemies3[i]->position.x, state.enemies3[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.enemies3[i]->Draw(program);
	}

	//draw bullets
	for (size_t i = 0; i < state.bullets.size(); i++) {
		transform(state.bullets[i]->position.x, state.bullets[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.bullets[i]->Draw(program);
	}
	//draw door 
	for (size_t i = 0; i < state.doors3.size(); i++) {
		transform(state.doors3[i]->position.x, state.doors3[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.doors3[i]->Draw(program);
	}
	//draw worms
	for (size_t i = 0; i < state.worms3.size(); i++) {
		transform(state.worms3[i]->position.x, state.worms3[i]->position.y, program, modelViewMatrix, projectionMatrix);
		state.worms3[i]->Draw(program);
	}
}
void renderEndScreen(ShaderProgram* program, const GameState &state, Matrix& modelViewMatrix, Matrix& projectionMatrix, GLuint fontText, float texCoords[], GLuint background) {
	float vertices1[] = { -12.0f, -12.0f, 12.0f, -12.0f, 12.0f, 12.0f, -12.0f, -12.0f, 12.0f, 12.0f, -12.0f, 12.0f };
	drawGameScreen(background, vertices1, texCoords, program);

	displayScores((int)state.player->score, (int)state.player2->score, program, modelViewMatrix, projectionMatrix, fontText);

	transform(-4.3f, 1.8f, program, modelViewMatrix, projectionMatrix);
	if (state.player->score > state.player2->score) {
		DrawText(program, fontText, "Blue Bird WINS! ", 1.0f, -0.25f);
	}
	else {
		DrawText(program, fontText, "Red Bird WINS!", 1.0f, -0.25f);
	}
	transform(-2.0f, -2.0f, program, modelViewMatrix, projectionMatrix);
	DrawText(program, fontText, "Game Over", 0.5f, 0.0f);

	SDL_GL_SwapWindow(displayWindow);
}
void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Bird Fight", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif
	glViewport(0, 0, 800, 600);

}
//move array of enemies for level 1
void moveEnemies(GameState &state, float elapsed) {
	for (size_t i = 0; i < 12; i++) {//go to right
		state.enemies[i]->position.x += elapsed*state.enemies[i]->velocity.x;
	}
	for (size_t i = 0; i < 12; i++) {//when enemies reach either ends, switch directions
		if (state.enemies[i]->position.x >= 4.0f || state.enemies[i]->position.x <= -4.0f) {
			state.enemies[i]->velocity.x *= -1.0f;
		}
	}
	for (size_t i = 12; i < 18; i++) {//go up
		state.enemies[i]->position.y += elapsed*state.enemies[i]->velocity.y;
	}
	for (size_t i = 12; i < 18; i++) {//when enemies reach either ends, switch directions
		if (state.enemies[i]->position.y >= 4.0f || state.enemies[i]->position.y <= -4.0f) {
			state.enemies[i]->velocity.y *= -1.0f;
		}
	}

}
//check if 2 entities collided
bool checkCollision(Entity *ent1, Entity *ent2) {
	//if any are true, ent1 and ent2 are not intersecting
	if (
		((ent1->position.y + (ent1->sprite.size / 2)) <= (ent2->position.y - (ent2->sprite.size / 2))) || //ent1's top < ent2 bottom
		((ent1->position.y - (ent1->sprite.size / 2)) >= (ent2->position.y + (ent2->sprite.size / 2))) || //ent1's bottom > ent2 top
		((ent1->position.x + (ent1->sprite.size / 2)) <= (ent2->position.x - (ent2->sprite.size / 2))) || //ent1's right < ent2 left 
		((ent1->position.x - (ent1->sprite.size / 2)) >= (ent2->position.x + (ent2->sprite.size / 2)))    //ent1's left > ent2 right 
		) {
		return false;
	}
	else {
		Mix_PlayChannel(-1, shotEnemy, 0); //play collided sound 
		return true;
	}
}
//remove bullets & enemies that have collided with each other
void removeCollisions(GameState &state) {
	for (size_t i = 0; i < state.bullets.size(); i++) {	//for all the bullets in bullet vector
		for (size_t j = 0; j< state.enemies.size(); j++) {
			if (checkCollision(state.bullets[i], state.enemies[j])) {	//if bullet collides with enemy
				state.enemies[j]->health = 1.0f; 	//set enemy's health to 1.0f =dead
				state.enemies[j]->position.y = -100.0f; //move enemy from screen
				state.bullets[i]->position.x = -100.0f; //move bullet from screen
				return;
			}
		}
	}
}
void removeCollisions2(GameState &state) {
	for (size_t i = 0; i < state.bullets.size(); i++) {	//for all the bullets in bullet vector
		for (size_t j = 0; j< state.enemies2.size(); j++) {
			if (checkCollision(state.bullets[i], state.enemies2[j])) {	//if bullet collides with enemy
				state.enemies2[j]->health = 1.0f; 	//set enemy's health to 1.0f =dead
				state.enemies2[j]->position.y = -100.0f; //move enemy from screen
				state.bullets[i]->position.x = -100.0f; //move bullet from screen
				return;
			}
		}
	}
}
//check if a player reaches a worm in each of the levels
void checkPlayerReachWorm1(GameState &state) {

	//if  player1 collides with a worm, set worm1 to true
	for (size_t i = 0; i < state.worms1.size(); i++) {
		if (checkCollision(state.worms1[i], state.player)) {
			player1WormL1_1 = true;
			state.worms1[i]->position.y = -100.0f;
			Mix_PlayChannel(-1, wormSound, 0);
		}
	}

	//if  player2 collides with a worm, set worm1 to true
	for (size_t i = 0; i < state.worms1.size(); i++) {
		if (checkCollision(state.worms1[i], state.player2)) {
			player2WormL1_2 = true;
			state.worms1[i]->position.y = -100.0f;
			Mix_PlayChannel(-1, wormSound, 0);
		}
	}
}
void checkPlayerReachWorm2(GameState &state) {
	//if  player1 collides with a worm, set worm1 to true
	for (size_t i = 0; i < state.worms2.size(); i++) {
		if (checkCollision(state.worms2[i], state.player)) {
			player1WormL2_1 = true;
			state.worms2[i]->position.y = -100.0f;
			Mix_PlayChannel(-1, wormSound, 0);
		}
	}

	//if  player2 collides with a worm, set worm2 to true
	for (size_t i = 0; i < state.worms2.size(); i++) {
		if (checkCollision(state.worms2[i], state.player2)) {
			player2WormL2_2 = true;
			state.worms2[i]->position.y = -100.0f;
			Mix_PlayChannel(-1, wormSound, 0);
		}
	}
}
void checkPlayerReachWorm3(GameState &state) {
	//if  player1 collides with a worm, set worm1 to true
	for (size_t i = 0; i < state.worms3.size(); i++) {
		if (checkCollision(state.worms3[i], state.player)) {
			player1WormL3_1 = true;
			state.worms3[i]->position.y = -100.0f;
			Mix_PlayChannel(-1, wormSound, 0);
		}
	}

	//if  player2 collides with a worm, set worm2 to true
	for (size_t i = 0; i < state.worms3.size(); i++) {
		if (checkCollision(state.worms3[i], state.player2)) {
			player2WormL3_2 = true;
			state.worms3[i]->position.y = -100.0f;
			Mix_PlayChannel(-1, wormSound, 0);
		}
	}
}
//for each level check if player reached birdhouse door
bool checkPlayerReachDoor(GameState &state) {
	bool aPlayerWon = false;
	//if  player1 has a worm, then check if P1 reached door, player 1 scores 1PT
	if (player1WormL1_1) {
		for (size_t i = 0; i < state.doors.size(); i++) {	//for all the doors in door vector		
			if (checkCollision(state.doors[i], state.player)) {	//if door collides with player1
				state.player->score += 1.0f; 	//set player1's score to +=1.0f
				state.player->position.y = -100.0f; //move player1 from screen
				state.player2->position.y = -100.0f; //move player1 from screen
				Mix_PlayChannel(-1, doorSound, 0);
				aPlayerWon = true;
				return aPlayerWon;
			}
		}
	}

	//if  player2 has a worm, then check if P2 reached door, player 2 scores 1PT
	if (player2WormL1_2) {
		for (size_t i = 0; i < state.doors.size(); i++) {	//for all the doors in door vector		
			if (checkCollision(state.doors[i], state.player2)) {	//if door collides with player1
				state.player2->score += 1.0f; 	//set player2's score to +=1.0f
				state.player->position.y = -100.0f; //move player1 from screen
				state.player2->position.y = -100.0f; //move player1 from screen
				Mix_PlayChannel(-1, doorSound, 0);
				aPlayerWon = true;
				return aPlayerWon;
			}
		}
	}

	return aPlayerWon;
}
bool checkPlayerReachDoor2(GameState &state) {
	bool aPlayerWon = false;
	//if  player1 has a worm, then check if P1 reached door, player 1 scores 1PT
	if (player1WormL2_1) {
		for (size_t i = 0; i < state.doors2.size(); i++) {	//for all the doors in door vector		
			if (checkCollision(state.doors2[i], state.player)) {	//if door collides with player1
				state.player->score += 1.0f; 	//set player1's score to +=1.0f
				state.player->position.y = -100.0f; //move player1 from screen
				state.player2->position.y = -100.0f; //move player1 from screen
				Mix_PlayChannel(-1, doorSound, 0);
				aPlayerWon = true;
				return aPlayerWon;
			}
		}
	}

	//if  player2 has a worm, then check if P2 reached door, player 2 scores 1PT
	if (player2WormL2_2) {
		for (size_t i = 0; i < state.doors2.size(); i++) {	//for all the doors in door vector		
			if (checkCollision(state.doors2[i], state.player2)) {	//if door collides with player1
				state.player2->score += 1.0f; 	//set player2's score to +=1.0f
				state.player->position.y = -100.0f; //move player1 from screen
				state.player2->position.y = -100.0f; //move player2 from screen
				Mix_PlayChannel(-1, doorSound, 0);
				aPlayerWon = true;
				return aPlayerWon;
			}
		}
	}

	return aPlayerWon;
}
bool checkPlayerReachDoor3(GameState &state) {
	bool aPlayerWon = false;
	if (player1WormL3_1) {
		//if  player1 has a worm, then check if P1 reached door, player 1 scores 1PT
		for (size_t i = 0; i < state.doors3.size(); i++) {	//for all the doors in door vector		
			if (checkCollision(state.doors3[i], state.player)) {	//if door collides with player1
				state.player->score += 1.0f; 	//set player1's score to +=1.0f
				state.player->position.y = -100.0f; //move player1 from screen
				state.player2->position.y = -100.0f; //move player1 from screen
				Mix_PlayChannel(-1, doorSound, 0);
				aPlayerWon = true;
				return aPlayerWon;
			}
		}

	}
	//if  player2 has a worm, then check if P2 reached door, player 2 scores 1PT
	if (player2WormL3_2) {
		for (size_t i = 0; i < state.doors3.size(); i++) {	//for all the doors in door vector		
			if (checkCollision(state.doors3[i], state.player2)) {	//if door collides with player1
				state.player2->score += 1.0f; 	//set player2's score to +=1.0f
				state.player->position.y = -100.0f; //move player1 from screen
				state.player2->position.y = -100.0f; //move player1 from screen
				Mix_PlayChannel(-1, doorSound, 0);
				aPlayerWon = true;
				return aPlayerWon;
			}
		}
	}
	return aPlayerWon;
}
//for each level update score if player collides with enemy or successfully reaches door
void updateScore(GameState &state) {

	checkPlayerReachWorm1(state);
	playerWonL1 = checkPlayerReachDoor(state);

	//if  player1 collides with an enemy, player 1 explode sound & remove, set player2 score +=1
	for (size_t i = 0; i < state.enemies.size(); i++) {	//for all the enemies in enemy vector		
		if (checkCollision(state.enemies[i], state.player)) {	//if enemy collides with player1
			playerLostL1 = true; 	//player1 LOST level	
			state.player->health = 1.0f; 	//set player1's health to 1.0f =dead
			state.player2->score += 1.0f; 	//set player2's score to +=1.0f
			state.player->position.y = -100.0f; //move player1 from screen
			Mix_PlayChannel(-1, shotEnemy, 0); //play collided sound 
			return;
		}
	}
	//if  player2 collides with an enemy, player 2 explode sound & remove, set player1 score +=1
	for (size_t i = 0; i < state.enemies.size(); i++) {	//for all the enemies in enemy vector		
		if (checkCollision(state.enemies[i], state.player2)) {	//if enemy collides with player2
			playerLostL1 = true; 	//player2 LOST level
			state.player2->health = 1.0f; 	//set player2's health to 1.0f =dead
			state.player->score += 1.0f; 	//set player1's score to +=1.0f
			state.player2->position.y = -100.0f; //move player2 from screen
			Mix_PlayChannel(-1, shotEnemy, 0); //play collided sound 
			return;
		}
	}
}
void updateScore2(GameState &state) {
	checkPlayerReachWorm2(state);
	playerWonL2 = checkPlayerReachDoor2(state);

	for (size_t i = 0; i < state.enemies2.size(); i++) {	//for all the enemies in enemy vector		
		if (checkCollision(state.enemies2[i], state.player)) {	//if enemy collides with player1
			playerLostL2 = true; 	//player1 LOST level
			state.player2->score += 1.0f; 	//set player2's score to +=1.0f
			state.player->position.y = -100.0f; //move player1 from screen
			Mix_PlayChannel(-1, shotEnemy, 0); //play collided sound 
			return;
		}
	}

	for (size_t i = 0; i < state.enemies2.size(); i++) {	//for all the enemies in enemy vector		
		if (checkCollision(state.enemies2[i], state.player2)) {	//if enemy collides with player1
			playerLostL2 = true; 	//player1 LOST level
			state.player->score += 1.0f; 	//set player1's score to +=1.0f
			state.player2->position.y = -100.0f; //move player2 from screen
			Mix_PlayChannel(-1, shotEnemy, 0); //play collided sound 
			return;
		}
	}
}
void updateScore3(GameState &state) {
	checkPlayerReachWorm3(state);
	playerWonL3 = checkPlayerReachDoor3(state);

	for (size_t i = 0; i < state.enemies3.size(); i++) {	//for all the enemies in enemy vector		
		if (checkCollision(state.enemies3[i], state.player)) {	//if enemy collides with player1
			playerLostL3 = true; 	//set player1's health to 1.0f =dead
			state.player2->score += 1.0f; 	//set player2's score to +=1.0f
			state.player->position.y = -100.0f; //move player1 from screen
			Mix_PlayChannel(-1, shotEnemy, 0); //play collided sound 
			return;
		}
	}

	for (size_t i = 0; i < state.enemies3.size(); i++) {	//for all the enemies in enemy vector		
		if (checkCollision(state.enemies3[i], state.player2)) {	//if enemy collides with player1
			playerLostL3 = true; 	//set player2's health to 1.0f =dead
			state.player->score += 1.0f; 	//set player1's score to +=1.0f
			state.player2->position.y = -100.0f; //move player2 from screen
			Mix_PlayChannel(-1, shotEnemy, 0); //play collided sound 
			return;
		}
	}
}
//for each level move the enemies in designated way
void moveEnemies2(GameState &state, float elapsed) {
	//move enemies up down
	for (size_t i = 0; i < 4; i++) {
		state.enemies2[i]->position.y += elapsed*state.enemies2[i]->velocity.y;
	}
	for (size_t i = 0; i < 4; i++) {//when enemies reach either ends, switch directions
		if (state.enemies2[i]->position.y >= 3.0f || state.enemies2[i]->position.y <= -3.0f) {
			state.enemies2[i]->velocity.y *= -1.0f;
		}
	}
	for (size_t i = 4; i < 8; i++) {
		state.enemies2[i]->velocity.x += 0.001f;
		state.enemies2[i]->velocity.y -= 0.000007f;
		state.enemies2[i]->position.x += elapsed*state.enemies2[i]->velocity.x;
		state.enemies2[i]->position.y += elapsed*state.enemies2[i]->velocity.y;
	}
	for (size_t i = 4; i < 8; i++) {//when enemies reach either ends, switch directions
		if (state.enemies2[i]->position.x >= 6.0f || state.enemies2[i]->position.x <= -6.0f) {
			state.enemies2[i]->velocity.x *= -1.0f;
		}
		if (state.enemies2[i]->position.y >= 5.0f || state.enemies2[i]->position.y <= -5.0f) {
			state.enemies2[i]->velocity.y *= -1.0f;
		}
	}
}
void moveEnemies3(GameState &state, float elapsed) {
	for (size_t i = 0; i < 6; i++) {
		state.enemies3[i]->velocity.x += 0.001f;
		state.enemies3[i]->velocity.y -= 0.000009f;
		state.enemies3[i]->position.x += elapsed*state.enemies3[i]->velocity.x;
		state.enemies3[i]->position.y += elapsed*state.enemies3[i]->velocity.y;
	}
	for (size_t i = 0; i < 6; i++) {//when enemies reach either ends, switch directions
		if (state.enemies3[i]->position.x >= 6.0f || state.enemies3[i]->position.x <= -6.0f) {
			state.enemies3[i]->velocity.x *= -1.0f;
		}
		if (state.enemies3[i]->position.y >= 5.0f || state.enemies3[i]->position.y <= -5.0f) {
			state.enemies3[i]->velocity.y *= -1.0f;
		}
	}

	for (size_t i = 6; i < 12; i++) {//go to right
		state.enemies3[i]->position.x += elapsed*state.enemies3[i]->velocity.x;
	}
	for (size_t i = 6; i < 12; i++) {//when enemies reach either ends, switch directions
		if (state.enemies3[i]->position.x >= 4.0f || state.enemies3[i]->position.x <= -4.0f) {
			state.enemies3[i]->velocity.x *= -1.0f;
		}
	}
	for (size_t i = 12; i < 18; i++) {//go up
		state.enemies3[i]->position.y += elapsed*state.enemies3[i]->velocity.y;
	}
	for (size_t i = 12; i < 18; i++) {//when enemies reach either ends, switch directions
		if (state.enemies3[i]->position.y >= 4.0f || state.enemies3[i]->position.y <= -4.0f) {
			state.enemies3[i]->velocity.y *= -1.0f;
		}
	}
}


//for each level update the game states with scores and necessary entity movements
void updateGameState(GameState &state, float elapsed) {
	moveEnemies(state, elapsed);
	//move bullets
	for (size_t i = 0; i < state.bullets.size(); i++) {
		if (state.bullets[i] == nullptr) {
			continue;
		}
		if (state.bullets[i]->position.y>4.0f) {
			state.bullets.erase(state.bullets.begin() + i);
			i -= 1;
		}
		else {
			state.bullets[i]->position.y += elapsed*state.bullets[i]->velocity.y;
		}
	}
	removeCollisions(state);
	updateScore(state);
}
void updateGameState2(GameState &state, float elapsed) {
	moveEnemies2(state, elapsed);
	//move bullets
	for (size_t i = 0; i < state.bullets.size(); i++) {
		if (state.bullets[i] == nullptr) {
			continue;
		}
		if (state.bullets[i]->position.y>4.0f) {
			state.bullets.erase(state.bullets.begin() + i);
			i -= 1;
		}
		else {
			state.bullets[i]->position.y += elapsed*state.bullets[i]->velocity.y;
		}
	}
	removeCollisions2(state);
	updateScore2(state);
}
void updateGameState3(GameState &state, float elapsed) {
	moveEnemies3(state, elapsed);
	//move bullets
	for (size_t i = 0; i < state.bullets.size(); i++) {
		if (state.bullets[i] == nullptr) {
			continue;
		}
		if (state.bullets[i]->position.y>4.0f) {
			state.bullets.erase(state.bullets.begin() + i);
			i -= 1;
		}
		else {
			state.bullets[i]->position.y += elapsed*state.bullets[i]->velocity.y;
		}
	}
	updateScore3(state);
}

int main(int argc, char *argv[]) {
	Setup();

	SDL_Event event;
	bool done = false;
	float lastFrameTicks = 0.0f;
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl"); //supports texture
	GLuint bgSpriteSheet = LoadTexture("bgSprite.png");
	GLuint background0 = LoadTexture("mario_clouds.png");
	GLuint background4 = LoadTexture("sky.png");
	GLuint spriteSheetTexture = LoadTexture("player.png");
	GLuint gameFont = LoadTexture("font1.png");
	SheetSprite playerSprite = SheetSprite(spriteSheetTexture, 0.0f / 256.0f, 100.0f / 512.0f, 47.0f / 256.0f, 24.0f / 512.0f, 0.3f); //blue bird
	SheetSprite player2Sprite = SheetSprite(spriteSheetTexture, 0.0f / 256.0f, 48.0f / 512.0f, 47.0f / 256.0f, 24.0f / 512.0f, 0.3f); //red bird

	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-7.0f, 7.0f, -6.0f, 6.0f, -1.0f, 1.0f);
	Entity playerEntity = Entity(playerSprite, 1.0f, -2.0f, 0.0f, 0.1f, 0.1f, 0.0f);
	Entity player2Entity = Entity(player2Sprite, -1.0f, -2.0f, 0.0f, 0.1f, 0.1f, 0.0f);

	GameState gState = GameState(&playerEntity, &player2Entity);
	GameMode mode = STATE_MAIN_MENU;

	//Loading music and sound effects
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	bulletShoot = Mix_LoadWAV("shooting.wav");
	shotEnemy = Mix_LoadWAV("explode.wav");
	doorSound = Mix_LoadWAV("entrance.wav");
	wormSound = Mix_LoadWAV("270339__littlerobotsoundfactory__pickup-02.wav");
	Mix_Chunk* winSound = Mix_LoadWAV("353546__maxmakessounds__success.wav");
	Mix_Music* gameMusic = Mix_LoadMUS("Visager_-_22_-_Battle_Loop.mp3");
	Mix_VolumeMusic(25);
	Mix_PlayMusic(gameMusic, -1);

	const Uint8 *keys = SDL_GetKeyboardState(NULL);

	while (!done) {
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;

		//move player left & right
		if (keys[SDL_SCANCODE_LEFT]) {
			if ((playerEntity.position.x - (playerEntity.sprite.width / 2)) > -6.5f) {
				playerEntity.position.x -= 2.0f *elapsed;
			}
		}
		if (keys[SDL_SCANCODE_RIGHT]) {
			if ((playerEntity.position.x + (playerEntity.sprite.width / 2)) < 6.5f) {
				playerEntity.position.x += 2.0f* elapsed;
			}
		}
		if (keys[SDL_SCANCODE_SPACE]) {
			//shoot bullet
			//note bullet moves faster when space bar held down bit longer & relesed
			Entity bullet1 = createBullets(gState.player);
			gState.makeBulletVector(&bullet1);
		}

		if (keys[SDL_SCANCODE_UP]) {
			if (playerEntity.position.y + (playerEntity.sprite.height / 2) < 5.0f) {
				playerEntity.position.y += elapsed;
			}

		}
		if (keys[SDL_SCANCODE_DOWN]) {
			if (playerEntity.position.y - (playerEntity.sprite.height / 2) > -5.0f) {
				playerEntity.position.y -= elapsed;
			}
		}

		//MOVE for PLAYER 2
		if (keys[SDL_SCANCODE_A]) {
			if ((player2Entity.position.x - (player2Entity.sprite.width / 2)) > -6.5f) {
				player2Entity.position.x -= 2.0f *elapsed;
			}
		}
		if (keys[SDL_SCANCODE_D]) {
			if ((player2Entity.position.x + (player2Entity.sprite.width / 2)) < 6.5f) {
				player2Entity.position.x += 2.0f* elapsed;
			}
		}
		if (keys[SDL_SCANCODE_X]) {
			//shoot bullet
			//note bullet moves faster when space bar held down bit longer & relesed
			Entity bullet1 = createBullets(gState.player2);
			gState.makeBulletVector(&bullet1);
		}

		if (keys[SDL_SCANCODE_W]) { //player 2 up
			if (player2Entity.position.y + (player2Entity.sprite.height / 2) < 5.0f) {
				player2Entity.position.y += elapsed;
			}

		}
		if (keys[SDL_SCANCODE_S]) { //player 2 down
			if (player2Entity.position.y - (player2Entity.sprite.height / 2) > -5.0f) {
				player2Entity.position.y -= elapsed;
			}
		}
		//press ESC to exit
		if (keys[SDL_SCANCODE_ESCAPE]) {
			exit(0);
		}

		switch (mode) {

		case STATE_MAIN_MENU:
			mode = updateGame(elapsed, gState, &event, &program, modelviewMatrix, projectionMatrix, done, gameFont, texCoords, background0);
			break;

		case STATE_GAME_LEVEL:

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
			}
			glClearColor(0.0f, 0.6f, 1.0f, 0.5f); //blue background - sky
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(program.programID);

			//press ESC to exit
			if (keys[SDL_SCANCODE_ESCAPE]) {
				exit(0);
			}

			Render(&program, gState, projectionMatrix, modelviewMatrix, gameFont, bgSpriteSheet);
			updateGameState(gState, elapsed);
			SDL_GL_SwapWindow(displayWindow);
			mode = goToLevel2(gState);
			break;

		case STATE_LEVEL_TWO:

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
			}
			glClearColor(1.0f, 0.7f, 0.0f, 0.5f); //orangish yellow background - sunset
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(program.programID);

			if (playerEntity.position.y < -7.0f || player2Entity.position.y < -7.0f) { //reset players position if outside bounds
				playerEntity.position.x = 2.0f;
				playerEntity.position.y = -2.0f;
				playerEntity.health = 0.0f;
				player2Entity.position.x = -2.0f;
				player2Entity.position.y = -2.0f;
				player2Entity.health = 0.0f;
			}

			Render2(&program, gState, projectionMatrix, modelviewMatrix, gameFont, bgSpriteSheet);
			updateGameState2(gState, elapsed);
			SDL_GL_SwapWindow(displayWindow);
			mode = goToLevel3(gState);

			break;

		case STATE_LEVEL_THREE:

			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
					done = true;
				}
			}
			glClearColor(0.128f, 0.128f, 0.128f, 0.5f); //dark grey background - night sky
			glClear(GL_COLOR_BUFFER_BIT);
			glUseProgram(program.programID);

			if (playerEntity.position.y < -7.0f || player2Entity.position.y < -7.0f) { //reset players position if outside bounds
				playerEntity.position.x = 2.0f;
				playerEntity.position.y = -2.0f;
				playerEntity.health = 0.0f;
				player2Entity.position.x = -2.0f;
				player2Entity.position.y = -2.0f;
				player2Entity.health = 0.0f;
			}

			Render3(&program, gState, projectionMatrix, modelviewMatrix, gameFont, bgSpriteSheet);
			updateGameState3(gState, elapsed);
			SDL_GL_SwapWindow(displayWindow);
			mode = endGame(gState);
			break;

		case STATE_GAME_OVER:
			Mix_HaltMusic();
			Mix_PlayChannel(-1, winSound, 0);
			renderEndScreen(&program, gState, modelviewMatrix, projectionMatrix, gameFont, texCoords, background4);
			Sleep(5000);
			exit(0);
		}

	}
	Mix_FreeChunk(shotEnemy);
	Mix_FreeChunk(bulletShoot);
	Mix_FreeChunk(winSound);
	Mix_FreeChunk(wormSound);
	Mix_FreeMusic(gameMusic);
	SDL_Quit();
	return 0;
}