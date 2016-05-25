#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <qnamespace.h>

const int MINIMAL_WIDTH = 50;
const int BORDER = 3;
const int CORNER = 10;

enum class Horizontal
{
	left,
	right,
	none
};

enum class Vertical
{
	top,
	bottom,
	none
};

class Position
{
public:
	Position() :
	    horizontal  (Horizontal::none),
	    vertical    (Vertical::none)
	{
	}

	Position(int x, int y, int width, int height);

	Qt::CursorShape toCursorShape();

	void clear()
	{
		horizontal = Horizontal::none;
		vertical   = Vertical::none;
	}

	bool left()
	{
		return horizontal == Horizontal::left;
	}
	bool right()
	{
		return horizontal == Horizontal::right;
	}
	bool top()
	{
		return vertical == Vertical::top;
	}
	bool bottom()
	{
		return vertical == Vertical::bottom;
	}

private:
	Horizontal horizontal;
	Vertical vertical;
};



#endif // GEOMETRY_H
