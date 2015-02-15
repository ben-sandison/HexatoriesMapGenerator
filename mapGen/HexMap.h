#include <vector>
#include <string>
#include <GL\glew.h>
#include <GLFW/glfw3.h>

#ifndef __HEXMAP_H
#define __HEXMAP_H

class HexMap {

	GLuint vaoMap, vboMap, progMap;
	std::vector<GLint> tileVerts;
	/*
		Default vertex array. These are adjusted for each hex based on its grid position.
	*/
	static const GLint defTileVerts[];

	/*
		The colour values for each tile. RGB.
	*/
	static const GLint cols[];


public:

	void updateVAO();
	void changeTileCol(int, int);
	int getCol(int);

	bool initMap();

	void setupVAO(std::vector<GLint>, std::vector<GLushort>);

	int getAllTiles(std::vector<GLint> &, std::vector<GLushort> &);

	/*
	Just draws the map & grid currently.
	*/
	void drawMap();

	/*
	Calculate vertex position. Also includes the colour of each vertex.
	Usage:
	HexTile.initTile(xPos, yPos, col, vertsArray);
	It appends the tiles vertices to the end of the passed array.
	*/
	static void calcTileVerts(int, int, int, std::vector<GLint> &);

	/*
	Returns the string containing map data to interpret. Really needs reworking.
	Usage: string map = mapFromFile("BeachMap");
	*/
	void colourToFile();

	/*
	Converts mouse point to x and y of tile in the map.
	Return true if in the grid.
	*/
	static bool pointToTile(double, double, int &, int &);
};
#endif