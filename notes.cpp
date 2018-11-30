#include <QtGui>
#include <QtWidgets>
#include <QSysInfo>
#include <time.h>
#include <math.h>
#include "notes.h"

/*
 Problems:
 1) Hotkeys do not work:
	a. close note - never
	b. put window to the top - after the firs use
 2) Notes lose focus often after closing or creating one with hotkey.
 Todo:
 1) Filter some other windows
*/

QSet<Notes*> allMyNotes;

QWidget *globalParent = nullptr;

QColor randomColor()
{
	return QColor::fromHsl(rand() % 359, 64 + rand() % 64, 128 + rand() % 96);
}

QRect centerPosition(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT)
{
	QPoint screenCenter = QGuiApplication::primaryScreen()->availableGeometry().center();

	return QRect(screenCenter.x() - width / 2,
	             screenCenter.x() - height / 2,
	             width, height);
}

QRect randomPosition(int width = DEFAULT_WIDTH, int height = DEFAULT_HEIGHT)
{
	QSize screenSize = QGuiApplication::primaryScreen()->availableGeometry().size();

	return QRect(rand()%(screenSize.width()  - width  * 3) + width,
	             rand()%(screenSize.height() - height * 3) + height,
	             width, height);
}

NotesData::NotesData() :
           text(""),
           color(randomColor()),
           place(randomPosition()),
           onTop(false)
{
}

NotesData::NotesData(QString inText, QColor inColor, QRect inPlace, bool inTop) :
           text(inText),
           color(inColor),
           place(inPlace),
           onTop(inTop)
{
}

QDataStream &operator >>(QDataStream &stream, NotesData &note)
{
	stream>>note.text;
	stream>>note.color;
	stream>>note.place;
	stream>>note.onTop;
	return stream;
}

QDataStream &operator <<(QDataStream &stream, const NotesData &note)
{
	stream<<note.text;
	stream<<note.color;
	stream<<note.place;
	stream<<note.onTop;
	return stream;
}

QDataStream &operator <<(QDataStream &stream, NotesData *note)
{
	stream<<*note;
	return stream;
}

Notes::Notes() :
    QWidget(globalParent, Qt::Window),
    NotesData(),
	cmdNew("+"), cmdTop("^"), cmdClose("x"),
	mousePressedX(0), mousePressedY(0),
    isPressed(false)
{
	init();
}

Notes::Notes(NotesData nData) :
    QWidget(globalParent, Qt::Window),
    NotesData(nData),
    cmdNew("+"), cmdTop("^"), cmdClose("x"),
    mousePressedX(0), mousePressedY(0),
    isPressed(false)
{
	init();
}

Notes::Notes(QString inText, QColor inColor, QRect inPlace, bool inTop) :
    QWidget(globalParent, Qt::Window),
    NotesData(inText, inColor, inPlace, inTop),
	cmdNew("+"), cmdTop("^"), cmdClose("x"),
	mousePressedX(0), mousePressedY(0),
    isPressed(false)
{
	init();
}

Notes::~Notes()
{
}

bool Notes::isEmpty()
{
	return ctrlTxt.toPlainText().isEmpty();
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
	connect(&cmdClose, SIGNAL(clicked()), SLOT(close()));

	connect(&ctrlTxt, SIGNAL(textChanged()), SLOT(updateText()));

	QShortcut* shortcutB = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_B), &ctrlTxt);
	connect(shortcutB, SIGNAL(activated()), this, SLOT(makeBold()));

	QShortcut* shortcutI = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_I), &ctrlTxt);
	connect(shortcutI, SIGNAL(activated()), this, SLOT(makeItalic()));

	QShortcut* shortcutS = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_S), &ctrlTxt);
	connect(shortcutS, SIGNAL(activated()), this, SLOT(makeStrike()));

	QShortcut* shortcutU = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_U), &ctrlTxt);
	connect(shortcutU, SIGNAL(activated()), this, SLOT(makeUnderline()));

	QShortcut* shortcutP = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_P), &ctrlTxt);
	connect(shortcutP, SIGNAL(activated()), this, SLOT(makePlain()));

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
	ctrlTxt.setFrameShape(QFrame::NoFrame);

	//setShortcut()

	//placing controls

	QGridLayout* pbxLayout = new QGridLayout(this);
	pbxLayout->setMargin(2);
	pbxLayout->setSpacing(1);
	pbxLayout->addWidget(&cmdNew, 0, 0);
	pbxLayout->addWidget(&cmdTop, 0, 1);
	pbxLayout->addWidget(&cmdClose, 0, 3);
	pbxLayout->addWidget(&ctrlTxt, 1, 0, 1, 4);
	setLayout(pbxLayout);

	//tab order

	setTabOrder(&ctrlTxt, &cmdNew);
	setTabOrder(&cmdNew, &cmdTop);
	setTabOrder(&cmdTop, &cmdClose);

	setAttribute(Qt::WA_DeleteOnClose);
	setMouseTracking(true);

	//transparent background for tex edit control

	QPalette palette;
	QBrush brush(QColor(0, 0, 0, 0));
	brush.setStyle(Qt::SolidPattern);
	palette.setBrush(QPalette::Active, QPalette::Base, brush);
	palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
	ctrlTxt.setPalette(palette);

	QFont font;
	font.setFamily("Bad Script");
	//font.setFamily("Segoe Script");
	font.setPointSize(14);
	//font.setBold(1);
	ctrlTxt.setFont(font);

	ctrlTxt.setText(text);

	if (onTop)
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
	else
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

	adjustMyRect();

	setGeometry(place);

