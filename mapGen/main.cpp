#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Log.h"
#include "Shaders.h"
#include "HexMap.h"

// (C) Group 16
using namespace std;

#pragma region vars
struct territory {
	vector<int> tilesInTerr;
	int owner = -1;
};
vector<territory> ters;
vector<int>availableTiles;

int prevTile = -1, stage = 0, colour = -1, currentTer = -1;
bool endCreate = false;
HexMap map;

#pragma region messages
string messages[3] = {
	"Welcome to the map creator.\n\nThere are two stages to map creation:\n1. Colouring\n2. Territories\n\n"
	"These will need to be done in the order stated and will be explained to you as \nyou reach them. "
	"Please bear in mind that you cannot return to a section once it is completed, so ensure you get it right!\n\n"
	"If you can, please ensure you can see this console window and the game screen atthe same time.\n\n"
	"When you are ready select the game window and press space to continue.",
	"1. Colouring\n\n"
	"Colouring is the process of changing the colours on the map. This is purely \ncosmetic.\n\n"
	"To change the colour of a tile there are two steps:\n\n1. Press a numeric key (1-3) to select a colour.\n"
	"2. Click on the tile you wish to change.\n\n\nTips:\n\nYou only need to select a colour if you are changing from the previous colour.\n\n"
	"You can use the arrow keys to select tiles around the one you previously selected.\n\n\n"
	"Colours:\n\n1 - Sand    2 - Water    3 - Land\n\nPress space once you are finished.\n",
	"2. Territories\n\n"
	"Territories are what make up the areas on the map. You will now be creating them\n\n"
	"To create a territory:\n\n1. Select the owner of the territory (1 - neutral  2 - you  3 - AI)\n"
	"2. Select the tiles that make up your territory, you will notice they become \nwhite.\n"
	"3. Press space to move on to the next territory.\n\n\n Tips:\n\nYou cannot go back to a territory once you have moved on.\n"
	"You can deselect a tile by clicking it again (assuming you haven't moved on)\nTiles in other territories will become black when you move on.\n"
	"You can change the territores owner at any point during tile selection (but not after you move on).\n\n"
	"Once all tiles have been put into territories press space. The creator will \nclose and your map can be found in maps\\map.mp\n\n"
};
#pragma endregion
#pragma endregion

void clearConsole() {
	for (int i = 0; i < 10; i++)
		cout << "\n\n\n\n\n\n\n\n\n\n";
}

int moveOn() {

	if (stage == 1) map.colourToFile();

	if (stage < 2) {
		clearConsole();
		stage++;
		cout << messages[stage];
	} 

	if (stage == 2) {
		territory ter;
		ters.push_back(ter);
		if (currentTer >= 0) {
			for (unsigned int i = 0; i < ters[currentTer].tilesInTerr.size(); i++) {
				map.changeTileCol(ters[currentTer].tilesInTerr[i], 4);
			}
		}
		currentTer++;
	}

	return 1;
}

void handleStage(int tile) {

	if (stage != 0) prevTile = tile;

	if (stage == 1) {
		if (colour == -1) {
			cout << "Please select a colour: \n1 - Sand  2 - Water  3 - Land\n";
			return;
		}
		map.changeTileCol(tile, colour);
	}

	if (stage == 2) {
		if (ters[currentTer].owner == -1) {
			cout << "Select an owner by pressing 1, 2 or 3.\n";
			return;
		}
		auto pos = find(ters[currentTer].tilesInTerr.begin(), ters[currentTer].tilesInTerr.end(), tile);

		if (pos != ters[currentTer].tilesInTerr.end()) {

			availableTiles.push_back(tile);
			ters[currentTer].tilesInTerr.erase(pos);
			map.changeTileCol(tile, 0);

		} else {

			pos = find(availableTiles.begin(), availableTiles.end(), tile);

			if (pos != availableTiles.end()) {

				availableTiles.erase(pos);
				ters[currentTer].tilesInTerr.push_back(tile);
				map.changeTileCol(tile, 3);
			}
		}
	}
}

