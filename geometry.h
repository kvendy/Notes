#ifndef GEOMETRY_H
#define GEOMETRY_H

#include <qnamespace.h>
#include <QRect>
#include <QMultiMap>

const int DEFAULT_WIDTH  = 180;
const int DEFAULT_HEIGHT = 165;
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
	    horizontal_  (Horizontal::none),
	    vertical_    (Vertical::none)
	{
	}

	Position(Horizontal horizontal, Vertical vertical) :
		horizontal_  (horizontal),
		vertical_    (vertical)
	{
	}

	Position(int x, int y, int width, int height);

	Qt::CursorShape toCursorShape() const;

	void clear()
	{
		horizontal_ = Horizontal::none;
		vertical_   = Vertical::none;
	}

	bool left() const
	{
		return horizontal_ == Horizontal::left;
	}
	bool right() const
	{
		return horizontal_ == Horizontal::right;
	}
	bool top() const
	{
		return vertical_ == Vertical::top;
	}
	bool bottom() const
	{
		return vertical_ == Vertical::bottom;
	}
	bool corner() const
	{
		return vertical_ != Vertical::none && horizontal_ != Horizontal::none;
	}
	bool vertical() const
	{
		return vertical_ != Vertical::none && horizontal_ == Horizontal::none;
	}
	bool horizontal() const
	{
		return vertical_ == Vertical::none && horizontal_ != Horizontal::none;
	}
	bool moving() const
	{
		return vertical_ == Vertical::none && horizontal_ == Horizontal::none;
	}

private:
	Horizontal horizontal_;
	Vertical vertical_;
};

class SnapManager
{
public:
	void snap(QRect& rect, const Position& position);
	void snap(int &x, int &y, int &width, int &height, const Position& position);
	void clear();
	void addRect(const QRect& rect);
	void addRect(int x, int y, int width, int height);
	bool overlapCheck(const QRect& rect);
private:
	bool checkLine(int a, int b1, int b2, const Position &position, bool exact);
	Line horizontal, vertical;
};

#endif // GEOMETRY_H
