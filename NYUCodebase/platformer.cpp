#ifdef _WINDOWS
#define _CRT_SECURE_NO_WARNINGS
#endif
#include <stdio.h>
#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "Matrix.h"
#include <vector>
#include <string>
#include <SDL_mixer.h>
#include <fstream>
#include <string>
#include <iostream>
#include <ostream>
#include <sstream>



#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

#define TILE_SIZE 0.4f 
#define SPRITE_COUNT_X 30
#define SPRITE_COUNT_Y 30
#define LEVEL_HEIGHT 32
#define LEVEL_WIDTH 128
#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6


SDL_Window* displayWindow;
unsigned int ** levelData;
int mapWidth;
int mapHeight;
Matrix modelMatrixForWorld;
Matrix modelMatrix;
Matrix modelMatrixEnemy;
Matrix modelMatrixStart;
Matrix projectionMatrix;
Matrix viewMatrix;
ShaderProgram *program;


const Uint8 *keys = SDL_GetKeyboardState(NULL);
GLuint sheet;
GLuint sheet_rgba;
int firstFrame = 0;
float placeX;
float placeY;
float speed;


int gameState;

using namespace std;



void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY)
{
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);

}

bool isSolid(int ind) {
	//(val == 124 || val == 64 || val == 96 || val == 97 || val == 512 || val == 487 || val == 484
	//|| val == 488 || val == 516 || val == 518 || val == 586 || val == 557 || val == 159
	if (ind == 123)
		return true;
	if (ind == 152)
		return true;
	if (ind == 68)
		return true;
	if (ind == 483)
		return true;
	if (ind == 98)
		return true;
	if (ind == 556)
		return true;
	if (ind == 563)
		return true;
	if (ind == 158)
		return true;
	if (ind == 63)
		return true;
	if (ind == 92)
		return true;
	if (ind == 332)
		return true;
	if (ind == 497)
		return true;
	if (ind == 557)
		return true;
	if (ind == 592)
		return true;
	if (ind == 218)
		return true;
	if (ind == 183)
		return true;
	if (ind == 585)
		return true;
	if (ind == 529)
		return true;


	return false;


}


bool atEnd(int ind){
	if (ind == 313)
		return true;
	return false;


}

bool isDead(int ind) {
	if (ind == 574)
		return true;
	if (ind == 578)
		return true;
	if (ind == 11)
		return true;

	return false;

}


