#include "geometry.h"

Position::Position(int x, int y, int width, int height)
{
	if (y < BORDER)
		vertical_ = Vertical::top;
	else if (y > height - BORDER)
		vertical_ = Vertical::bottom;
	else
		vertical_ = Vertical::none;

	if (x < CORNER)
		horizontal_ = Horizontal::left;
	else if (x > width - CORNER)
		horizontal_ = Horizontal::right;
	else
		horizontal_ = Horizontal::none;
}

Qt::CursorShape Position::toCursorShape() const
{
	Qt::CursorShape shape;

	if ((horizontal_ == Horizontal::left && vertical_ == Vertical::top) ||
	    (horizontal_ == Horizontal::right && vertical_ == Vertical::bottom))
		shape = Qt::SizeFDiagCursor;
	else if ((horizontal_ == Horizontal::left && vertical_ == Vertical::bottom) ||
	         (horizontal_ == Horizontal::right && vertical_ == Vertical::top))
		shape = Qt::SizeBDiagCursor;
	else if (horizontal_ == Horizontal::left || horizontal_ == Horizontal::right)
		shape = Qt::SizeHorCursor;
	else if (vertical_ == Vertical::top || vertical_ == Vertical::bottom)
		shape = Qt::SizeVerCursor;
	else
		shape = Qt::ArrowCursor;

	return shape;
}

void SnapManager::snap(QRect &rect, const Position &position)
{
	int x = rect.x(), y = rect.y(), width = rect.width(), height = rect.height();
	snap(x, y, width, height, position);
	rect.setX(x);
	rect.setY(y);
	rect.setWidth(width);
	rect.setHeight(height);
}

void SnapManager::snap(int &x, int &y, int &width, int &height, const Position &position)
{
	int optimalX = x, optimalY = y, optimalHeight = height, optimalWidth = width,
	    minimalDistance = SNAP + 1;

	if (position.vertical() || position.corner() || position.moving())
	for (auto it = horizontal.lowerBound(y - SNAP); it.key() < horizontal.upperBound(y + height + SNAP).key(); ++it)
	{
		if ((it.value().first  < x + width + SNAP) &&
		    (it.value().second > x - SNAP))
		{
			if ((abs(y - it.key()) < minimalDistance) && !position.bottom())
			{
				minimalDistance = abs(y - it.key());

				optimalY = it.key();
				if (position.top())
					optimalHeight = height + y - it.key();
			}
			if ((abs(y + height - it.key()) < minimalDistance) && !position.top())
			{
				minimalDistance = abs(y + height - it.key());

				if (position.bottom())
					optimalHeight = it.key() - y;
				else
					optimalY = it.key() - height;
			}
		}
	}

	minimalDistance = SNAP + 1;

	if (position.horizontal() || position.corner() || position.moving())
	for (auto it = vertical.lowerBound(x - SNAP); it.key() < vertical.upperBound(x + width + SNAP).key(); ++it)
	{
		if ((it.value().first < y + height + SNAP) &&
		    (it.value().second > y - SNAP))
		{
			if ((abs(x - it.key()) < minimalDistance) && !position.right())
			{
				minimalDistance = abs(x - it.key());
				optimalX = it.key();
				if (position.left())
					optimalWidth = width + x - it.key();
			}
			if ((abs(x + width - it.key()) < minimalDistance) && !position.left())
			{
				minimalDistance = abs(x + width - it.key());

				if (position.right())
					optimalWidth = it.key() - x;
				else
					optimalX = it.key() - width;
			}
		}
	}

	x = optimalX;
	y = optimalY;
	width = optimalWidth;
	height = optimalHeight;
}

void SnapManager::clear()
{
	horizontal.clear();
	vertical.clear();
}

void SnapManager::addRect(const QRect &rect)
{
	addRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void SnapManager::addRect(int x, int y, int width, int height)
{
	horizontal.insert(y,           {x, x + width});
	horizontal.insert(y  + height, {x, x + width});

	vertical.insert(x + width,  {y, y + height});
	vertical.insert(x,          {y, y + height});
}
