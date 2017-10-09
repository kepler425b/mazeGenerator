#include <SDL.h>
#include <string> 
#include <math.h>
#include <stdio.h>
#include <Windows.h>
#include <time.h>

#define STACK_SIZE 50

SDL_Event Event;
SDL_Renderer *Renderer = NULL;
SDL_Window *Window = NULL;
SDL_Surface *Screen = NULL;
int SCW = 1000;
int SCH = 1000;
int cellWidth = 20;
int widthInTiles = SCW / cellWidth;

typedef struct cell{
	int col, row;
	bool visited;
	bool walls[4];
	SDL_Rect cellRect;
} cell;

typedef cell stackElement;

typedef struct {
	stackElement *content;
	int top;
	int max_size;
} stackT;

bool stackIsEmpty(stackT *stackIn)
{
	return stackIn->top < 0;
}

bool stackIsFull(stackT *stackIn)
{
	return stackIn->top >= stackIn->max_size - 1;
}

void stackInit(stackT *stackIn, int maxSize)
{
	stackElement *newContent;

	newContent = (stackElement *)malloc(sizeof(stackElement) * maxSize);
	if (newContent == NULL)
	{
		printf("Error loading memory.");
		exit(1);
	}
	else
	{
		stackIn->content = newContent;
		stackIn->max_size = maxSize;
		stackIn->top = -1;
	}
}

void stackDestroy(stackT *stackIn)
{
	free(stackIn->content);
	stackIn->content = NULL;
	stackIn->max_size = 0;
	stackIn->top = -1;
}

void push(stackT *stackIn, stackElement element)
{
	if (!stackIsFull(stackIn))
	{
		stackIn->content[++stackIn->top] = element;
	}
	else
	{
		printf("Stack is full.");
		exit(1);
	}
}

stackElement* pop(stackT *stackIn)
{
	if (!stackIsEmpty(stackIn))
	{
		return &stackIn->content[stackIn->top--];
	}
	else
	{
		printf("Stack is empty.");
		exit(1);
	}
}

bool quit = false;

bool init()
{
	SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_BLEND);
	bool success = true;
	if (SDL_Init(SDL_INIT_EVERYTHING))
	{
		printf("Failed to initilize SDL. SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		Window = SDL_CreateWindow("Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCW, SCH, SDL_WINDOW_RESIZABLE);
		if (Window == NULL)
		{
			printf("Failed creating window. SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			Renderer = SDL_CreateRenderer(Window, 0, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
			if (Renderer == NULL)
			{
				printf("Failed creating renderer. SDL Error: %s\n", SDL_GetError());
			}
		}
	}
	return success;
}

void close()
{
	SDL_DestroyWindow(Window);
	SDL_DestroyRenderer(Renderer);
	Window = NULL;
	Renderer = NULL;
	SDL_Quit();
}

void setColor(int r, int g, int b)
{
	SDL_SetRenderDrawColor(Renderer, r, g, b, 255);
}