void DrawText(ShaderProgram *program, int fontTexture, std::string text, float size, float spacing) {
	float texture_size = 1.0 / 16.0f;
	std::vector<float> vertexData;
	std::vector<float> texCoordData;
	for (int i = 0; i < text.size(); i++) {
		float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
		float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
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
	glUseProgram(program->programID);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glBindTexture(GL_TEXTURE_2D, fontTexture);
	glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}




class Tile
{
public:
	float xPos;
	float yPos;
	bool isSolid;
	int solidint;
	float width;
	float height;
	unsigned int value;
	Tile(float x, float y, bool solid, float wid, float hei) : xPos(x), yPos(y), isSolid(solid), width(wid), height(hei)
	{

	}
};

std::vector<Tile*> allOfTheTiles;



float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

class Entity
{
public:
	float xPos;
	float yPos;
	int gridx = 0;
	int gridy = 0;
	float xLimitLeft;
	float xLimitRight;
	float yLimitUp;
	int textureID;
	float width;
	float height;
	float velocity_x;
	float velocity_y;
	float acceleration_x;
	float acceleration_y;
	float friction_x = 1.1;
	float friction_y = 2.5;
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;
	bool isAtEnd = false;
	bool goingRight = true;
	bool isAlive = true;
	string state;

	Entity(float x, float y, int texture, float width, float velX, float velY, float accelX, float accelY, float fricX, float fricY, bool collide) : xPos(x), yPos(y), textureID(texture),
		acceleration_x(accelX), acceleration_y(accelY), velocity_x(velX), velocity_y(velY), collidedBottom(collide)
	{

	}
	void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
		int spriteCountY, int posX, int posY, Matrix modelM);
	void setPlayer(float x, float y, Matrix modelM)
	{
		
		xPos += x;
		yPos += y;
		modelM.identity();
		modelM.Translate(xPos, yPos, 0);
		program->setModelMatrix(modelM);

	}
	void collide() {
		worldToTileCoordinates(xPos, yPos - TILE_SIZE/2 , &gridx, &gridy);
		if (isSolid(levelData[gridy][gridx])){
			collidedBottom = true;
			
			velocity_y = 0;
			acceleration_y = 0;
		}
		else {
			collidedBottom = false;
			acceleration_y = 9.81;
		}
		worldToTileCoordinates(xPos, yPos + TILE_SIZE / 2, &gridx, &gridy);
		if (isSolid(levelData[gridy][gridx])){
			collidedTop = true;
			
			velocity_y = 0;
			acceleration_y = 9.81;
		}
		else {
			collidedTop = false;
		}
		worldToTileCoordinates(xPos + TILE_SIZE / 2, yPos, &gridx, &gridy);
		if (isSolid(levelData[gridy][gridx])){
			
			collidedRight = true;
			velocity_x = 0;
			acceleration_x = 0;

		}
		else if (atEnd(levelData[gridy][gridx])){
			isAtEnd = true;

		}
		else {
			collidedRight = false;
		}
		worldToTileCoordinates(xPos - TILE_SIZE / 2, yPos, &gridx, &gridy);
		if (isSolid(levelData[gridy][gridx])){
			
			collidedLeft = true;
			velocity_x = 0;
			acceleration_x = 0;
		}
		else {
			collidedLeft = false;
		}
	}

	void moveEnemy(float elapsed) {
		if (state == "Patrol"){
			if (xPos < xLimitRight && goingRight){
				acceleration_x = 5;
			}
			else if (!goingRight && xPos < xLimitLeft){
				acceleration_x = 5;
				goingRight = true;
			}

			else {
				acceleration_x = -5;
				goingRight = false;
			}
		}
		else if (state == "ChasingLeft" && xPos > xLimitLeft){
			acceleration_x = -5;

		}
		else if (state == "ChasingRight" && xPos < xLimitRight) {
			acceleration_x = 5;
		}
		else{
			acceleration_x = 0;
		}
		velocity_x = lerp(velocity_x, 0.0f, elapsed * 1.1);
		velocity_y = lerp(velocity_y, 0.0f, elapsed * friction_y);

		velocity_x += acceleration_x * elapsed;
		velocity_y += acceleration_y * elapsed;
		xPos += velocity_x * elapsed;
		yPos -= velocity_y * elapsed;



		modelMatrixEnemy.identity();
		modelMatrixEnemy.Translate(xPos, yPos, 0);
		program->setModelMatrix(modelMatrixEnemy);

	}


	void movePlayer(float elapsed)
	{
		if (velocity_x > 9 || velocity_x < -9){
			acceleration_x = 0;
		}
		
		if (velocity_y > 2 || velocity_y < -2){
			acceleration_y = 0;
		}
		

		velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
		velocity_y = lerp(velocity_y, 0.0f, elapsed * friction_y);

		velocity_x += acceleration_x * elapsed;
		velocity_y += acceleration_y * elapsed;
		xPos += velocity_x * elapsed;
		yPos -= velocity_y * elapsed;

	

		modelMatrix.identity();
		modelMatrix.Translate(xPos, yPos, 0);
		if (acceleration_x < 0)
			modelMatrix.Scale(-1, 1, 0);
		program->setModelMatrix(modelMatrix);

	}

	void isPlayerDead() {
		if (isDead(levelData[gridy][gridx])){
			isAlive = false;
		
		}
		else {
			isAlive = true;;
		}


	}
	
	
	void checkMovement(){

		if (keys[SDL_SCANCODE_RIGHT] && gridx < xLimitRight)
		{

			float penetration;
			if (!collidedRight){

				//alien.setPlayer(speed, 0);
				acceleration_x = 10;

			}

			else {
				velocity_x = 0;
				penetration = fabs(gridx * TILE_SIZE - (xPos - TILE_SIZE / 2));
				setPlayer(-penetration + 0.0001, 0, modelMatrix);


			}


		}
		else if (keys[SDL_SCANCODE_LEFT] && xPos > xLimitLeft)
		{
			float penetration;
			if (!collidedLeft){
				acceleration_x = -10;
			}
			else {
				velocity_x = 0;
				penetration = fabs(gridx * TILE_SIZE - (xPos - TILE_SIZE * 3 / 2));
				setPlayer(penetration - 0.0001, 0, modelMatrix);
			}
		}
		else {
			acceleration_x = 0;


		}

		if (keys[SDL_SCANCODE_UP] && collidedBottom)
		{
			float penetration;
			//Mix_PlayChannel(-1, deathSound, 0);
			if (!collidedTop){
				
				acceleration_y = -50.81;
			}
			else {
				penetration = fabs(gridy * TILE_SIZE + (yPos + TILE_SIZE / 2));
				setPlayer(0, -penetration + 0.0001, modelMatrix);
			}


		}
		else if (keys[SDL_SCANCODE_DOWN])
		{

			float penetration;

			if (!collidedBottom)
			{
				acceleration_y = 9.81;
			}
			else {
				penetration = fabs(gridy * TILE_SIZE + (yPos + TILE_SIZE / 2));
				setPlayer(0, penetration - 0.001, modelMatrix);
			}


		}



		if (!collidedBottom)
		{
			acceleration_y -= 6.81;
		}
		else {
			float penetration = fabs(gridy * TILE_SIZE + (yPos + TILE_SIZE / 2));
			setPlayer(0, penetration - 0.001, modelMatrix);
		}

	}

	void seePlayer (Entity& player){
		
		if (fabs(player.yPos - yPos) < .25f && fabs(player.xPos - (xPos + 40)) < 3){
			if (player.xPos - (xPos + 40) <  0) {
				state = "ChasingLeft";
			}
			else if (player.xPos - (xPos + 40) > 0) {
				state = "ChasingRight";
			}
			if (fabs(player.xPos - (xPos + 40)) < 0.25){
				player.isAlive = false;
				cout << "ur mum" << endl;
			}
		}
		else {
			state = "Patrol";
		}
	}

	
};

