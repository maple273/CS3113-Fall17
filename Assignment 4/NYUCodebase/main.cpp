/*
Mary Qiu
Assignment #4: Platformer Game Demo

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
#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#define FIXED_TIMESTEP 0.0166666f


using namespace std;
SDL_Window* displayWindow;

enum EntityType {ENTITY_PLAYER, ENTITY_ENEMY, ENTITY_DOOR};

int mapWidth;
int mapHeight;
int** levelData;
int SPRITE_COUNT_X = 1.0f;
int SPRITE_COUNT_Y = 1.0f;
float TILE_SIZE = 0.1f;
int map;



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

class SheetSprite {
public:
	SheetSprite() {};   // default constructor
	SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size) :
		textureID(textureID), u(u), v(v), width(width), height(height), size(size) {};
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
	Vector3() {};   // default constructor
	Vector3(float x, float y, float z) : x(x), y(y), z(z) {};
	float x;
	float y;
	float z;
};

class Entity {
public:
	Entity() {};  // default constructor
	Entity(float posX, float posY, float posZ, EntityType type, bool isStatic) : position(posX, posY, posZ), entityType(type), isStatic(isStatic){}
	
	void Update(float elapsed, const Uint8 *keys);

	bool CollidesWith(Entity &entity);
	
	void Draw(ShaderProgram *program);

	void setSheetSprite(GLuint sheet);

	SheetSprite sprite;

	Vector3 position;
	Vector3 size;
	Vector3 velocity;
	Vector3 acceleration;

	bool isStatic;
	EntityType entityType;

	//Contact Flags: True - if collided   |  False - no collision
	bool collidedTop;
	bool collidedBottom;
	bool collidedLeft;
	bool collidedRight;
};

Entity player;
Entity enemy;
Entity door;

//converts to grid coordinates from entity's positions
void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
	*gridX = (int)(worldX / 0.1);
	*gridY = (int)(-worldY / 0.1);
}


void Entity::Draw(ShaderProgram *program) {
	sprite.Draw(program);
}
void Entity::Update(float elapsed, const Uint8 *keys ) {

	//player controls
	if (keys[SDL_SCANCODE_A]) {
		if (player.position.x / 2 > -3.70) {
			player.position.x -= elapsed;
		}
	}
	else if (keys[SDL_SCANCODE_D]) {
		if (player.position.x / 2 < 3.70) {
			player.position.x += elapsed;
		}
	}
	else if (keys[SDL_SCANCODE_SPACE]) { //to jump
		player.position.y += 1.0;
	}
};

bool Entity::CollidesWith(Entity &entity) {
	//collision
	//velocity of the entity reset to 0 and related collision flags are set to true for collision detection
	if (entity.position.y < entity.position.y - entity.position.y/2) {
		entity.velocity.y = 0;
		return entity.collidedBottom = true;
	}
	else if (entity.position.y > entity.position.y - entity.position.y / 2) {
		entity.velocity.y = 0;
		return entity.collidedTop = true;
	}
	else if (entity.position.x < entity.position.x- entity.position.x / 2) {
		entity.velocity.x = 0;
		return entity.collidedLeft = true;
	}
	else if (entity.position.x < entity.position.x - entity.position.x / 2) {
		entity.velocity.x = 0;
	    return	entity.collidedRight = true;
	}
	return false;
};

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

//reads header data from file
bool readHeader(ifstream &stream) {
	//Reads key = value pairs
	string line;
	mapWidth = -1;
	mapHeight = -1;

	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "width") {
			//Converts a string into an integer and assigns it to mapWidth
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height") {
			//Converts a string into an integer and assigns it to mapHeight
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new int*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new int[mapWidth];
		}
		return true;
	}
}

//reads tile data from file
bool readLayerData(ifstream &stream) {
	string line;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "data") {
			for (int y = 0; y < mapHeight; y++) {
				getline(stream, line);
				istringstream lineStream(line);
				string tile;
				for (int x = 0; x < mapWidth; x++) {
					getline(lineStream, tile, ',');
					//Converts a string into an integer and assigns it to val
					int val = atoi(tile.c_str());
					if (val > 0) {
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1;
					}
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	return true;
}

//places the entity at the postion
void placeEntity(string type, float x, float y, float z) {
	if (type == "Player") {
		player = Entity(x, y, z, ENTITY_PLAYER, false);
	}
	else if (type == "Enemy") {
		enemy = Entity(x, y, z, ENTITY_ENEMY, false);
	}
	else if (type == "door") {
		door = Entity(x, y, z, ENTITY_DOOR, false);
	}
}

//attempt to assign sprites to apprioprate entities but i am having trouble converting spritesheet to .xml file
/*void setSheetSprite(GLuint sprite1) {
	SheetSprite player = SheetSprite(sprite1, )
	SheetSprite enemy = SheetSprite(sprite1, )
	SheetSprite door = SheetSprite(sprite1, )
	player.sprite = player;
	enemy.sprite = enemy;
	door.sprite = door;
	
}*/