#ifdef Q_OS_WIN32
	win10 = QSysInfo::WindowsVersion >= QSysInfo::WV_WINDOWS10;
	if (win10)
	{
		xborder = ::GetSystemMetrics(SM_CXPADDEDBORDER) + ::GetSystemMetrics(SM_CXFRAME);
		yborder = ::GetSystemMetrics(SM_CXPADDEDBORDER) + ::GetSystemMetrics(SM_CYFRAME);
	}
	else
	{
		xborder = 0;
		yborder = 0;
	}
#endif
}

void Notes::adjustMyRect()
{
	sm.clear();
	getMyWindows();

	while (sm.overlapCheck(place))
	{
		place.translate(place.width() + SNAP * 2, 0);

		if (place.x() + place.width() > qApp->desktop()->availableGeometry(this).right())
			place.translate(SNAP * 2 - 4 * place.width(), place.height() / 2);
		if (place.y() + place.height() > qApp->desktop()->availableGeometry(this).height())
			place = randomPosition(place.width(), place.height());
	}
}

void Notes::mousePressEvent(QMouseEvent* pe)
{
	if(pe->button() == Qt::LeftButton)
		isPressed = true;

	mousePressedX = pe->x();
	mousePressedY = pe->y();

	position = Position(pe->x(), pe->y(), width(), height());
	sm.clear();
	getOSWindows();
	//getMyWindows();
}

void Notes::mouseReleaseEvent(QMouseEvent* pe)
{
	isPressed = false;
	position.clear();
	place = geometry();
}

void Notes::mouseMoveEvent(QMouseEvent* pe)
{
	if (!isPressed)
		setCursor(Position(pe->x(), pe->y(), width(), height()).toCursorShape());

	if (isPressed)
	{
		int newX = x(), newY = y(), newWidth = width(), newHeight = height();

		//calculating new rect
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

		sm.snap(newX, newY, newWidth, newHeight, position);

		if (newWidth < MINIMAL_WIDTH)
			return;

		if (newHeight < MINIMAL_HEIGHT)
			return;

		setGeometry(newX, newY, newWidth, newHeight);
	}
}

void Notes::keyPressEvent(QKeyEvent * event)
{
	if (event->modifiers() == Qt::ControlModifier)
	{
		if (event->key() == Qt::Key_Tab)
		{
			auto it = allMyNotes.find(this);
			it++;
			if (it == allMyNotes.end())
				it = allMyNotes.begin();

			(*it)->activateWindow();
		}
	}
}

void Notes::paintEvent(QPaintEvent *)
{
	QPainter painter(this);
	QLinearGradient gradient (0, 0, width(), height());
	gradient.setColorAt(0, color);
	gradient.setColorAt(1, color.darker(110));
	painter.setBrush(gradient);
	painter.setPen(QPen(color, 1, Qt::SolidLine));
	painter.drawRect(rect());

	//Goes horribly wrong on winXP
	//QStyleOptionSizeGrip opt;
	//opt.init(this);
	//opt.corner = Qt::BottomRightCorner;
	//style()->drawControl(QStyle::CE_SizeGrip, &opt, &painter, this);
}

void Notes::contextMenuEvent ( QContextMenuEvent * event )
{
	pmnu->popup(event->globalPos());
}

void Notes::closeEvent(QCloseEvent *event)
{
	auto it = allMyNotes.find(this);
	allMyNotes.erase(it);
}

void Notes::newForm()
{
	Notes* note = new Notes("", randomColor(), geometry(), false);
	note->show();
	allMyNotes.insert(note);
}

void Notes::topForm()
{
	//int widthChangeTop = width();
	//int heightChangeTop = height();
	hide();
	if (onTop)
	{
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
		onTop = false;
		cmdTop.setText("^");
	}
	else
	{
		setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
		onTop = true;
		cmdTop.setText("v");
	}
	//resize(widthChangeTop, heightChangeTop);
	show();
}

void Notes::makeBold()
{
	if (ctrlTxt.textCursor().hasSelection())
	{
		QTextCharFormat format;
		int newFormat,
		    oldFormat = ctrlTxt.textCursor().charFormat().fontWeight();
		if (oldFormat < QFont::Bold)
			newFormat = QFont::Bold;
		else
			newFormat = QFont::Normal;
		format.setFontWeight(newFormat);
		ctrlTxt.textCursor().mergeCharFormat(format);
	}
}

void Notes::makeItalic()
{
	if (ctrlTxt.textCursor().hasSelection())
	{
		QTextCharFormat format;
		format.setFontItalic(!ctrlTxt.textCursor().charFormat().fontItalic());
		ctrlTxt.textCursor().mergeCharFormat(format);
	}
}

