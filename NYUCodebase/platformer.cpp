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

SDL_Window* displayWindow;
unsigned int ** levelData;
int mapWidth;
int mapHeight;
Matrix modelMatrixForWorld;
Matrix modelMatrix;
Matrix projectionMatrix;
Matrix viewMatrix;
ShaderProgram *program;
std::vector<float> vertexData;
std::vector<float> texCoordData;
std::vector<Tile*> allOfTheTiles;
const Uint8 *keys = SDL_GetKeyboardState(NULL);
GLuint sheet;
int firstFrame = 0;
float placeX;
float placeY;
float speed;

using namespace std;


float lerp(float v0, float v1, float t) {
	return (1.0 - t)*v0 + t*v1;
}

class Entity
{
public:
	float xPos;
	float yPos;
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
	float friction_x;
	float friction_y;
	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;


	Entity(float x, float y, int texture, float width, float velX, float velY, float accelX, float accelY, float fricX, float fricY) : xPos(x), yPos(y), textureID(texture),
		acceleration_x(accelX), acceleration_y(accelY), velocity_x(velX), velocity_y(velY)
	{

	}
	void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
		int spriteCountY, int posX, int posY);
	void setPlayer(float x, float y)
	{
		
		xPos += x;
		yPos += y;
		modelMatrix.identity();
		modelMatrix.Translate(xPos, yPos, 0);
		program->setModelMatrix(modelMatrix);

	}
	void movePlayer(float elapsed)
	{

		/*velocity_x = lerp(velocity_x, 0.0f, FIXED_TIMESTEP * friction_x);
		velocity_y = lerp(velocity_y, 0.0f, FIXED_TIMESTEP * friction_y);*/
		velocity_x += acceleration_x * elapsed;
		velocity_y += acceleration_y * elapsed;
		xPos += velocity_x * elapsed;
		yPos += velocity_y * elapsed;

		
		/*modelMatrix.identity();
		modelMatrix.Translate(xPos, yPos, 0);
		program->setModelMatrix(modelMatrix);
*/
	}
	

	
};

void Entity::DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
	int spriteCountY, int posX, int posY)
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
	program->setModelMatrix(modelMatrix);
	modelMatrix.identity();
	modelMatrix.Translate(xPos, yPos, 0);
	glUseProgram(program->programID);
	glBindTexture(GL_TEXTURE_2D, sheet);
	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);
	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
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
		// draw our arrays
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

	for (int y = 0; y < LEVEL_HEIGHT; y++) {
		for (int x = 0; x < LEVEL_WIDTH; x++) {
			if (levelData[y][x] != 0)
			{
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
	}
	program->setModelMatrix(modelMatrixForWorld);
	program->setViewMatrix(viewMatrix);
	program->setProjectionMatrix(projectionMatrix);

	
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
						if (val == 124 || val == 123 || val == 122 || val == 151 || val == 152 || val == 153)
						{
							//cout << val << "solid" << endl;
							newTile->isSolid = true;
							
							newTile->solidint = 1;
							//cout << newTile->isSolid << endl;
						}
						else {
							newTile->solidint = 5;
						}
						//cout << newTile->xPos << endl;
						// be careful, the tiles in this format are indexed from 1 not 0
						levelData[y][x] = val - 1; 
						allOfTheTiles.push_back(newTile);
						
						//os << levelData[y][x] << endl;
						//cout << x << "           " << y << endl;
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
			float placeX = atoi(xPosition.c_str()) / 30 * TILE_SIZE;
			float placeY = atoi(yPosition.c_str()) / 30 * -TILE_SIZE;
			//placeEntity(type, placeX, placeY);
		}
	}
	return true;
}
void read() {
	ifstream infile("world.txt");
	string line;
	if (!infile)
	{
		cout << "OH NOOOOOOOOOOOO!" << endl;
	}
	else
	{
		//cout << "SUCCESS!!!" << endl;
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
		}
	}

}




void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY)
{
	*gridX = (int)(worldX / TILE_SIZE);
	*gridY = (int)(-worldY / TILE_SIZE);
	
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
	program->setModelMatrix(modelMatrixForWorld);
	program->setProjectionMatrix(projectionMatrix);
	program->setViewMatrix(viewMatrix);
	speed = 1;

	SDL_Event event;
	bool done = false;
	read();
	sheet = LoadTexture("spritesheet.png");
	float lastFrameTicks = 0.0f;
	float ticks;
	float elapsed;




	//===============================================================================================================================================
	Entity alien(placeX, placeY, sheet, TILE_SIZE/2, .25, .25, .25, -.25, 0, 0.0f);
	TileMap tileMap(-alien.xPos, -alien.yPos);
	//===============================================================================================================================================



	//to position the alien at the beginning of the level
	alien.setPlayer(5.4f, -6.6f);
	alien.xLimitLeft = alien.xPos;


	int * x = nullptr;
	int *y= nullptr;
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
		render();
		alien.DrawSpriteSheetSprite(program, 21, 30, 30, placeX, placeY);














		
		
		if (keys[SDL_SCANCODE_RIGHT])
		{
			
			
			//alien.movePlayer(elapsed);
			alien.setPlayer(speed, 0);
			
		}	
		if (keys[SDL_SCANCODE_LEFT] && alien.xPos > alien.xLimitLeft)
		{
			//alien.movePlayer(elapsed);
			alien.setPlayer(-speed, 0);

		}
		if (keys[SDL_SCANCODE_UP])
		{
			//alien.movePlayer(elapsed);

			
			alien.setPlayer(0, speed);

		}
		if (keys[SDL_SCANCODE_DOWN])
		{

			ofstream os;
			os.open("solids.txt");
			os << "-------------------------------------------------" << endl;
			for (int i = 0; i < allOfTheTiles.size(); i++) {
				//os << allOfTheTiles[i]->solidint << endl;
				//os << allOfTheTiles[i]->value << endl;
			}
			os << "----------------------------------------------" << endl;
			for (int i = 0; i < allOfTheTiles.size(); i++) {
				//cout << allOfTheTiles[i]->yPos << endl;
				if (-alien.yPos + TILE_SIZE/2 <= allOfTheTiles[i]->yPos + TILE_SIZE/2 && -alien.yPos + TILE_SIZE/2 >= allOfTheTiles[i]->yPos - TILE_SIZE/2)
				{
					//cout << "collide y" << endl;
					//cout << allOfTheTiles[i]->xPos + TILE_SIZE / 2 << " v " <<  allOfTheTiles[i]->xPos - TILE_SIZE / 2 << endl;
					if (alien.xPos <= allOfTheTiles[i]->xPos + TILE_SIZE / 2 && alien.xPos >= allOfTheTiles[i]->xPos - TILE_SIZE / 2)
					{
						os << "collide x" << endl;
						os << allOfTheTiles[i]->solidint << endl;
						os << allOfTheTiles[i]->value << endl;
						if (allOfTheTiles[i]->solidint == 1)
							cout << "is solid" << endl;
					}

				}
			}
			//alien.movePlayer(elapsed);
			alien.setPlayer(0, -speed);
			//cout << alien.yPos << endl;
			//cout << alien.xPos << endl;
		}
		


		viewMatrix.identity();
		viewMatrix.Translate(-alien.xPos, -alien.yPos, 0);
	
		SDL_GL_SwapWindow(displayWindow);

	}
	//os.close();
	delete program;
	delete levelData;
	clearTheHeap(allOfTheTiles);
//#ifdef _WIN32
//	std::cin.get();
//#endif


	SDL_Quit();
	return 0;
}