//reads entity data from file
bool readEntityData(ifstream &stream) {
	string line;
	string type;
	while (getline(stream, line)) {
		if (line == "") { break; }
		istringstream sStream(line);
		string key, value;
		getline(sStream, key, '=');
		getline(sStream, value);
		if (key == "type") {
			type = value;
		}
		else if (key == "location") {
			istringstream lineStream(value);
			string xPosition, yPosition, zPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			getline(lineStream, zPosition, ',');
			float placeX = atoi(xPosition.c_str())*TILE_SIZE;
			float placeY = atoi(yPosition.c_str())*-TILE_SIZE;
			float placeZ = atoi(zPosition.c_str())*-TILE_SIZE;
			placeEntity(type, placeX, placeY, placeZ);
		}
	}
	return true;
}

//get keyword and reads appropiate data from file
void readFile(ifstream &infile) {
	string line;

	//reads each line in file
	while (getline(infile, line)) {
		if (line == "[header]") {
			if (!readHeader(infile)) {
				assert(false);
			}
		}
		else if (line == "[layer]") {
			readLayerData(infile);
		}
		else if (line == "[ObjectsLayer]") {
			readEntityData(infile);
		}
	}
}

void renderGameMap(vector<float> vertexData, vector<float> texCoordData, ShaderProgram program) {
	for (int y = 0; y < mapHeight; y++) {
		for (int x = 0; x < mapWidth; x++) {
			float u = (float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X;
			float v = (float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y;
			float spriteWidth = 1.0f / (float)SPRITE_COUNT_X;
			float spriteHeight = 1.0f / (float)SPRITE_COUNT_Y;
			vertexData.insert(vertexData.end(), {
				TILE_SIZE * x, -TILE_SIZE * y,
				TILE_SIZE * x, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				TILE_SIZE * x, -TILE_SIZE * y,
				(TILE_SIZE * x) + TILE_SIZE, (-TILE_SIZE * y) - TILE_SIZE,
				(TILE_SIZE * x) + TILE_SIZE, -TILE_SIZE * y
			});
			texCoordData.insert(texCoordData.end(), {
				u, v,
				u, v + (spriteHeight),
				u + spriteWidth, v + (spriteHeight),
				u, v,
				u + spriteWidth, v + (spriteHeight),
				u + spriteWidth, v
			});
		}
	}
	glBindTexture(GL_TEXTURE_2D, map);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program.texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, mapWidth*mapHeight * 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);
}

void Setup() {
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Platformer Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
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
	ShaderProgram program(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");

	SDL_Event event;
	bool done = false;

	//loading textures for players, enemies, door, and map
	GLuint sprites = LoadTexture("spritesheet_rgba.png");
	GLuint background = LoadTexture("backgrounds.png");

	
	Matrix projectionMatrix;
	Matrix modelviewMatrix;
	projectionMatrix.SetOrthoProjection(-4.0f, 4.0f, -3.0f, 3.0f, -1.0f, 1.0f);

	//time
	float lastFrameTicks = 0.0f;

	//
	vector<float> vertexData;
	vector<float> texCoordData;

	//Checks to see if a key is pressed 
	const Uint8 *keys = SDL_GetKeyboardState(NULL);


	//GAME LOOP
	while (!done) {
		//time
		float ticks = (float)SDL_GetTicks() / 1000.0f;
		float elapsed = ticks - lastFrameTicks;  //# of seconds elapsed since last frame
		lastFrameTicks = ticks;


		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}
		}
		glClear(GL_COLOR_BUFFER_BIT); 

		glUseProgram(program.programID);

		//opens file
		ifstream infile("tilemap1.txt");
		//reads file
		readFile(infile);
		renderGameMap(vertexData, texCoordData, program);

		SDL_GL_SwapWindow(displayWindow);
	}

	SDL_Quit();
	return 0;
}