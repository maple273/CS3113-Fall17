/*
Mary Qiu

Homework 5: Space Invaders

Player controls:
A - move left
D - move right
SPACE - to attack enemy's spaceships

*/


#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include <vector>
#define FIXED_TIMESTEP 0.0166666f
#include <SDL_mixer.h>


SDL_Window* displayWindow;
using namespace std;

enum GameMode { TITLE_SCREEN, GAME_LEVEL };




class SheetSprite {
public:
	SheetSprite() {};   // default constructor
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) : textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
	void Draw(ShaderProgram *program);
	float size;
	unsigned int textureID;
	float u;
	float v;
	float width;
	float height;
};

class Vector3 {
public:
	Vector3() {};
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
	float x;
	float y;
	float z;
};

class Entity {
public:
	Entity() {};
	Entity(SheetSprite spritey, float posx, float posy, float posz, float velx, float vely, float velz) : sprite(spritey), position(posx, posy, posz), velocity(velx, vely, velz) {}
	void Draw(ShaderProgram *program);
	Vector3 position;
	Vector3 velocity;
	SheetSprite sprite;
};

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

vector<Entity *>  fillEnemies() {
	vector<Entity *>  invaders;
	GLuint spriteSheet = LoadTexture("sheet.png");
	//Loading enemyRed3 ship
	SheetSprite invaderShip = SheetSprite(spriteSheet, 224.0f / 1024.0f, 580.0f / 1024.0f, 103.0f / 1024.0f, 84.0f / 1024.0f, .4f);
	float x_value = -1.0f;
	float y_value = 2.0f;
	while (y_value > -2.0) {
		invaders.push_back(new Entity(invaderShip, x_value, y_value, 0.0, 0.5f, -0.1f, 0.0f));
		x_value += 1.0f;
		if (x_value == 2.0) {
			x_value = -1.0;
			y_value -= 1.0f;
		}
	}
	return invaders;
}

class GameState {
public:
	GameState() {};
	GameState(Entity *object) : player(object) {
		enemiesVec = fillEnemies();
	};
	Entity *player;
	vector <Entity *> enemiesVec;
	vector <Entity *> bulletsVec;
};

void createPlayer(GLuint gameSheet, Matrix &projectionMatrix, Matrix &modelviewMatrix, ShaderProgram *program, GameState game) {
	glUseProgram(program->programID);
	modelviewMatrix.Identity();
	modelviewMatrix.Translate(game.player->position.x, game.player->position.y, game.player->position.z);
	program->SetProjectionMatrix(projectionMatrix);
	program->SetModelviewMatrix(modelviewMatrix);
	game.player->Draw(program);
}

void createEnemies(GameState state, ShaderProgram *program, Matrix &projectionMatrix, Matrix &modelviewMatrix) {
	for (size_t index = 0; index < state.enemiesVec.size(); index++) {
			if (state.enemiesVec[index] == NULL) {
				continue;
			}
			modelviewMatrix.Identity();
			modelviewMatrix.Translate(state.enemiesVec[index]->position.x, state.enemiesVec[index]->position.y, 0.0f);
			program->SetProjectionMatrix(projectionMatrix);
			program->SetModelviewMatrix(modelviewMatrix);
			state.enemiesVec[index]->Draw(program);	
		}

}

void createBullets(GLuint gameSheet, GameState state, ShaderProgram *program, Matrix &projectionMatrix, Matrix &modelviewMatrix) {
	for (size_t i = 0; i < state.bulletsVec.size(); i++) {
		modelviewMatrix.Identity();
		modelviewMatrix.Translate(state.bulletsVec[i]->position.x, state.bulletsVec[i]->position.y, 0.0f);
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelviewMatrix);
		state.bulletsVec[i]->Draw(program);
	}
}

void RenderGame(ShaderProgram *p,  Matrix &projectionMatrix, Matrix &modelviewMatrix, GLuint gameSheet, GameState game) {
	createPlayer(gameSheet, projectionMatrix, modelviewMatrix, p, game);
	createEnemies(game, p, projectionMatrix, modelviewMatrix);
	createBullets(gameSheet, game, p, projectionMatrix, modelviewMatrix);
}