void tersToFile() {

	string filePath = "Maps\\map.mp";
	ofstream map(filePath, ofstream::out | ofstream::app);

	if (!map.is_open()) {
		log("Failed to load file: %s\n", filePath.c_str());
		return;
	}
	map << ters.size() << ",";
	for (unsigned int i = 0; i < ters.size(); i++) {

		map << ters[i].owner << "," << ters[i].tilesInTerr.size() << ",";

		for (unsigned int j = 0; j < ters[i].tilesInTerr.size(); j++) {

			map << ters[i].tilesInTerr[j] << ",";

		}
	}
	endCreate = true;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {

	double xValue = NULL;
	double yValue = NULL;

	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		
		glfwGetCursorPos(window, &xValue, &yValue);
		
		int gridX, gridY;

		if (map.pointToTile(xValue, yValue, gridX, gridY)) {
			int tile = gridX * 33 + gridY;
			handleStage(tile);
		}
	}
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {

	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		if (availableTiles.size() != 0) {
			moveOn();
		} else {
 			tersToFile();
		}
	}

	int movement = 0;

	if (key == GLFW_KEY_DOWN && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		movement = 1;
	}

	if (key == GLFW_KEY_UP && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		movement = -1;    
	}

	if (key == GLFW_KEY_LEFT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		movement = -33;
	}

	if (key == GLFW_KEY_RIGHT && (action == GLFW_REPEAT || action == GLFW_PRESS)) {
		movement = 33;
	}

	if (movement != 0) {
		if ((prevTile + movement) < 40 * 33 &&
			(prevTile + movement) >= 0) {
			handleStage(prevTile + movement);
		} 
	}

	if (stage == 1) {
		if (key == GLFW_KEY_1 && action == GLFW_PRESS)
			colour = 0;
		if (key == GLFW_KEY_2 && action == GLFW_PRESS)
			colour = 1;
		if (key == GLFW_KEY_3 && action == GLFW_PRESS)
			colour = 2;
	}

	if (stage == 2) {
		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			ters[currentTer].owner = 0;
			cout << "Territory owner: neutral\n";
		}
		if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			ters[currentTer].owner = 2;
			cout << "Territory owner: you\n";
		}
		if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
				ters[currentTer].owner = 1;
				cout << "Territory owner: AI\n";
		}
	}
}

void _update_fps_counter(GLFWwindow* window) {

	double xValue = NULL;
	double yValue = NULL;
	glfwGetCursorPos(window, &xValue, &yValue);
	
	static double previous_seconds = glfwGetTime();
	static int frame_count;
	double current_seconds = glfwGetTime();
	double elapsed_seconds = current_seconds - previous_seconds;

	if (elapsed_seconds > 0.25) {

		previous_seconds = current_seconds;
		double fps = (double)frame_count / elapsed_seconds;
		char tmp[128];

		sprintf_s(tmp, "CONSOLE FOR INSTRUCTIONS @ fps: %.2f", fps);
		glfwSetWindowTitle(window, tmp);
		frame_count = 0;
	}

	frame_count++;

}

GLFWwindow* initWindow(void) {

	//glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	GLFWwindow* window = NULL;

	const GLFWvidmode * mode = glfwGetVideoMode(glfwGetPrimaryMonitor());	// Get screen resolution
	int window_width = mode->width;
	int window_height = mode->height;

	if (window_width >= 1024 || window_height >= 768) {		// Ensures resolution is big enough and if necessary makes window fullscreen

		if (window_width == 1024 || window_height == 768) {
			window = glfwCreateWindow(726, 738, "Hexatories", glfwGetPrimaryMonitor(), NULL);
		} else {
			window = glfwCreateWindow(726, 738, "Hexatories", NULL, NULL);
		}
	}

	return window;
}

int main(void) {
#pragma region stuff
	restart_log();

	// Initialize the library 
	if (!glfwInit())
		return -1;

	GLFWwindow* window = initWindow();

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glDepthFunc(GL_NEVER);	//Renders in order drawn (last drawn on top)

	// Make the window's context current
	glfwMakeContextCurrent(window);

	glewExperimental = true;

	if (glewInit() != GLEW_OK) {
		glfwTerminate();
		log("GLEW failed to initialise\n");
		return -1;
	}

	if (!map.initMap()) {
		glfwTerminate();
		return -1;
	}

	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, key_callback);
#pragma endregion

	cout << messages[stage];

	for (int i = 0; i < 1320; i++) {
		availableTiles.push_back(i);
	}

	// Loop until the user closes the window
	while (!glfwWindowShouldClose(window) &! endCreate)
	{

		_update_fps_counter(window);

		map.drawMap();
		
		// Swap front and back buffers 
		glfwSwapBuffers(window);

		// Poll for and process events 
		glfwPollEvents();
	}

	glfwTerminate();

	return 0;
}