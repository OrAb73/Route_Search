
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "glut.h"
#include <queue>
#include <vector>
#include "Cell.h"
#include <iostream>

using namespace std;        // Complexity of BFS vs. DFS
							// B - branching factor
							// d - depth 
const int WIDTH = 600;		// BFS - O(B^d)
const int HEIGHT = 600;     // DFS is non-deterministic
							// Complexity time vs. place
							// Place complexity of BFS - O(B^d)	
const int MSZ = 100;		// Place complexity of DFS - O(B*d)	- much better


const int SPACE = 0;
const int WALL = 1;
const int START = 2;
const int TARGET = 3;
const int PATH = 4;
const int GRAY = 5;
const int BLACK = 6;

const int UP = 0;
const int DOWN = 1;
const int LEFT = 2;
const int RIGHT = 3;
const int IS_FREE = -1;

int maze[MSZ][MSZ] = { 0 };

bool runBFS = false;
bool runDFS = false;

queue<Cell*> grays;
vector <Cell*> dfs_grays;


void SetupMaze()
{
	int i, j;
	// outer margins
	for (i = 0; i < MSZ; i++)
	{
		maze[0][i] = WALL; // first line
		maze[MSZ - 1][i] = WALL; // last line
		maze[i][0] = WALL; // left column
		maze[i][MSZ - 1] = WALL; // right column
	}

	// innter space
	for (i = 1; i < MSZ-1; i++)
		for (j = 1; j < MSZ-1; j++)
		{
			if (i % 2 == 1) // mostly spaces
			{
				if (rand() % 10 > 1) // 80% 
					maze[i][j] = SPACE;
				else
					maze[i][j] = WALL;
			}
			else // mostly walls
			{
				if (rand() % 10 >= 3) // 70% 
					maze[i][j] = WALL;
				else
					maze[i][j] = SPACE;
			}
		}

	maze[MSZ / 2][MSZ / 2] = START;
	maze[rand() % MSZ][rand() % MSZ] = TARGET;



}


void init()
{
	glClearColor(0.5,0.5,0.5,0);// color of window background
	glOrtho(0, MSZ, 0, MSZ, -1, 1); // set the coordinates system

	srand(time(0));

	SetupMaze();
}

void ShowMaze()
{
	int i, j;
	for (i = 0; i < MSZ; i++)
		for (j = 0; j < MSZ; j++)
		{
			//1.set color of cell
			switch (maze[i][j])
			{
				case SPACE: 
					glColor3d(1, 1, 1); // white 
					break;
				case WALL:
					glColor3d(0, 0, 0); // black
					break;
				case START:
					glColor3d(0.6, 0.6, 1); // blue
					break;
				case TARGET:
					glColor3d(1, 0, 0); // red
					break;
				case PATH:
					glColor3d(1, 0, 1); // magenta
					break;
				case GRAY:
					glColor3d(0, 1, 0); // green
					break;
				case BLACK:
					glColor3d(0.6, 0.6, 0.6); // gray
					break;
			}
			//2.show cell
			glBegin(GL_POLYGON);
			glVertex2d(j, i);
			glVertex2d(j, i + 1);
			glVertex2d(j + 1, i + 1);
			glVertex2d(j + 1, i);
			glEnd();
		}
}

void RestorePath(Cell* pc)
{
	Cell* p;
	while (pc != nullptr)
	{
		maze[pc->getRow()][pc->getCol()] = PATH;
		pc = pc->getParent();
	}
}

bool CheckNeighbor(int row, int col, Cell* pCurrent, bool isBFS)
{
	if (maze[row][col] == TARGET)
	{
		if(isBFS)
			runBFS = false;
		else
			runDFS = false;

		cout << "The solution has been found.\n";
		RestorePath(pCurrent);
		return false;
	}
	else // maze[row][col] must be SPACE (WHITE)
	{
		Cell* pc = new Cell(row, col, pCurrent);
		maze[row][col] = GRAY;
		if (isBFS)
			grays.push(pc);
		else
			dfs_grays.push_back(pc);
		return true;
		
	}
	
}

