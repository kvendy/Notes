#include <QtGui>
#include <QtWidgets>
#include <time.h>
#include <math.h>
#include "notes.h"

/*
 Короче обнаруженные проблемы:
 1) Закрываю заметку по Альт-Ф4, а потом ВНИЗАПНЕ крэш. Суть в том что не пересчитываются инстанции.
	Возможные решения:
	- перегрузить closeEvent,
	- избавиться от счетчика инстанций и сделать итератор. Предпочтительно.
 2) Окна накладываются, если:
	а. сделать три окна,
	б. закрыть среднее
	в. создавать окна от первого
 3) Ресайз за угол, взаимодействие с несколькими окнами:
	- и вообще окно перемещается при достижении минимума сверху и слева, позор
 4) Не работает хоткей закрытия окна вообще и хоткей вывода в топ после первого использования
 5) Часто теряется фокус по закытию одного из окон; пропадает фокус при создании нвых окон хоткеем.
	Нужно следить за фокусом вручную короче.
------------------------------------------------------------------------------------------------------
 6) Ни в одной реализации липких окон нет проверки по внутренним границам. Убрать.
*/

QVector<Notes*> allMyNotes;

void resetNoteInstances();

Notes::Notes() : QWidget(0),
	cmdNew("+"), cmdTop("^"), cmdClose("x"),
	mousePressedX(0), mousePressedY(0),
    isPressed(false),
    color1(QColor::fromHsl(rand() % 359, 64 + rand() % 64, 128 + rand() % 128)),
    color2(color1.darker(110)),
    onTop(false)
{
	init();
	setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
	resize(180, 165);
}

Notes::Notes(QColor inColor1, QColor inColor2, QPoint inPlace, QSize inSize, QString inText, bool inTop) : QWidget(0),
	cmdNew("+"), cmdTop("^"), cmdClose("x"),
	mousePressedX(0), mousePressedY(0),
    isPressed(false),
	color1(inColor1), color2(inColor2),
	txtl(inText),
	onTop(inTop)
{
	init();

	if (onTop)
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
	else
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

	resize(inSize);
	move(inPlace);
}

void Notes::init()
{
	pmnu = new QMenu("&Menu", this);

	colors.insert("&Blue",   QColor::fromRgb(217, 243, 251));
	colors.insert("&Green",  QColor::fromRgb(210, 255, 204));
	colors.insert("&Pink",   QColor::fromRgb(246, 211, 246));
	colors.insert("P&urple", QColor::fromRgb(222, 218, 254));
	colors.insert("&White",  QColor::fromRgb(255, 255, 255));
	colors.insert("&Yellow", QColor::fromRgb(254, 254, 204));

	for (auto it = colors.cbegin(); it != colors.cend(); ++it)
		pmnu->addAction(it.key());

	connect(pmnu, SIGNAL(triggered(QAction*)), this, SLOT(setColorByAction(QAction*)));

	connect(&cmdNew, SIGNAL(clicked()), SLOT(newForm()));
	connect(&cmdTop, SIGNAL(clicked()), SLOT(topForm()));
	connect(&cmdClose, SIGNAL(clicked()), SLOT(closeForm()));

	cmdNew.setFixedSize (24, 24);
	cmdNew.setFlat(true);
	cmdNew.setCursor(Qt::ArrowCursor);
	cmdNew.setShortcut(Qt::CTRL + Qt::Key_N);
	cmdTop.setFixedSize (24, 24);
	cmdTop.setFlat(true);
	cmdTop.setCursor(Qt::ArrowCursor);
	cmdTop.setShortcut(Qt::CTRL + Qt::Key_T);
	cmdClose.setFixedSize (24, 24);
	cmdClose.setFlat(true);
	cmdClose.setCursor(Qt::ArrowCursor);
	cmdClose.setShortcut(Qt::CTRL + Qt::Key_X);
	txtl.setFrameShape(QFrame::NoFrame);

	//setShortcut()

	//расположение элементов

	QGridLayout* pbxLayout = new QGridLayout(this);
	pbxLayout->setMargin(2);
	pbxLayout->setSpacing(1);
	pbxLayout->addWidget(&cmdNew, 0, 0);
	pbxLayout->addWidget(&cmdTop, 0, 1);
	pbxLayout->addWidget(&cmdClose, 0, 3);
	pbxLayout->addWidget(&txtl, 1, 0, 1, 4);
	setLayout(pbxLayout);

	//порядок табуляции

	setTabOrder(&txtl, &cmdNew);
	setTabOrder(&cmdNew, &cmdTop);
	setTabOrder(&cmdTop, &cmdClose);

	setAttribute(Qt::WA_DeleteOnClose);
	setMouseTracking(true);

	//прозрачный фон области редактирования текста

	QPalette palette;
	QBrush brush(QColor(0, 0, 0, 0));
	brush.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Base, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
	txtl.setPalette(palette);

	QFont font;
	//font.setFamily(QString::fromUtf8("Flow"));
	font.setFamily(QString::fromUtf8("Segoe Script"));
	font.setPointSize(14);
	//font.setBold(1);
	setFont(font);
}

