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
		if ((it.value().first - SNAP < x + width) &&
			(it.value().second + SNAP > x))
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
		if ((it.value().first - SNAP < y + height) &&
			(it.value().second + SNAP > y))
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
	windows.clear();
	horizontal.clear();
	vertical.clear();
}

void SnapManager::addRect(const QRect &rect)
{
	addRect(rect.x(), rect.y(), rect.width(), rect.height());
}

void SnapManager::addRect(int x, int y, int width, int height)
{
	Line newHorizontal, newVertical;

	newHorizontal.insert(y,           {x, x + width});
	newHorizontal.insert(y  + height, {x, x + width});

	newVertical.insert(x + width,  {y, y + height});
	newVertical.insert(x,          {y, y + height});

	//check existing windows
	foreach (QRect window, windows)
	{
		Line newHLine;

		for (auto it = newHorizontal.begin(); it != newHorizontal.end(); ++it)
		{
			newHLine.unite(checkLineToInsert(window.top(), window.bottom(), window.left(), window.right(), it.key(), it.value().first, it.value().second));
		}

		newHorizontal = newHLine;

		Line newVLine;
		for (auto it = newVertical.begin(); it != newVertical.end(); ++it)
		{
			newVLine.unite(checkLineToInsert(window.top(), window.bottom(), window.left(), window.right(), it.key(), it.value().first, it.value().second));
		}

		newVertical = newVLine;
	}

	horizontal.unite(newHorizontal);
	vertical.unite(newVertical);

	windows.append(QRect(x, y, width, height));
}

bool SnapManager::overlapCheck(const QRect &rect)
{
	Position hPos = Position(Horizontal::left, Vertical::none);
	Position vPos = Position(Horizontal::none, Vertical::top);

	if (checkLine(rect.x(), rect.y(), rect.y() + rect.height(), vPos, true) &&
		checkLine(rect.y(), rect.x(), rect.x() + rect.width(),  hPos, true) &&
		checkLine(rect.x() + rect.width(),  rect.y(), rect.y() + rect.height(), vPos, true) &&
		checkLine(rect.y() + rect.height(), rect.x(), rect.x() + rect.width(), hPos, true))
		return true;

	return false;
}

Line SnapManager::checkLineToInsert(int rect_top, int rect_bottom, int rect_left, int rect_right, int base_line, int base_left, int base_right)
{
	Line line;

	if (rect_top < base_line && rect_bottom > base_line)
	{
		if(rect_left < base_left)
		{
			if (rect_right < base_left) //full line
				line.insert(base_line, {base_left, base_right});
			else if (rect_right < base_right) //crop
				line.insert(base_line, {rect_right, base_right});
			// else //no line
		}
		else
		{
			if (rect_left > base_right) //full line
				line.insert(base_line, {base_left, base_right});
			else if (rect_right > base_right) //crop
				line.insert(base_line, {base_left, rect_left});
			else //two lines
			{
				line.insert(base_line, {base_left, rect_left});
				line.insert(base_line, {rect_right, base_right});
			}
		}
	}
	else //full line
		line.insert(base_line, {base_left, base_right});

	return line;
}

bool SnapManager::checkLine(int a, int b1, int b2, const Position &position, bool exact)
{
	bool result = false;

	Line *lineSet;

	if (position.horizontal())
		lineSet = &horizontal;
	else if (position.vertical())
		lineSet = &vertical;

	for (auto it = lineSet->lowerBound(a - SNAP); it.key() < lineSet->upperBound(a + SNAP).key(); ++it)
	{
		if ((it.value().first - SNAP < b2) &&
			(it.value().second + SNAP > b1))
		{
			if (exact)
			{
				if ((it.value().first + SNAP > b1) &&
					(it.value().second - SNAP < b2))
					result = true;
			}
			else
				result = true;
		}
	}

	return result;
}