void Entity::DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
	int spriteCountY, int posX, int posY, Matrix modelM)
{
	/*posX *=TILE_SIZE;
	posY *= TILE_SIZE;*/
	float u = (float)(((int)index) % spriteCountX) / (float)spriteCountX;
	float v = (float)(((int)index) / spriteCountX) / (float)spriteCountY;
	float spriteWidth = 1.0 / (float)spriteCountX;
	float spriteHeight = 1.0 / (float)spriteCountY;
	GLfloat texCoords[] = {
		u, v + spriteHeight,
		u + spriteWidth, v,
		u, v,
		u + spriteWidth, v,
		u, v + spriteHeight,
		u + spriteWidth, v + spriteHeight
	};
	float vertices[] = { -0.5f*TILE_SIZE, -0.5f*TILE_SIZE,
		0.5f*TILE_SIZE, 0.5f*TILE_SIZE,
		-0.5f*TILE_SIZE, 0.5f*TILE_SIZE,
		0.5f*TILE_SIZE, 0.5f*TILE_SIZE,
		-0.5f*TILE_SIZE, -0.5f*TILE_SIZE,
		0.5f*TILE_SIZE, -0.5f*TILE_SIZE };
	program->setModelMatrix(modelM);
	modelM.identity();
	modelM.Translate(xPos, yPos, 0);
	glUseProgram(program->programID);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glClearColor(0.0, 0.0, 0.0, 0.0);
	glBindTexture(GL_TEXTURE_2D, sheet_rgba);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
	//glBlendFunc(GL_ONE, GL_ZERO);
}





class TileMap
{
public:
	float xPos;
	float yPos;
	TileMap(float x, float y) : xPos(x), yPos(y)
	{

	}
	
private:

};




class SheetSprite 
{
	public:
		SheetSprite();
		SheetSprite(unsigned int textureIDD, float U, float V, float Width, float Height, float
			Size) : textureID(textureIDD), u(U), v(V), width(Width), height(Height), size(Size)
		{
	
		}
		void Draw(ShaderProgram *program);
		float size;
		unsigned int textureID;
		float u;
		float v;
		float width;
		float height;
	
};