void RunBFSIteration()
{
	Cell* pCurrent;
	int row, col;
	bool go_on = true;

	if (grays.empty())
	{
		runBFS = false;
		cout << "There is no solution. Grays is empty\n";
		return;
	}
	else // grays is not empty
	{
		pCurrent = grays.front();
		grays.pop(); // extract the first element from grays
		// 1. paint pCurrent black
		row = pCurrent->getRow();
		col = pCurrent->getCol();
		if (maze[row][col] != START)
			maze[row][col] = BLACK;
		// 2. check all the neighbors of pCurrent
		// go up
		if (maze[row + 1][col] == SPACE || maze[row + 1][col] == TARGET)
			go_on = CheckNeighbor(row + 1, col, pCurrent, true);
		// down
		if (go_on && (maze[row - 1][col] == SPACE || maze[row - 1][col] == TARGET))
			go_on = CheckNeighbor(row - 1, col, pCurrent, true);
		// left
		if (go_on && (maze[row][col - 1] == SPACE || maze[row][col - 1] == TARGET))
			go_on = CheckNeighbor(row, col - 1, pCurrent, true);
		// right
		if (go_on && (maze[row][col + 1] == SPACE || maze[row][col + 1] == TARGET))
			go_on = CheckNeighbor(row, col + 1, pCurrent, true);
		
	}
	

}

void RunDFSIteration()
{
	Cell* pCurrent;
	int row, col;
	bool go_on = true;

	if (dfs_grays.empty())
	{
		runDFS = false;
		cout << "There is no solution. Grays is empty\n";
		return;
	}

	else
	{
		pCurrent = dfs_grays.back();
		dfs_grays.pop_back();
		// 1. paint pCurrent black
		row = pCurrent->getRow();
		col = pCurrent->getCol();
		if (maze[row][col] != START)
			maze[row][col] = BLACK;
		// 2. Add all white or target neighbors to dfs_grays
		// What is the order of neighbors? Is the order matter?
		// Let's make it random
		int directions[4] = { IS_FREE, IS_FREE, IS_FREE, IS_FREE };
		int index;
		for (int dir = 0; dir < 4; dir++)
		{
			do {
				index = rand() % 4;
			} while (directions[index] != IS_FREE);
			directions[index] = dir;
		}
		// now let's check random directions
		for (int dir = 0; dir < 4 && go_on; dir++)
		{
			switch (directions[dir])
			{
			case UP:
				// try to go up
				if (maze[row + 1][col] == SPACE || maze[row + 1][col] == TARGET)
					go_on = CheckNeighbor(row + 1, col, pCurrent, false);
				break;
			case DOWN:
				if (maze[row - 1][col] == SPACE || maze[row - 1][col] == TARGET)
					go_on = CheckNeighbor(row - 1, col, pCurrent, false);
				break;
			case LEFT:
				if (maze[row][col - 1] == SPACE || maze[row][col - 1] == TARGET)
					go_on = CheckNeighbor(row, col - 1, pCurrent, false);
				break;
			case RIGHT:
				if (maze[row][col + 1] == SPACE || maze[row][col + 1] == TARGET)
					go_on = CheckNeighbor(row, col + 1, pCurrent, false);
				break;
			}
		}

		// DFS before random directions:
		//// try to go up
		//if (maze[row + 1][col] == SPACE || maze[row + 1][col] == TARGET)
		//	go_on = CheckNeighbor(row + 1, col, pCurrent, false);
		//// try to go down
		//if (go_on && (maze[row - 1][col] == SPACE || maze[row - 1][col] == TARGET))
		//	go_on = CheckNeighbor(row - 1, col, pCurrent, false);
		//// try to go left
		//if (go_on && (maze[row][col - 1] == SPACE || maze[row][col - 1] == TARGET))
		//	go_on = CheckNeighbor(row, col - 1, pCurrent, false);
		//// try to go right
		//if (go_on && (maze[row][col + 1] == SPACE || maze[row][col + 1] == TARGET))
		//	go_on = CheckNeighbor(row, col + 1, pCurrent, false);


	}

}
void display()
{
	glClear(GL_COLOR_BUFFER_BIT); // clean frame buffer

	// put here your drawings
	ShowMaze();
	
	
	glutSwapBuffers(); // show all
}

void idle() 

{
	if (runBFS)
		RunBFSIteration();
	if (runDFS)
		RunDFSIteration();

	glutPostRedisplay(); // indirectly call to display
}

void menu(int choice)
{
	Cell* pc = new Cell(MSZ / 2, MSZ / 2, nullptr);

	switch (choice)
	{
	case 1: // BFS
		grays.push(pc);
		runBFS = true;
		break;
	case 2: // DFS
		dfs_grays.push_back(pc);
		runDFS = true;
		break;
	
	}
}


void main(int argc, char* argv[]) 
{
	glutInit(&argc, argv);
	// definitions for visual memory(frame buffer) and double buffer
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInitWindowPosition(600, 20);
	glutCreateWindow("BFS - DFS");

	// display is a refresh function
	glutDisplayFunc(display);
	// idle is an upadate function
	glutIdleFunc(idle);
	// add menu
	glutCreateMenu(menu);
	glutAddMenuEntry("BFS", 1);
	glutAddMenuEntry("DFS", 2);
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	init();

	glutMainLoop();
}