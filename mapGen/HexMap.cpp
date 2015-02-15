#include <fstream>
#include "HexMap.h"
#include "Log.h"
#include "Shaders.h"

using namespace std;

#pragma region defaultVertsCols

const GLint HexMap::defTileVerts[] = {
	0, 727,	//left
	6, 716,	//bottom left
	18, 716,	//bottom right
	24, 727,	//right
	18, 738,	//top right
	6, 738,	//top left
};

const GLint HexMap::cols[] = {
	182, 149, 62,	// sand
	62, 97, 182,	// water
	64, 168, 66,	// grass
	255, 255, 255,	// white
	0, 0, 0,		// black
};
#pragma endregion

void HexMap::updateVAO() {
	glBindVertexArray(vaoMap);
	glBindBuffer(GL_ARRAY_BUFFER, vboMap);
	glBufferData(GL_ARRAY_BUFFER, tileVerts.size() * sizeof(GLint), tileVerts.data(), GL_STATIC_DRAW);
	glBindVertexArray(0);
}

void HexMap::changeTileCol(int tile, int newCol) {
	int pos = 2 + tile * 6 * 5;

	for (int j = 0; j < 6; j++) {
		for (int i = 0; i < 3; i++) {
			tileVerts[pos + 5 * j + i] = cols[(newCol * 3) + i];
		}
	}
	updateVAO();
}

int HexMap::getCol(int tile) {

	int tileCols[3]; 
	int pos = 2 + 5 * 6 * tile;
	int i;

	for (i = 0; i < 3; i++)
		tileCols[i] = tileVerts[pos + i];
	
	for (i = 0; i < sizeof(cols); i += 3) {
		if (cols[i] == tileCols[0] && cols[i + 1] == tileCols[1] && cols[i + 2] == tileCols[2])
			break;
	}
	return i / 3;
}

bool HexMap::initMap() {

	vector<GLushort> indices;

	int mapPos = getAllTiles(tileVerts, indices);

	setupVAO(tileVerts, indices);

	return true;
}

void HexMap::setupVAO(vector<GLint> verts, vector<GLushort> indices) {

	/*
	Vertex shader, fragment shader
	*/
	GLuint vs, fs, eboMap;

	/*
	Sets up all the vertex attribute stuff. In order of code blocks:

	Define and bind vao.
	Define and bind vbo. (Vertex information)
	Define and bind ebo. (Draw order)
	Load texture.
	Load program.
	Define position variable for shader.
	Define colour variable for shader.
	Define texture variable for shader.
	Unbind vao.
	*/
	glGenVertexArrays(1, &vaoMap);
	glBindVertexArray(vaoMap);

	glGenBuffers(1, &vboMap);
	glBindBuffer(GL_ARRAY_BUFFER, vboMap);
	glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLint), verts.data(), GL_STATIC_DRAW);

	glGenBuffers(1, &eboMap);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboMap);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLushort), indices.data(), GL_STATIC_DRAW);

	vs = compileVShader("map_vs");
	fs = compileFShader("map_fs");
	progMap = createProgram(vs, fs);

	GLint posAttrib = glGetAttribLocation(progMap, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_INT, GL_FALSE, 5 * sizeof(GLint), 0);

	GLint colAttrib = glGetAttribLocation(progMap, "colour");
	glEnableVertexAttribArray(colAttrib);
	glVertexAttribPointer(colAttrib, 3, GL_INT, GL_FALSE, 5 * sizeof(GLint), (void*)(2 * sizeof(GLint)));

	glBindVertexArray(0);
}