void Notes::mousePressEvent(QMouseEvent* pe)
{
	if(pe->button() == Qt::LeftButton)
		isPressed = true;

	mousePressedX = pe->x();
	mousePressedY = pe->y();

	position = Position(pe->x(), pe->y(), width(), height());

	getOSWindows();
}

void Notes::mouseReleaseEvent(QMouseEvent* pe)
{
	isPressed = false;

	position.clear();

	//qDebug() << otherWindows;
	//qDebug() << x() << y() << width() << height();
}

void Notes::mouseMoveEvent(QMouseEvent* pe)
{
	if (!isPressed)
		setCursor(Position(pe->x(), pe->y(), width(), height()).toCursorShape());

	if (isPressed)
	{
		int newX = x(), newY = y(), newWidth = width(), newHeight = height();

		//считаем новый Rect
		if (position.left())
		{
			newWidth = newWidth - pe->globalX() + newX;
			newX = pe->globalX();
		}
		else if (position.right())
			newWidth = pe->globalX() - newX;

		if (position.top())
		{
			newHeight = newHeight - pe->globalY() + newY;
			newY = pe->globalY();
		}
		else if (position.bottom())
			newHeight = pe->globalY() - newY;

		if (!position.top() && !position.bottom() && !position.left() && !position.right())
		{
			newX = pe->globalX() - mousePressedX;
			newY = pe->globalY() - mousePressedY;
		}

		snap(newX, newY, newWidth, newHeight);

		setGeometry(newX, newY, newWidth, newHeight);
	}
}

void Notes::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	QLinearGradient gradient (0, 0, width(), height());
	gradient.setColorAt(0, color1);
	gradient.setColorAt(1, color2);
	painter.setBrush(gradient);
	painter.setPen(QPen(color1, 1, Qt::SolidLine));
	painter.drawRect(rect());

	//В ХР тут чертешто
	//QStyleOptionSizeGrip opt;
	//opt.init(this);
	//opt.corner = Qt::BottomRightCorner;
	//style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
}

void Notes::contextMenuEvent ( QContextMenuEvent * event )
{
	pmnu->popup(event->globalPos());
}

void Notes::newForm()
{
	Notes* note = new Notes();
	note->show();
	allMyNotes.append(note);
	note->getPos(pos(), width());
	resetNoteInstances();
}

void Notes::topForm()
{
	int widthChangeTop = width();
	int heightChangeTop = height();
	hide();
	if (onTop)
	{
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
		onTop = false;
		cmdTop.setText("^");
	}
	else
	{
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
		onTop = true;
		cmdTop.setText("v");
	}
	resize(widthChangeTop, heightChangeTop);
	show();
}

void Notes::setColorByAction(QAction * act)
{
	color1 = colors.find(act->text()).value();
	color2 = color1.darker(110);
	update();
}

void Notes::closeForm()
{
	allMyNotes.remove(instance);
	resetNoteInstances();
	close();
}

