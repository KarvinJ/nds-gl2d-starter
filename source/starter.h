#pragma once

#include <nds.h>
#include <gl2d.h>

typedef struct
{
	float x;
	float y;
	float w;
	float h;
	unsigned int color;
} Rectangle;

void drawRectangle(Rectangle &rectangle);

bool hasCollision(Rectangle &bounds, Rectangle &ball);

void initSubSprites();