void moveObj(GameState &game, float elapsed) {
		//moves the enemies position
		for (size_t e = 0; e < game.enemiesVec.size(); e++) {
			if (game.enemiesVec[e] == NULL) { 
				continue; 
			}
			game.enemiesVec[e]->position.x += elapsed * game.enemiesVec[e]->velocity.x;
		}
		//moves bullets upward
		for (size_t b = 0; b < game.bulletsVec.size(); b++) {
			if (game.bulletsVec[b] == NULL) { 
				continue;
			}
			game.bulletsVec[b]->position.y += elapsed * game.bulletsVec[b]->velocity.y;
		}
}

void checkCollision(GameState &state) {
	bool collisionStat = false;
	for (size_t e = 0; e < state.enemiesVec.size(); e++) {
		if (state.enemiesVec[e] == NULL) { 
			continue; 
		}
		float p = state.enemiesVec[e]->position.x + state.enemiesVec[e]->sprite.width;
		float p2 = state.enemiesVec[e]->position.x - state.enemiesVec[e]->sprite.width;
		if ((p/2) >= 2.9f || (p2 / 2) <= -2.9) {
			collisionStat = true;
			break;
		}
	}
	if (collisionStat) {
		for (size_t e = 0; e < state.enemiesVec.size(); e++) {
			if (state.enemiesVec[e] == NULL) { 
				continue;
			}
			state.enemiesVec[e]->velocity.x *= -1.0;
		}
		collisionStat = false;
	}
}

//updates the game everytime a player attacks and etc.
void update(GameState &game, float elapsed) {
	moveObj(game,elapsed);
	checkCollision(game);	
}



//Draw methods
void Entity::Draw(ShaderProgram *program) {
	sprite.Draw(program);
}
void SheetSprite::Draw(ShaderProgram *program) {
	GLfloat textureCoords[] = {
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
		-0.5f * size * aspect, -0.5f * size ,
		0.5f * size * aspect, -0.5f * size };

	//draw
	glBindTexture(GL_TEXTURE_2D, textureID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, textureCoords);
	glEnableVertexAttribArray(program->positionAttribute);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);


}

