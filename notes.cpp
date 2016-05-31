#include <QtGui>
#include <QtWidgets>
#include <time.h>
#include <math.h>
#include "notes.h"

/*
 Короче обнаруженные проблемы:
 1) Окна накладываются, если:
	а. сделать три окна,
	б. закрыть среднее
	в. создавать окна от первого
 2) Не работает хоткей закрытия окна вообще и хоткей вывода в топ после первого использования
 3) Часто теряется фокус по закытию одного из окон; пропадает фокус при создании нвых окон хоткеем.
	Нужно следить за фокусом вручную короче.
------------------------------------------------------------------------------------------------------
 4) Ни в одной реализации липких окон нет проверки по внутренним границам. Убрать?
*/

QSet<Notes*> allMyNotes;

NotesData::NotesData() :
           text(""),
           color(QColor::fromHsl(rand() % 359, 64 + rand() % 64, 128 + rand() % 128)),
           place(QRect(300, 300, 180, 165)),
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
    QWidget(0),
    NotesData(),
	cmdNew("+"), cmdTop("^"), cmdClose("x"),
	mousePressedX(0), mousePressedY(0),
    isPressed(false)
{
	init();
}

Notes::Notes(NotesData nData) :
    QWidget(0),
    NotesData(nData),
    cmdNew("+"), cmdTop("^"), cmdClose("x"),
    mousePressedX(0), mousePressedY(0),
    isPressed(false)
{
	init();
}

Notes::Notes(QColor inColor, QRect inPlace, QString inText, bool inTop) :
    QWidget(0),
    NotesData(inText, inColor, inPlace, inTop),
	cmdNew("+"), cmdTop("^"), cmdClose("x"),
	mousePressedX(0), mousePressedY(0),
    isPressed(false)
{
	init();
}

bool Notes::empty()
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

	//расположение элементов

	QGridLayout* pbxLayout = new QGridLayout(this);
	pbxLayout->setMargin(2);
	pbxLayout->setSpacing(1);
	pbxLayout->addWidget(&cmdNew, 0, 0);
	pbxLayout->addWidget(&cmdTop, 0, 1);
	pbxLayout->addWidget(&cmdClose, 0, 3);
	pbxLayout->addWidget(&ctrlTxt, 1, 0, 1, 4);
	setLayout(pbxLayout);

	//порядок табуляции

	setTabOrder(&ctrlTxt, &cmdNew);
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
	ctrlTxt.setPalette(palette);

	QFont font;
	font.setFamily(QString::fromUtf8("Flow"));
	//font.setFamily(QString::fromUtf8("Segoe Script"));
	font.setPointSize(14);
	//font.setBold(1);
	setFont(font);

	ctrlTxt.setText(text);

	if (onTop)
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowStaysOnTopHint);
	else
		setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);

	setGeometry(place);
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

		sm.snap(newX, newY, newWidth, newHeight, position);

		if (newWidth < MINIMAL_WIDTH)
			return;

		if (newHeight < MINIMAL_HEIGHT)
			return;

		setGeometry(newX, newY, newWidth, newHeight);
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

void Notes::closeEvent(QCloseEvent *event)
{
	auto it = allMyNotes.find(this);
	allMyNotes.erase(it);
}

void Notes::newForm()
{
	Notes* note = new Notes();
	note->show();
	allMyNotes.insert(note);
	note->getPos(pos(), width());
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

void Notes::updateText()
{
	text = ctrlTxt.toPlainText();
}

void Notes::setColorByAction(QAction * act)
{
	color = colors.find(act->text()).value();
	update();
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
	place = geometry();
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
			if(GetWindowText(hwnd, title, 255))
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
		if (note != this) //может наступить момент, когда он будет сам с собой сверяться. отсекаем на всякий случай
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

void Medium::slotSaveNotes()
{
	QFile file("notes.dat");
	file.open(QIODevice::WriteOnly);
	QDataStream out(&file);

	auto it = allMyNotes.begin();
	while (it != allMyNotes.end())
		if ((*it)->empty())
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
	note->getPos(QPoint(rand()%100 + 100, rand()%100 + 100), 0);
	note->show();
	allMyNotes.insert(note);
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

	QFontDatabase::addApplicationFont (":/fonts/Flow.otf");
	QFontDatabase::addApplicationFont (":/fonts/Flow_B.otf");

	Medium medium;
	readNotes();

	return app.exec();
}