void renderWalls(int i, int j, cell (&cellArray)[STACK_SIZE][STACK_SIZE])
{
	setColor(24, 48, 16);
	if (cellArray[i][j].walls[0] == true)
	{
		SDL_RenderDrawLine(Renderer, cellArray[i][j].cellRect.x, cellArray[i][j].cellRect.y, cellArray[i][j].cellRect.x + cellArray[i][j].cellRect.w, cellArray[i][j].cellRect.y);	//top
	}
	if (cellArray[i][j].walls[1] == true)
	{
		SDL_RenderDrawLine(Renderer, cellArray[i][j].cellRect.x + cellArray[i][j].cellRect.w, cellArray[i][j].cellRect.y, cellArray[i][j].cellRect.x + cellArray[i][j].cellRect.w, cellArray[i][j].cellRect.y + cellArray[i][j].cellRect.h);	//right
	}
	if (cellArray[i][j].walls[2] == true)
	{
		SDL_RenderDrawLine(Renderer, cellArray[i][j].cellRect.x, cellArray[i][j].cellRect.y + cellArray[i][j].cellRect.h, cellArray[i][j].cellRect.x + cellArray[i][j].cellRect.w, cellArray[i][j].cellRect.y + cellArray[i][j].cellRect.h);	//bottom
	}
	if (cellArray[i][j].walls[3] == true)
	{
		SDL_RenderDrawLine(Renderer, cellArray[i][j].cellRect.x, cellArray[i][j].cellRect.y, cellArray[i][j].cellRect.x, cellArray[i][j].cellRect.y + cellArray[i][j].cellRect.w);	//left
	}
}
stackT currentStack;
cell *nextArray[4];
int returnTop = 0;
int returnRight = 0;
int returnBottom = 0;
int returnLeft = 0;
int nextID = 0;
cell* returnNext(cell *cellArray, cell (cellArray2)[][STACK_SIZE])
{
	int top    = 0;
	int right  = 0;
	int bottom = 0;
	int left   = 0;
	int neighbourSum = 0;
	nextArray[0] = NULL;
	nextArray[1] = NULL;
	nextArray[2] = NULL;
	nextArray[3] = NULL;

	if (cellArray->col > 0 && cellArray->col < widthInTiles && cellArray2[cellArray->col-1][cellArray->row].visited == false)
	{
		top = cellArray->col - 1;
		nextArray[neighbourSum] = &cellArray2[top][cellArray->row];
		neighbourSum++;
		returnTop++;
	}
	if (cellArray->row > -1 && cellArray->row < widthInTiles-1 && cellArray2[cellArray->col][cellArray->row+1].visited == false)
	{
		right = cellArray->row + 1;
		nextArray[neighbourSum] = &cellArray2[cellArray->col][right];
		neighbourSum++;
		returnRight++;
	}
	if (cellArray->col > -1 && cellArray->col < widthInTiles-1 && cellArray2[cellArray->col+1][cellArray->row].visited == false)
	{
		bottom = cellArray->col + 1;
		nextArray[neighbourSum] = &cellArray2[bottom][cellArray->row];
		neighbourSum++;
		returnBottom++;
	}
	if (cellArray->row > 0 && cellArray->row < widthInTiles && cellArray2[cellArray->col][cellArray->row-1].visited == false)
	{
		left    = cellArray->row - 1;
		nextArray[neighbourSum] = &cellArray2[cellArray->col][left];
		neighbourSum++;
		returnLeft++;
	}
	if (neighbourSum == 0)
	{
		return NULL;
	}
	else
	{
		nextID = (rand() % neighbourSum);

		if (cellArray->col - nextArray[nextID]->col == -1) //bottom case
		{
			cellArray->walls[2] = false;
			nextArray[nextID]->walls[0] = false;
		}
		if (cellArray->col - nextArray[nextID]->col == 1) //top case
		{
			cellArray->walls[0] = false;
			nextArray[nextID]->walls[2] = false;
		}
		if (cellArray->row - nextArray[nextID]->row == -1) //right case
		{
			cellArray->walls[1] = false;
			nextArray[nextID]->walls[3] = false;
		}
		if (cellArray->row - nextArray[nextID]->row == 1) //left case
		{
			cellArray->walls[3] = false;
			nextArray[nextID]->walls[1] = false;
		}
		return nextArray[nextID];
	}
}
	
int main(int argc, char* argv[])
{
	if (!init())
	{
		printf("Failed to initialize!\n");
	}
	else
	{

	}
	srand(time(NULL));
	stackInit(&currentStack, widthInTiles*cellWidth);
	cell cellArray[STACK_SIZE][STACK_SIZE] = { NULL };
	cell *current = &cellArray[0][0];
	cell *next = NULL;
	cell *last[STACK_SIZE*STACK_SIZE] = { NULL };
	bool traversing = false;
	for (int i = 0; i < widthInTiles; i++)
	{
		for (int j = 0; j < widthInTiles; j++)
		{
			cellArray[i][j].cellRect.x = j * cellWidth;
			cellArray[i][j].cellRect.y = i * cellWidth;
			cellArray[i][j].cellRect.w = cellWidth;
			cellArray[i][j].cellRect.h = cellWidth;
			cellArray[i][j].col = i;
			cellArray[i][j].row = j;
			cellArray[i][j].walls[0] = true;
			cellArray[i][j].walls[1] = true;
			cellArray[i][j].walls[2] = true;
			cellArray[i][j].walls[3] = true;
		}
	}
	while (!quit)
	{
		while (SDL_PollEvent(&Event))
		{
			if (Event.type == SDL_QUIT)
			{
				close();
				quit = true;
			}
		}
		traversing = false;
		current->visited = true;
		next = returnNext(current, cellArray);
		if (next != NULL && next->visited == false)
		{
			push(&currentStack, *current);
			current = next;	
		}
		else if (currentStack.top > 0)
		{
			traversing = true;
			current = pop(&currentStack);
		}
		printf("returned top: %d, right: %d, bottom: %d, left: %d times. currentStack size: %d, nextID: %d.\r", 
			returnTop, returnRight, returnBottom, returnLeft, currentStack.top, nextID);

		SDL_RenderClear(Renderer);
		for (int i = 0; i < widthInTiles; i++)
		{
			for (int j = 0; j < widthInTiles; j++)
			{
				setColor(96, 160, 16);
				SDL_RenderFillRect(Renderer, &cellArray[i][j].cellRect);
				if (cellArray[i][j].visited)
				{
					setColor(48, 96, 16);
					SDL_RenderFillRect(Renderer, &cellArray[i][j].cellRect);
				}
				setColor(92, 16, 224);
				SDL_RenderFillRect(Renderer, &current->cellRect);
				setColor(50, 50, 50);
				renderWalls(i, j, cellArray);
			}
		}
		SDL_RenderPresent(Renderer);
		if (!traversing)
		{
			SDL_Delay(100);
		}
		else
		{
			SDL_Delay(5);
		}
	}
	return(0);
}
