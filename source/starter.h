#pragma once

#include <nds.h>
#include <gl2d.h>

// some useful defines
#define HALF_WIDTH (SCREEN_WIDTH / 2)
#define HALF_HEIGHT (SCREEN_HEIGHT / 2)

typedef struct
{
	float x;
	float y;
	float w;
	float h;
	unsigned int color;
} Rectangle;

const int WHITE = RGB15(255, 255, 255);
const int RED = RGB15(255, 0, 0);
const int GREEN = RGB15(0, 255, 0);
const int BLUE = RGB15(0, 0, 255);

void drawRectangle(Rectangle &rectangle);

bool hasCollision(Rectangle &bounds, Rectangle &ball);

void initSubSprites();