void SheetSprite::Draw(ShaderProgram* program) {
	
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
			(-0.5f) * size * aspect, -0.5f * size,
			(0.5f) * size * aspect, 0.5f * size,
			(-0.5f) * size * aspect, 0.5f * size,
			(0.5f)* size * aspect, 0.5f * size,
			(-0.5f) * size * aspect, -0.5f * size,
			(0.5f)* size * aspect, -0.5f * size };
		
		glUseProgram(program->programID);
		glBindTexture(GL_TEXTURE_2D, textureID);
		glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
		glEnableVertexAttribArray(program->positionAttribute);
		glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
		glEnableVertexAttribArray(program->texCoordAttribute);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(program->positionAttribute);
		glDisableVertexAttribArray(program->texCoordAttribute);
}
	








GLuint LoadTexture(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, surface->w, surface->h, 0, GL_RGB,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}

GLuint LoadTextureRGBA(const char *image_path) {
	SDL_Surface *surface = IMG_Load(image_path);
	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surface->w, surface->h, 0, GL_RGBA,
		GL_UNSIGNED_BYTE, surface->pixels);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	SDL_FreeSurface(surface);
	return textureID;
}



bool readHeader(std::ifstream &stream) {
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
			mapWidth = atoi(value.c_str());
		}
		else if (key == "height"){
			mapHeight = atoi(value.c_str());
		}
	}
	if (mapWidth == -1 || mapHeight == -1) {
		return false;
	}
	else { // allocate our map data
		levelData = new unsigned int*[mapHeight];
		for (int i = 0; i < mapHeight; ++i) {
			levelData[i] = new unsigned int[mapWidth];
		}
		return true;
	}
}


void render() {

	std::vector<float> vertexData;
	std::vector<float> texCoordData;

	float marigin = 2.0f / 629.0f;

	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] != 0)
			{
				
				float u = ((float)(((int)levelData[y][x]) % SPRITE_COUNT_X) / (float)SPRITE_COUNT_X) + marigin;
				float v = ((float)(((int)levelData[y][x]) / SPRITE_COUNT_X) / (float)SPRITE_COUNT_Y) + marigin;
				float spriteWidth = (1.0f / (float)SPRITE_COUNT_X) - 2 * marigin;
				float spriteHeight = (1.0f / (float)SPRITE_COUNT_Y) - 2 * marigin;
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
	}
	//forworld
	program->setModelMatrix(modelMatrixForWorld);
	program->setViewMatrix(viewMatrix);
	program->setProjectionMatrix(projectionMatrix);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	
	glUseProgram(program->programID);
	glBindTexture(GL_TEXTURE_2D, sheet);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, vertexData.size() / 2);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);



}


bool readLayerData(std::ifstream &stream)
{
	
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
					unsigned int val = (unsigned int)atoi(tile.c_str());
					if (val > 0) {
						//float x, float y, bool solid, float wid, float hei)
						Tile* newTile = new Tile(x*TILE_SIZE, y*TILE_SIZE, false, TILE_SIZE / 2, TILE_SIZE / 2);
						//os << val << endl;
						newTile->value = val;
						if (val == 124 || val == 64 || val == 96 || val == 97 || val == 512 || val == 487 || val == 484
						|| val == 488 || val == 516 || val == 518 || val == 586 || val == 557 || val == 159)
						{
							newTile->isSolid = true;
							
							newTile->solidint = 1;
						}
						else {
							newTile->solidint = 5;
						}
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1; 
						allOfTheTiles.push_back(newTile);
						

					}
				
					else {
						levelData[y][x] = 0;
					}
				}
			}
		}
	}
	//os.close();
	return true;
}
bool readEntityData(std::ifstream &stream) {
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
			string xPosition, yPosition;
			getline(lineStream, xPosition, ',');
			getline(lineStream, yPosition, ',');
			float placeX = atoi(xPosition.c_str()) / 30.0f * TILE_SIZE;
			float placeY = atoi(yPosition.c_str()) / 30.0f * -TILE_SIZE;
			cout << "X:" << placeX << " Y: " << placeY  << endl;
		}
	}
	return true;
}
void read(string world) {
	ifstream infile(world);
	string line;
	if (!infile)
	{
		cout << "OH NOOOOOOOOOOOO!" << endl;
	}
	else
	{
		while (getline(infile, line))
		{
			if (line == "[header]")
			{
				if (!readHeader(infile))
				{
					break;
				}
			}
			else if (line == "[layer]")
			{
				readLayerData(infile);
			}
			else if (line == "[Player]")
			{
				readEntityData(infile);
				
			}
			else if (line == "[Enemies]"){
				readEntityData(infile);

			}
		}
	}

}