void Notes::makePlain()
{
	if (ctrlTxt.textCursor().hasSelection())
	{
		ctrlTxt.textCursor().setCharFormat(QTextCharFormat());
		ctrlTxt.textCursor().setBlockFormat(QTextBlockFormat());
	}
}

void Notes::makeUnderline()
{
	if (ctrlTxt.textCursor().hasSelection())
	{
		QTextCharFormat format;
		format.setFontUnderline(!ctrlTxt.textCursor().charFormat().fontUnderline());
		ctrlTxt.textCursor().mergeCharFormat(format);
	}
}

void Notes::makeStrike()
{
	if (ctrlTxt.textCursor().hasSelection())
	{
		QTextCharFormat format;
		format.setFontStrikeOut(!ctrlTxt.textCursor().charFormat().fontStrikeOut());
		ctrlTxt.textCursor().mergeCharFormat(format);
	}
}

void Notes::updateText()
{
	text = ctrlTxt.toHtml();
}

void Notes::setColorByAction(QAction * act)
{
	color = colors.find(act->text()).value();
	update();
}

#ifdef Q_OS_WIN32
BOOL Notes::StaticEnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	Notes *pThis = reinterpret_cast<Notes*>(lParam);
	return pThis->EnumWindowsProc(hwnd);
}

BOOL Notes::EnumWindowsProc(HWND hwnd)
{
	RECT rect;
	WCHAR title[255];
	if(::GetWindowRect(hwnd, &rect) && ::IsWindowVisible(hwnd))
	{
		int x      = rect.left,
		    y      = rect.top,
		    width  = rect.right  - rect.left,
		    height = rect.bottom - rect.top;

		if (width != 0 && height != 0 &&
		    (x != this->x() || y != this->y() || width != this->width() || height != this->height()))
		{
			if (win10)
			{
				LONG lStyle = ::GetWindowLong(hwnd, GWL_STYLE);

				if (lStyle & WS_THICKFRAME)
				{
					x += xborder;
					width -= xborder * 2;
					height -= yborder;
				}
			}

			if(::GetWindowText(hwnd, title, 255))
				otherWindowsNames.append(QString::fromWCharArray(title));
			else
				otherWindowsNames.append("");

			//otherWindows.append(QRect(x, y, width, height));
			sm.addRect(x, y, width, height);
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

				if (win_attr.width != 0 && win_attr.height != 0 &&
					(screen_x != this->x() || screen_y != this->y() ||
					 win_attr.width != this->width() || win_attr.height != this->height()))
					//otherWindows.append(QRect(screen_x, screen_y, win_attr.width, win_attr.height));
					sm.addRect(screen_x, screen_y, win_attr.width, win_attr.height);
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

#ifdef Q_OS_WIN32
	::EnumWindows(StaticEnumWindowsProc, reinterpret_cast<LPARAM>(this));
#elif defined(Q_OS_LINUX)
	Display *display = XOpenDisplay(":0.0");
	Window rootWindow = XDefaultRootWindow(display);
	enumerateWindows(display, rootWindow);
#endif

	//for (int i = 0; i < otherWindowsNames.size() && i < otherWindows.size(); i++)
	//	qDebug() << otherWindowsNames.at(i) << otherWindows.at(i);
}

void Notes::getMyWindows()
{
	foreach(Notes *note, allMyNotes)
		if (note != this)
			sm.addRect(note->geometry());

	for (int i = 0; i < qApp->desktop()->numScreens(); i++)
		sm.addRect(qApp->desktop()->availableGeometry(i));
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

Medium::~Medium()
{
}

void Medium::slotSaveNotes()
{
	QFile file("notes.dat");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	auto it = allMyNotes.begin();
	while (it != allMyNotes.end())
		if ((*it)->isEmpty())
			it = allMyNotes.erase(it);
		else
			++it;

	out<<allMyNotes;

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
	note->show();
	allMyNotes.insert(note);
}

void Medium::slotTrayAct(QSystemTrayIcon::ActivationReason reason)
{
	foreach(Notes *note, allMyNotes)
		note->activateWindow();
	/*

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
	QFile file("notes.dat");
	if (file.open(QIODevice::ReadOnly))
	{
		QDataStream in(&file);

		QList<NotesData> notesToOpen;
		in >> notesToOpen;

		foreach (NotesData nData, notesToOpen)
		{
			Notes* note = new Notes(nData);
			note->show();
			allMyNotes.insert(note);
			note->activateWindow();
		}
		file.close();
	}

	if (allMyNotes.isEmpty())
	{
		Notes* note = new Notes();
		note->show();
		allMyNotes.insert(note);
		note->activateWindow();
	}
}

int main(int argc, char** argv)
{
	srand(time(NULL));
	QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(false);

	QFontDatabase::addApplicationFont (":/fonts/BadScript.ttf");

	QWidget window;
	globalParent = &window;

	Medium medium;
	readNotes();

	return app.exec();
}