//from class slides
void DrawText(ShaderProgram *program, GLuint gameFontTex, string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	vector<float> vertexData;
	vector<float> texCoordData;

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

	//draw
	glUseProgram(program->programID);
	glBindTexture(GL_TEXTURE_2D, gameFontTex);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

//Displays title screen
GameMode displayTitle(SDL_Event* event, bool done, GLuint gameFont, ShaderProgram* program, Matrix& projectionMatrix, Matrix& modelViewMatrix) {
	while (!done) {
		while (SDL_PollEvent(event)) {
			if (event->type == SDL_QUIT || event->type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}

		glClear(GL_COLOR_BUFFER_BIT);

		glUseProgram(program->programID);
		//TITLE
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		modelViewMatrix.Translate(-0.2f, -3.0f, 0.0f);
		DrawText(program, gameFont, "Space Invaders", 0.4f, 0.0f);

		//START instruction
		program->SetProjectionMatrix(projectionMatrix);
		program->SetModelviewMatrix(modelViewMatrix);
		modelViewMatrix.Identity(); //resets the matrix to have no transformation
		modelViewMatrix.Translate(-2.5f, 1.0f, 0.0f);
		DrawText(program, gameFont, "Press SPACE to begin", 0.3f, 0.0f);

		const Uint8 *keys = SDL_GetKeyboardState(NULL); //gets keyboard state
		if (keys[SDL_SCANCODE_SPACE]) {
			return GAME_LEVEL;  //if space then start game
		}
		SDL_GL_SwapWindow(displayWindow);
	}
	return TITLE_SCREEN;
}

void playerControl(Mix_Chunk *shootSound, GameState &game, float elapsed, SheetSprite playerSprite, SheetSprite bulletSprite, const Uint8 *keys) {
	//Player controls
	if (keys[SDL_SCANCODE_A]) {
		if ( game.player->position.x  - playerSprite.width/ 2 > -3.70) {
			game.player->position.x -= elapsed;
		}
	}
	else if (keys[SDL_SCANCODE_D]) {
		if (game.player->position.x + playerSprite.width /2 < 3.70) {
			game.player->position.x += elapsed;
		}
	}
	else if (keys[SDL_SCANCODE_SPACE]) {
		//plays a sound when the user press space to shoot
		Mix_PlayChannel(-1, shootSound, 0);

		Entity * bullety = new Entity(bulletSprite, game.player->position.x, game.player->position.y + 1.0f, 0.0f, 0.0f, .5f, 0.0f);
		game.bulletsVec.push_back(bullety);
	}

}




void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Space Invaders", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	//Sets rendering area in pixels
	glViewport(0, 0, 640, 360);

	//Blend textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}


int main(int argc, char *argv[]) {
	Setup();

	//initializes SDL_mixer
	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);

	//loads sounds
	Mix_Chunk *startSound;
	startSound = Mix_LoadWAV("intro-to-game.wav");
	Mix_Chunk *shootSound;
	shootSound = Mix_LoadWAV("shoot.wav");

	//loads music
	Mix_Music *backgroundMusic;
	backgroundMusic = Mix_LoadMUS("BoxCat_Games_-_10_-_Epic_Song.mp3");

	//Plays intro sound
	Mix_PlayChannel(-1, startSound, 0);
	
		
	SDL_Event event;
	bool done = false;

	//time
	float lastFrameTicks = 0.0f;

	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl"); //supports texture
	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-4.0f, 4.0f, -3.0f, 3.0f, -1.0f, 1.0f);


	GLuint gameFont = LoadTexture("font1.png");
	GLuint gameSheet = LoadTexture("sheet.png");
	GameMode gMode = TITLE_SCREEN;
	
	//player ship
	SheetSprite playerSprite = SheetSprite(gameSheet, 112.0f / 1024.0f, 866.0f / 1024.0f, 112.0f / 1024.0f, 75.0f / 1024.0f, 0.2);
	Entity ship = Entity(playerSprite, 0.0f, -2.5f, 0.0f, .3f, .3f, 0.0f);
	GameState game = GameState(&ship);

	//bullet
	SheetSprite bulletSprite = SheetSprite(gameSheet, 858.0f / 1024.0f, 0.0f / 1024.0f, 9.0f / 1024.0f, 37.0f / 1024.0f, .1f);
	Entity bullet = Entity(bulletSprite, game.player->position.x, game.player->position.y + 1.0f, 0.0f, 0.0f, .5f, 0.0f);


	//GAME MODES
	while (!done) {
		switch (gMode) {
			case TITLE_SCREEN:
				gMode = displayTitle(&event, done, gameFont, &program, projectionMatrix, modelviewMatrix);
				break;
			case GAME_LEVEL:
				//plays background music  (-1) to loop forever
				Mix_PlayMusic(backgroundMusic, -1);

				while (!done) {
					while (SDL_PollEvent(&event)) {
						if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
							done = true;
						}
					}
					float ticks = (float)SDL_GetTicks() / 1000.0f;
					float elapsed = ticks - lastFrameTicks;
					lastFrameTicks = ticks;


					//gets keyboard state
					const Uint8 *keys = SDL_GetKeyboardState(NULL);

					glClear(GL_COLOR_BUFFER_BIT);
					glUseProgram(program.programID);


					playerControl(shootSound, game, elapsed, playerSprite, bulletSprite, keys);
					RenderGame(&program, projectionMatrix, modelviewMatrix, gameSheet, game);
					update(game, elapsed);
			

					SDL_GL_SwapWindow(displayWindow);
				}


		}
	}

	//clean up music and sounds on quit
	Mix_FreeChunk(startSound);
	Mix_FreeChunk(shootSound);
	Mix_FreeMusic(backgroundMusic);

	SDL_Quit();
	return 0;
}