int HexMap::getAllTiles(vector<GLint> &verts, vector<GLushort> &indices) {
#pragma region vars
	/*
	currTile - the current tile not in columns/rows. For instance the 3rd tile in the 2nd column would be (1 * 33 + 3). 41 being the column height.
	mapPos - how far through the map data we are (in terms of array elements)
	colCount - to count how many tiles to colour each time.
	colType - the current colour.
	baseIndices - the draw order for the hex tile vertices. Needs to be specified for each tile, hence "base".
	*/
	int currTile = 0;
	int mapPos = 2;

	GLushort baseIndices[] = { 0, 1, 5, 1, 2, 5, 2, 4, 5, 2, 3, 4 };

#pragma endregion

#pragma region initTilesTerrs
	/*
	Loops through each tile generating its vertex information and initialising them.
	*/
	for (int x = 0; x < 40; x++) {
		for (int y = 0; y < 33; y++) {

			HexMap::calcTileVerts(x, y, 2, verts);

			for (int i = 0; i < 12; i++) {	// Set up the vertex draw order for the current tile.
				indices.push_back(baseIndices[i] + 6 * currTile);
			}

			currTile++;
		}
	}
	return mapPos;
}

void HexMap::drawMap() {

	glBindVertexArray(vaoMap);

	glUseProgram(progMap);

	glDrawElements(GL_TRIANGLES, 15840, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
}

void HexMap::calcTileVerts(int x, int y, int col, vector<GLint> &vboArray) {

	/*
	xoff - either 1 or 0 depending on your x value. Needed as every other column is lower down (look at the map if confused).
	colPos - the start position in the types array of your selected colour. e.g. if type was 1 (water) colPos would be 3.
	*/
	int xoff = x % 2;
	int colPos = col * 3;

	/*
	Each vertex has 7 floats which define it. x, y, r, g, b, texX, texY.
	Goes through each of the vertices (of which there are 6, 6 * 7 = 42) defining their floats.

	i will always be a multiple of 7, for the 7 floats per vertex.

	note: i / 3.5 will increase by 2 each loop, and is used for the x/y positions from the verts array.
	Cast is needed just for the compiler to not throw a fit.
	*/
	for (int i = 0; i < 30; i += 5) {

		vboArray.push_back(defTileVerts[(int)(i / 2.5)] + (18 * x));	// Calculate the x pos of the current tile and store it.
		vboArray.push_back(defTileVerts[(int)(i / 2.5) + 1] - (22 * y + 11 * xoff));	// Calculate the y pos and store it.

		for (int j = colPos; j < colPos + 3; j++) {		// Loops through the colour array, appending your selected colour to the end of each vertex.
			vboArray.push_back(cols[j]);
		}
	}
}

void HexMap::colourToFile() {
	/*
	Sets up your filepath for you, because I'm nice like that.
	*/
	string filePath = "Maps\\map.mp";


	/*
	Opens your file for writing Look up fstream if confused. Basically allows for simpler file handling.
	*/
	ofstream map(filePath);

	if (!map.is_open()) {
		log("Failed to load file: %s\n", filePath.c_str());
		return;
	}

	map << "map";

	int prevCol = getCol(0);
	int col, colCount = 1;
	int maxTiles = 33 * 40;

	for (int i = 1; i < maxTiles; i++) {
		col = getCol(i);
		
		if (col != prevCol) {
			map << colCount << "," << prevCol << ",";
			colCount = 1;
		} else {
			colCount++;
		}
		prevCol = col;
	}
	map << colCount << "," << prevCol << ",";
}

bool HexMap::pointToTile(double mouseX, double mouseY, int &gridX, int &gridY) {

	/*
	Find rectangle within which point lies. Each rect has sections of 3 different tiles in.
	*/
	int rectX = (int)mouseX / 18;
	int rectY = (int)(mouseY - ((rectX % 2) * 11)) / 22;

	/*
	Mouse position relative to the current box
	*/
	mouseX -= 18 * rectX;
	mouseY -= 22 * rectY + ((rectX % 2) * 11);

	/*
	Inequality, test if we are in the main tile of this rectangle or the two smaller sections.
	If we are, grid co ords are rect co ords, otherwise x is -1 and y is rect +1/0/-1
	*/
	if (mouseX > 12 * abs(0.5 - mouseY / 22)) {
		gridX = rectX;
		gridY = rectY;
	} else {
		gridX = rectX - 1;
		gridY = rectY - (gridX % 2) + ((mouseY > 11) ? 1 : 0);
	}

	/*
	Final bounds test
	*/
	if (gridX < 0 || gridX > 39 || gridY < 0 || gridY > 32) return false;

	return true;
}