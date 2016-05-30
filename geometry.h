#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <qnamespace.h>
#include <QRect>
#include <QMultiMap>

const int MINIMAL_WIDTH  = 100;
const int MINIMAL_HEIGHT = 100;
const int BORDER = 3;
const int CORNER = 10;
const int SNAP = 10;

typedef QMultiMap<int, QPair<int, int> > Line;

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

	Qt::CursorShape toCursorShape() const;

	void clear()
	{
		horizontal = Horizontal::none;
		vertical   = Vertical::none;
	}

	bool left() const
	{
		return horizontal == Horizontal::left;
	}
	bool right() const
	{
		return horizontal == Horizontal::right;
	}
	bool top() const
	{
		return vertical == Vertical::top;
	}
	bool bottom() const
	{
		return vertical == Vertical::bottom;
	}

private:
	Horizontal horizontal;
	Vertical vertical;
};

class SnapManager
{
public:
	void snap(QRect& rect, const Position& position);
	void snap(int &x, int &y, int &width, int &height, const Position& position);
	void clear();
	void addRect(const QRect& rect);
	void addRect(int x, int y, int width, int height);
private:
	Line horizontal, vertical;
};



#endif // GEOMETRY_H