template<class H>
void clearTheHeap(vector<H*>& vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		delete vec[i];
		vec[i] = nullptr;
	}
	vec.clear();
}





int main(int argc, char *argv[])
{

	//allows me to print to the console. only affects windows users.
#ifdef _WIN32
	AllocConsole();
	freopen("conin$", "r", stdin);
	freopen("conout$", "w", stdout);
	freopen("conout$", "w", stderr);
#endif





	//==========================================================================================================================================
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Jet's World", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 320, SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);
#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 360);
	projectionMatrix.setOrthoProjection(-5.33f, 5.33f, -3.0f, 3.0f, -1.0f, 1.0f);
	program = new ShaderProgram(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
	glUseProgram(program->programID);


	//=============================================================================================================================================
	//forworld
	program->setModelMatrix(modelMatrixForWorld);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);
	speed = 1;

	SDL_Event event;
	bool done = false;
	string world1 = "world.txt";
	string world2 = "secondWorld.txt";
	read(world1);
	sheet = LoadTexture("spritesheet.png");
	sheet_rgba = LoadTextureRGBA("spritesheet_rgba.png");
	GLuint fontTexture = LoadTextureRGBA("font2.png");
	string welcome = "Press Space to start";
	string death = "You died, press start to restart";
	float lastFrameTicks = 0.0f;
	float ticks;
	float elapsed;
	gameState = 0;

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	Mix_Chunk *deathSound = Mix_LoadWAV("death.wav");

	Mix_Music *music;
	music = Mix_LoadMUS("BackgroundMusic.mp3");

	Mix_PlayMusic(music, -1);
	
	//===============================================================================================================================================
	Entity alien(placeX, placeY, sheet, TILE_SIZE/2, 0, 0, 0, 0, 0, 0.0f,true);
	Entity enemy1(placeX, placeY, sheet, TILE_SIZE / 2, 0, 0, 0, 0, 0, 0.0f, true);
	TileMap tileMap(-alien.xPos, -alien.yPos);
	//===============================================================================================================================================



	//to position the alien at the beginning of the level
	
	alien.setPlayer(5.4f, -6.6f, modelMatrix);
	enemy1.state = "Patrol";
	alien.xLimitLeft = alien.xPos;
	alien.xLimitRight = 114;
	alien.acceleration_x = 0;
	alien.acceleration_y = 0;

	
	//int * x = nullptr;
	//int *y= nullptr;
	while (!done) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
				done = true;
			}

		}
		
		ticks = (float)SDL_GetTicks() / 1000.0f;
		elapsed = ticks - lastFrameTicks;
		lastFrameTicks = ticks;
		speed = 1.5 * elapsed;

		glClear(GL_COLOR_BUFFER_BIT);

		glDisable(GL_CULL_FACE);
		//render();


		if (gameState == 0){

			DrawText(program, fontTexture, welcome, 0.2f, 0.02f);
			
			modelMatrix.identity();
			modelMatrix.setPosition(-2.5, 1, 1);

			program->setModelMatrix(modelMatrix);
			program->setProjectionMatrix(projectionMatrix);
			program->setViewMatrix(viewMatrix);
			if (keys[SDL_SCANCODE_SPACE]){
				
				gameState = 1;

			}

		}
		else if (gameState == 1) {
			
			alien.collide();
			alien.checkMovement();
			alien.isPlayerDead();

			if (alien.isAtEnd){
				read(world2);
				alien.isAtEnd = false;
				alien.velocity_x = 0;
				alien.velocity_y = 0;
				alien.acceleration_x = 0;
				alien.acceleration_y = 0;
				//enemy1.DrawSpriteSheetSprite(program, 169, 30, 30, placeX, placeY, modelMatrixEnemy);
				gameState = 2;
				//enemy1.setPlayer(5.4f, -6.0f, modelMatrixEnemy);
				modelMatrixEnemy.identity();
				modelMatrixEnemy.Translate(40, -9.75, 0);
				program->setModelMatrix(modelMatrixEnemy);
			}

			if (!alien.isAlive)
			{
				Mix_PlayChannel(-1, deathSound, 0);
				gameState = 99;
	
			}

			render();

			float fixedElapsed = elapsed;
			if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
				fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
			}
			while (fixedElapsed >= FIXED_TIMESTEP) {
				fixedElapsed -= FIXED_TIMESTEP;
				alien.movePlayer(FIXED_TIMESTEP);
			}
			alien.movePlayer(fixedElapsed);

			alien.DrawSpriteSheetSprite(program, 21, 30, 30, placeX, placeY, modelMatrix);
			

			/*
			modelMatrixEnemy.identity();
			modelMatrixEnemy.setPosition(alien.xPos, alien.yPos, 0);
			*/

			
			program->setModelMatrix(modelMatrixEnemy);

			viewMatrix.identity();
			viewMatrix.Translate(-alien.xPos, -alien.yPos, 0);

			
		}
		else if (gameState == 2){
			//alien.setPlayer(-alien.xPos + 4.0f, -alien.yPos - 4.0f, modelMatrix);
			alien.collide();
			alien.checkMovement();
		

			float fixedElapsed = elapsed;
			if (fixedElapsed > FIXED_TIMESTEP * MAX_TIMESTEPS) {
				fixedElapsed = FIXED_TIMESTEP * MAX_TIMESTEPS;
			}
			while (fixedElapsed >= FIXED_TIMESTEP) {
				fixedElapsed -= FIXED_TIMESTEP;
				alien.movePlayer(FIXED_TIMESTEP);
				//enemy1.moveEnemy(fixedElapsed);
			}

			render();
			enemy1.xLimitRight = 3;
			enemy1.xLimitLeft = -13;
			enemy1.acceleration_x = 2;
			enemy1.moveEnemy(0.0166);
			enemy1.seePlayer(alien);

			
			if (!alien.isAlive){
				Mix_PlayChannel(-1, deathSound, 0);

				gameState = 99;
			}

			program->setModelMatrix(modelMatrixEnemy);
			modelMatrixEnemy.identity();
			enemy1.yPos = -9.801;
			modelMatrixEnemy.Translate(40 + enemy1.xPos, -9.75, 0);
			modelMatrix.identity();
			alien.movePlayer(fixedElapsed);
			enemy1.DrawSpriteSheetSprite(program, 169, 30, 30, placeX, placeY, modelMatrixEnemy);
		
			alien.DrawSpriteSheetSprite(program, 21, 30, 30, placeX, placeY, modelMatrix);
			//cout << "X: " << alien.xPos << " Y: " << alien.yPos << endl;
			viewMatrix.identity();
			viewMatrix.Translate(-alien.xPos, -alien.yPos, 0);

		}
		else if (gameState == 99){
			
			DrawText(program, fontTexture, death, 0.2f, 0.02f);

			modelMatrix.identity();
			modelMatrix.setPosition(-2.5, 1, 1);

			program->setModelMatrix(modelMatrix);

			if (keys[SDL_SCANCODE_SPACE]){

				read(world1);
				gameState = 1;
				alien.setPlayer(-alien.xPos, -alien.yPos, modelMatrix);
				alien.setPlayer(5.4f, -6.6f, modelMatrix);
				alien.velocity_x = 0;
				alien.velocity_y = 0;
				alien.acceleration_x = 0;
				alien.acceleration_y = 0;

			}

		}
		
		SDL_GL_SwapWindow(displayWindow);

	}

	delete program;
	delete levelData;
	clearTheHeap(allOfTheTiles);
//#ifdef _WIN32
//	std::cin.get();
//#endif


	SDL_Quit();
	return 0;
}
