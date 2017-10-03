/*
Mary Qiu

Assignment #2: PONG game

- Doesn't need to keept score, BUT it must detect player wins
- Can use images or untextured polygons
- Can use keyboard, mouse or joystick input
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

using namespace std;
SDL_Window* displayWindow;


class Entity {
public:
	//constructor
	Entity(float x, float y, float width, float height) : x(x), y(y), width(width), height(height) {}

	float x;
	float y;

	float rotation;
	int textureID;

	float width;
	float height;

	float velocity;
	float direction_x;
	float direction_y;

	void Draw(ShaderProgram *p) {
		glEnableVertexAttribArray(p->positionAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(p->positionAttribute);
	};
	
};


void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("PONG", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
};


int main(int argc, char *argv[]) {
	Setup();

#ifdef _WINDOWS
	glewInit();
#endif

	SDL_Event event;
	bool done = false;

	glViewport(0, 0, 640, 360); //Sets rendering area in pixels
	ShaderProgram program(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl"); //supports non-textures

	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-4.0f, 4.0f, -3.0f, 3.0f, -1.0f, 1.0f);

	//time
	float lastFrameTicks = 0.0f;


	//OBJECTS
	Entity player_1(-3.5, 0.0, 0.2, 0.5); //left
	Entity player_2(-3.5, 0.0, 0.2, 0.5);  //right
	Entity ball(0.0, 0.0, 0.2, 0.2);
	Entity divider(0.0, 0.0, 0.0, 0.0);

	player_1.velocity = 1.90f;
	player_2.velocity = 1.90f;
	ball.velocity = 1.80f;

	player_1.direction_x = cos(rand() / 1000.0f);
	player_1.direction_y = sin(rand() / 1000.0f);
	player_2.direction_x = cos(rand() / 1000.0f);
	player_2.direction_y = sin(rand() / 1000.0f);
	ball.direction_x = cos(rand() / 1000.0f);
	ball.direction_y = sin(rand() / 1000.0f);

	//Checks to see if a key is pressed 
	const Uint8 *keys = SDL_GetKeyboardState(NULL);


	//GAME LOOP
	while (!done) {
		//time
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;  //# of seconds elapsed since last frame
		lastFrameTicks = ticks;
		
		//position += cos/sin *units a sec * elapsed
		ball.y += ball.velocity * ball.direction_y * elapsed;
		ball.x += ball.velocity * ball.direction_x * elapsed;

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT); //black bg
	  
		glUseProgram(program.programID);
		//PLAYER 1 - LEFT PADDLE	
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		float leftPaddle[] = { -4.0f, -0.8f + player_1.y, -3.75f, -0.8f + player_1.y, -3.75f, 0.5f + player_1.y, -4.0f, -0.8f + player_1.y, -3.75f, 0.5f + player_1.y, -4.0f, 0.5f + player_1.y };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, leftPaddle);
		player_1.Draw(&program);

		//PLAYER 2 - RIGHT PADDLE
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		float rightPaddle[] = { 3.75f, -0.8f + player_2.y, 4.0f, -0.8f + player_2.y, 4.0f, 0.5f + player_2.y, 3.75f, -0.8f + player_2.y, 4.0f, 0.5f + player_2.y, 3.75f, 0.5f + player_2.y };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, rightPaddle);
		player_2.Draw(&program);

		//BALL
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		float verticesBall[] = { 0.1f + ball.x, -0.1f + ball.y, -0.1f + ball.x, -0.1f + ball.y, 0.1f + ball.x, 0.1f + ball.y, -0.1f + ball.x, -0.1f + ball.y, -0.1f + ball.x, 0.1f + ball.y, 0.1f + ball.x, 0.1f + ball.y };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, verticesBall);
		ball.Draw(&program);

		//DIVIDER
		program.SetModelviewMatrix(modelviewMatrix);
		program.SetProjectionMatrix(projectionMatrix);
		float dividerVertices[] = { -0.05f, -3.0f, 0.05f, -3.0f, 0.05f, 3.0f, -0.05f, -3.0f, 0.05f, 3.0f, -0.05f, 3.0f };
		glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, dividerVertices);
		divider.Draw(&program);


		//CONTROLS
		//Player 1 - left paddle controls
		if (keys[SDL_SCANCODE_W]) {
			if (player_1.y + player_1.height / 2 < 2.75) { //left paddle can only move within the screen
				player_1.y += elapsed;
			}
		
		}
		if (keys[SDL_SCANCODE_S]) {
			if (player_1.y - player_1.height / 2 > -2.45) {
				player_1.y -= elapsed;
			}

		}

		//Player 2 - right paddle controls
		if (keys[SDL_SCANCODE_UP]) {
			if (player_2.y + player_2.height / 2 < 2.75) {  //right paddle can only move within the screen
				player_2.y += elapsed;
			}

		}
		if (keys[SDL_SCANCODE_DOWN]) {
			if (player_2.y - player_2.height / 2 > -2.45) {
				player_2.y -= elapsed;
				
			}

		}



		//COLLISIONS
		else if (ball.y > 3.0f) {
			ball.direction_y = -ball.direction_y;
			ball.y += ball.velocity * elapsed * ball.direction_y;
			ball.x += ball.velocity * elapsed * ball.direction_x;
		}
		else if (ball.x - ball.width / 2 <= (player_1.x + player_1.width / 2) && ball.y + ball.height / 2 <= (player_1.y + player_1.height / 2 + ball.height) && ball.y + ball.height / 2 >= (player_1.y - player_1.height / 2 + ball.height)){
			ball.direction_x *= -1.0;
			if (ball.direction_x < player_1.x) { cout << "Player 2 wins!!\n"; }
		}
		else if (ball.x + ball.width / 2 >= (player_2.x - player_2.width / 2) && ball.y + ball.height / 2 <= (player_2.y + player_2.height / 2 + ball.height) && ball.y + ball.height / 2 >= (player_2.y - player_2.height / 2 + ball.height)){
			ball.direction_x *= -1.0;
			if (ball.direction_x > player_2.x) { cout << "Player 1 wins!!\n"; }
		}

	

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}