void Notes::getPos (QPoint inPos, int inWidth)
{
	int a = inPos.x() + inWidth + 10;
	int b = inPos.y();

	foreach (Notes *note, allMyNotes)
	{
		if (note->pos() == QPoint(a, b))
			a += note->width() + 10;

		//if (abs(note->pos().y() - b) < 20)
			//if (abs(note->pos().x() - a) < 20)
				//a += note->width() + 10;

		if (a + width() > qApp->desktop()->availableGeometry(this).right())
		{
			a -= 4 * width() - 20;
			b += 20;
		}
		if (b + height() > qApp->desktop()->availableGeometry(this).height())
		{
			a += 20;
			b -= height() - 20;
		}
	}

	move (a, b);
}

#ifdef Q_OS_WIN32
BOOL Notes::StaticEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	Notes *pThis = reinterpret_cast<Notes*>(lParam);
	return pThis->EnumWindowsProc(hwnd);
}

BOOL Notes::EnumWindowsProc(HWND hwnd)
{
	int x = 0, y = 0, width = 0, height = 0;
	RECT rect;
	WCHAR title[255];
	if(::GetWindowRect(hwnd, &rect) && ::IsWindowVisible(hwnd))
	{
		x = rect.left;
		y = rect.top;
		width = rect.right - rect.left;
		height = rect.bottom - rect.top;
		if (width != 0 && height != 0 && (x != this->x() || y != this->y() || width != this->width() || height != this->height()))
		{
			if(GetWindowText(hwnd, title, 255))
				otherWindowsNames.append(QString::fromWCharArray(title));
			else
				otherWindowsNames.append("");

			otherWindows.append(QRect(x, y, width, height));
		}
	}

	return TRUE;
}
#elif defined(Q_OS_LINUX)
void Notes::enumerateWindows(Display *display, Window rootWindow)
{
	Status status;
	Window root;
	Window parent;
	Window *children;
	unsigned int childrenCount;
	XWindowAttributes win_attr;

	if(XQueryTree(display, rootWindow, &root, &parent, &children, &childrenCount))
	{
		for(unsigned int i = 0; i < childrenCount; ++i)
		{
			Window window = children[i];

			char* name = '\0';
			status = XFetchName(display, window, &name);
			if (status >= Success)
				otherWindowsNames.append(name);

			/* query the window's attributes. */
			status = XGetWindowAttributes(display, window, &win_attr);
			if (status >= Success)
			{
				int screen_x, screen_y;

				XTranslateCoordinates(display, root, root,
									  win_attr.x, win_attr.y, &screen_x, &screen_y,
									  &window);

				otherWindows.append(QRect(screen_x, screen_y, win_attr.width, win_attr.height));
			}

			XFree(name);

			enumerateWindows(display, children[i]);
		}
		XFree(children);
	}
}
#endif

void Notes::getOSWindows()
{
	otherWindows.clear();
	otherWindowsNames.clear();
	horLines.clear();
	vertLines.clear();

#ifdef Q_OS_WIN32
	::EnumWindows(StaticEnumWindowsProc, reinterpret_cast<LPARAM>(this));
#elif defined(Q_OS_LINUX)
	Display *display = XOpenDisplay(":0.0");
	Window rootWindow = XDefaultRootWindow(display);
	enumerateWindows(display, rootWindow);
#endif

	foreach (QRect rect, otherWindows)
	{
		horLines.insert(rect.y(),                  {rect.x(), rect.x() + rect.width()});
		horLines.insert(rect.y()  + rect.height(), {rect.x(), rect.x() + rect.width()});

		vertLines.insert(rect.x() + rect.width(),  {rect.y(), rect.y() + rect.height()});
		vertLines.insert(rect.x(),                 {rect.y(), rect.y() + rect.height()});
	}

	//for (int i = 0; i < otherWindowsNames.size() && i < otherWindows.size(); i++)
	//	qDebug() << otherWindowsNames.at(i) << otherWindows.at(i);
}

