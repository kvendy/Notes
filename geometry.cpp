#include "geometry.h"

Position::Position(int x, int y, int width, int height)
{
	if (y < BORDER)
		vertical = Vertical::top;
	else if (y > height - BORDER)
		vertical = Vertical::bottom;
	else
		vertical = Vertical::none;

	if (x < CORNER)
		horizontal = Horizontal::left;
	else if (x > width - CORNER)
		horizontal = Horizontal::right;
	else
		horizontal = Horizontal::none;
}

Qt::CursorShape Position::toCursorShape() const
{
	Qt::CursorShape shape;

	if ((horizontal == Horizontal::left && vertical == Vertical::top) ||
	    (horizontal == Horizontal::right && vertical == Vertical::bottom))
		shape = Qt::SizeFDiagCursor;
	else if ((horizontal == Horizontal::left && vertical == Vertical::bottom) ||
	         (horizontal == Horizontal::right && vertical == Vertical::top))
		shape = Qt::SizeBDiagCursor;
	else if (horizontal == Horizontal::left || horizontal == Horizontal::right)
		shape = Qt::SizeHorCursor;
	else if (vertical == Vertical::top || vertical == Vertical::bottom)
		shape = Qt::SizeVerCursor;
	else
		shape = shape = Qt::ArrowCursor;

	return shape;
}
