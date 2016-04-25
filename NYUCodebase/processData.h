#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>







class processData
{
public:
	bool readHeader(std::ifstream &stream);
	void render();
	bool readLayerData(std::ifstream &stream);
	bool readEntityData(std::ifstream &stream);
	void read();
	void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY);
};