void Notes::snap(int &x, int &y, int &width, int &height)
{
	int optimalX = x, optimalY = y, optimalHeight = height, optimalWidth = width,
	    minimalDistance = SNAP + 1;

	if (!position.right() && !position.left())
	for (auto it = horLines.lowerBound(y - SNAP); it.key() < horLines.upperBound(y + height + SNAP).key(); ++it)
	{
		if ((it.value().first < x + width + SNAP) &&
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

	if (!position.top() && !position.bottom())
	for (auto it = vertLines.lowerBound(x - SNAP); it.key() < vertLines.upperBound(x + width + SNAP).key(); ++it)
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

void resetNoteInstances()
{
	int i = 0;
	foreach(Notes *note, allMyNotes)
	{
		note->instance = i;
		i++;
	}
}

Medium::Medium(QObject* pobj) : QObject(pobj)
{
	notesHide = 0;

	pmnu = new QMenu("&Menu");
	pmnu->addAction("&New", this, SLOT(slotNewNote()));
	pmnu->addAction("&Hide", this, SLOT(slotHideNotes()))->setCheckable(1);;
	pmnu->addSeparator();
	pmnu->addAction("&Exit", qApp, SLOT(quit()));

	trayIcon = new QSystemTrayIcon;
	QIcon icon4tray;
	QSize ssize;
	ssize.setWidth(16);
	ssize.setHeight(16);
	icon4tray.addFile(":/images/notes.png", ssize, QIcon::Normal, QIcon::Off);
	trayIcon->setIcon(icon4tray);
	trayIcon->setContextMenu(pmnu);
	trayIcon->show();

	connect(qApp, SIGNAL(aboutToQuit()), SLOT(slotSaveNotes()));
	connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slotTrayAct(QSystemTrayIcon::ActivationReason)));
}

void Medium::slotSaveNotes()
{
	int notes = 0;
	QList<QColor> outColor1;
	QList<QColor> outColor2;
	QList<QSize> outSize;
	QList<QPoint> outPos;
	QList<QString> outText;
	QList<bool> outTop;

	foreach(Notes *note, allMyNotes)
	{
		if (note->txtl.toPlainText() != "")
		{
			outColor1<<note->color1;
			outColor2<<note->color2;
			outSize<<note->size();
			outPos<<note->pos();
			outTop<<note->onTop;
			outText<<note->txtl.toPlainText();
			notes++;
		}
	}

	QFile file("notes.dat");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);
	out<<notes<<outColor1<<outColor2<<outSize<<outPos<<outTop<<outText;
	file.close();
	trayIcon->hide();
}

void Medium::slotHideNotes()
{
	if (notesHide == 0)
		foreach(Notes *note, allMyNotes)
			note->hide();
	else
		foreach(Notes *note, allMyNotes)
			note->show();
	notesHide = !notesHide;
}

void Medium::slotNewNote()
{
	Notes* note = new Notes();
	note->getPos(QPoint(rand()%100 + 100, rand()%100 + 100), 0);
	note->show();
	allMyNotes.append(note);
	resetNoteInstances();
}

void Medium::slotTrayAct(QSystemTrayIcon::ActivationReason reason)
{
	foreach(Notes *note, allMyNotes)
		note->activateWindow();
	/*
	тут прописано разное поведение для разных событий - ненужно пока

	switch (reason)
	{
	case QSystemTrayIcon::Trigger:
		slotHideNotes();
	case QSystemTrayIcon::DoubleClick:
		foreach(Notes *note, allMyNotes)
			note->setFocus();
	case QSystemTrayIcon::MiddleClick:
		;
		break;
	default:
		;
	}*/
}

void readNotes()
{
	int notes = 0;
	QFile file("notes.dat");
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);

		QList<QColor> inColor1;
		QList<QColor> inColor2;
		QList<QSize> inSize;
		QList<QPoint> inPos;
		QList<QString> inText;
		QList<bool> inTop;

		in>>notes>>inColor1>>inColor2>>inSize>>inPos>>inTop>>inText;

		file.close();

		for (int i = 0; i < notes; i++)
		{
			Notes* note = new Notes(inColor1[i], inColor2[i], inPos[i], inSize[i], inText[i], inTop[i]);
			note->show();
			allMyNotes.append(note);
			note->activateWindow();
		}
	}
	if (notes == 0)
	{
		Notes* note = new Notes();
		note->show();
		allMyNotes.append(note);
		note->activateWindow();
	}

	resetNoteInstances();
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	QApplication app(argc, argv);

	QFontDatabase::addApplicationFont (":/fonts/Flow.otf");
	QFontDatabase::addApplicationFont (":/fonts/Flow_B.otf");

	Medium medium;
	readNotes();

	//QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF8"));
	//QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF8"));

	return app.exec();
}
