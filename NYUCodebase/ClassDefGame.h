//
//#ifdef _WINDOWS
//#define _CRT_SECURE_NO_WARNINGS
//#endif
//#include <stdio.h>
//#ifdef _WINDOWS
//#include <GL/glew.h>
//#endif
//#include <SDL.h>
//#include <SDL_opengl.h>
//#include <SDL_image.h>
//#include "ShaderProgram.h"
//#include "Matrix.h"
//#include <vector>
//#include <string>
//#include <SDL_mixer.h>
//#include <fstream>
//#include <string>
//#include <iostream>
//#include <ostream>
//#include <sstream>
//
//
//
//class Tile
//{
//public:
//	float xPos;
//	float yPos;
//	bool isSolid;
//	int solidint;
//	float width;
//	float height;
//	unsigned int value;
//	Tile(float x, float y, bool solid, float wid, float hei);
//};
//
//
//
//class Entity
//{
//public:
//	float xPos;
//	float yPos;
//	int gridx = 0;
//	int gridy = 0;
//	float xLimitLeft;
//	float xLimitRight;
//	float yLimitUp;
//	int textureID;
//	float width;
//	float height;
//	float velocity_x;
//	float velocity_y;
//	float acceleration_x;
//	float acceleration_y;
//	float friction_x = 1.1;
//	float friction_y = 2.5;
//	bool collidedTop = false;
//	bool collidedBottom = false;
//	bool collidedLeft = false;
//	bool collidedRight = false;
//
//
//	Entity(float x, float y, int texture, float width, float velX, float velY, float accelX, float accelY, float fricX, float fricY, bool collide);
//	void DrawSpriteSheetSprite(ShaderProgram *program, int index, int spriteCountX,
//		int spriteCountY, int posX, int posY);
//	void setPlayer(float x, float y)
//	{
//
//		xPos += x;
//		yPos += y;
//		modelMatrix.identity();
//		modelMatrix.Translate(xPos, yPos, 0);
//		program->setModelMatrix(modelMatrix);
//
//	}
//	void collide() {
//		
//		worldToTileCoordinates(xPos, yPos - TILE_SIZE / 2, &gridx, &gridy);
//		if (isSolid(levelData[gridy][gridx])){
//			collidedBottom = true;
//			velocity_y = 0;
//			acceleration_y = 0;
//		}
//		else {
//			collidedBottom = false;
//		}
//		worldToTileCoordinates(xPos, yPos + TILE_SIZE / 2, &gridx, &gridy);
//		if (isSolid(levelData[gridy][gridx])){
//			collidedTop = true;
//			velocity_y = 0;
//			acceleration_y = 0;
//		}
//		else {
//			collidedTop = false;
//		}
//		worldToTileCoordinates(xPos + TILE_SIZE / 2, yPos, &gridx, &gridy);
//		if (isSolid(levelData[gridy][gridx])){
//			collidedRight = true;
//			velocity_x = 0;
//			acceleration_x = 0;
//
//		}
//		else {
//			collidedRight = false;
//		}
//		worldToTileCoordinates(xPos - TILE_SIZE / 2, yPos, &gridx, &gridy);
//		if (isSolid(levelData[gridy][gridx])){
//			collidedLeft = true;
//			velocity_x = 0;
//			acceleration_x = 0;
//		}
//		else {
//			collidedLeft = false;
//		}
//	}
//	void movePlayer(float elapsed)
//	{
//		if (velocity_x > 2 || velocity_x < -2){
//			acceleration_x = 0;
//		}
//		if (velocity_y > 2 || velocity_y < -2){
//			acceleration_y = 0;
//		}
//
//		velocity_x = lerp(velocity_x, 0.0f, elapsed * friction_x);
//		velocity_y = lerp(velocity_y, 0.0f, elapsed * friction_y);
//
//		velocity_x += acceleration_x * elapsed;
//		velocity_y += acceleration_y * elapsed;
//		xPos += velocity_x * elapsed;
//		yPos -= velocity_y * elapsed;
//
//
//		modelMatrix.identity();
//		modelMatrix.Translate(xPos, yPos, 0);
//		program->setModelMatrix(modelMatrix);
//
//	}
//
//
//
//};