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
	- если окон несколько, течет размер
	- когда изменяю размер только в одной оси, прилипает к окнам по всем осям
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
    color2(color1.darker(130)),
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

	pmnu->addAction("&Blue", this, SLOT(slotSetColor3()));
	pmnu->addAction("&Green", this, SLOT(slotSetColor1()));
	pmnu->addAction("&Pink", this, SLOT(slotSetColor4()));
	pmnu->addAction("P&urple", this, SLOT(slotSetColor2()));
	pmnu->addAction("&White", this, SLOT(slotSetColor5()));
	pmnu->addAction("&Yellow", this, SLOT(slotSetColor6()));

	connect(&cmdNew, SIGNAL(clicked()), SLOT(slotNewForm()));
	connect(&cmdTop, SIGNAL(clicked()), SLOT(slotTopForm()));
	connect(&cmdClose, SIGNAL(clicked()), SLOT(slotCloseForm()));

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
	int a = x(), b = y(), c = width(), d = height();

	if (!isPressed)
		setCursor(Position(pe->x(), pe->y(), width(), height()).toCursorShape());

	if (isPressed)
	{
		if (position.left() && (c > MINIMAL_WIDTH))
		{
			c = width() - pe->x();
			a = pe->globalX();// - pressedX; //Оно работает нормально при значении Pressed равном нулю только почему-то
		}
		else if (position.right())
			c = pe->x();// + width() - pressedX; тут нужно старое значение ширины
		if (position.top() && (d > MINIMAL_WIDTH))
		{
			d = height() - pe->y();
			b = pe->globalY();// - pressedY;  //Оно работает нормально при значении Pressed равном нулю только почему-то
		}
		else if (position.bottom())
			d = pe->y();// + height() - pressedY; тут нужно старое значение ширины

		if (c < MINIMAL_WIDTH)
			c = MINIMAL_WIDTH;
		if (d < MINIMAL_WIDTH)
			d = MINIMAL_WIDTH;
		if (a > x() + width() - MINIMAL_WIDTH)
			a = x() + width() - MINIMAL_WIDTH;
		if (b > y() + height() - MINIMAL_WIDTH)
			b = y() + height() - MINIMAL_WIDTH;
		resize (c, d);

		if (!position.top() && !position.bottom() && !position.left() && !position.right())
		{
			a = pe->globalX() - mousePressedX;
			b = pe->globalY() - mousePressedY;
		}

		if (abs(a - qApp->desktop()->availableGeometry(this).x()) < SNAP)
		{
			if (position.left())
				resize(a - qApp->desktop()->availableGeometry(this).x() + width(), d);
			    //c = a - qApp->desktop()->availableGeometry().x() + width();
			a = qApp->desktop()->availableGeometry(this).x();
		}
		else if (abs(qApp->desktop()->availableGeometry(this).right() - (a + width())) < SNAP)
		{
			if (position.right())
				resize(qApp->desktop()->availableGeometry(this).right() - a, d);
			    //c = qApp->desktop()->availableGeometry().right() - a;
			//else
			    a = qApp->desktop()->availableGeometry(this).right() - width();
		}
		if (abs(b - qApp->desktop()->availableGeometry(this).y()) < SNAP)
		{
			if (position.top())
				resize(c, b - qApp->desktop()->availableGeometry(this).y() + height());
			    //d = b - qApp->desktop()->availableGeometry().y() + height();
			b = qApp->desktop()->availableGeometry(this).y();
		}
		else if (abs(qApp->desktop()->availableGeometry(this).bottom() - (b + height())) < SNAP)
		{
			if (position.bottom())
				resize(c, qApp->desktop()->availableGeometry(this).bottom() - b);
			    //d = qApp->desktop()->availableGeometry().bottom() - b;
			//else
			    b = qApp->desktop()->availableGeometry(this).bottom() - height();
		}

		//попробуем поприлипать к своим же окнам
		//Новый ресайз работает
		//но с проблемами: при ресайзе за угол и взаимодействии с несколькими другими окнами течет размер

		int anotherX1, anotherY1, anotherX2, anotherY2;

		foreach(Notes *note, allMyNotes)
		{
			if (note != this) //может наступить момент, когда он будет сам с собой сверяться. отсекаем на всякий случай
			{
				anotherX1 = note->x();
				anotherY1 = note->y();
				anotherX2 = note->x() + note->width();
				anotherY2 = note->y() + note->height();

				if ((x() < anotherX2 + SNAP)&&(x() > anotherX1 - width() - SNAP)) //область сравнения по горизонтали
				{
					//это у нас одинаковые стороны сближаются

					if (abs(anotherY2 - (b + height())) < SNAP)//это нижняя сторона
					{
						if (position.bottom())
							resize(c, anotherY2 - b);
						b = anotherY2 - height();
					}
					if (abs(b - anotherY1) < SNAP)//а это верхняя
					{
						if (position.top())
							resize(c, b - anotherY1 + height());
						b = anotherY1;
					}

					//а тут типа противоположные

					if (abs(b - anotherY2) < SNAP)//верх
					{
						if (position.top())
							resize(c, b - anotherY2 + height());
						b = anotherY2;
					}
					if (abs(anotherY1 - (b + height())) < SNAP)//низ
					{
						if (position.bottom())
							resize(c, anotherY1 - b);
						b = anotherY1 - height();
					}
				}

				if ((y() < anotherY2 + SNAP)&&(y() > anotherY1 - height() - SNAP)) //область сравнения по вертикали
				{
					//одинаковые

					if (abs(anotherX2 - (a + width())) < SNAP)//правая сторона
					{
						if (position.right())
							resize(anotherX2 - a, d);
						a = anotherX2 - width();
					}
					if (abs(a - anotherX1) < SNAP)//левая
					{
						if (position.left())
							resize(a - anotherX1 + width(), d);
						a = anotherX1;
					}

					//противоположные

					if (abs(a - anotherX2) < SNAP)//левая
					{
						if (position.left())
							resize(a - anotherX2 + width(), d);
						a = anotherX2;
					}
					if (abs(anotherX1 - (a + width())) < SNAP)//правая
					{
						if (position.right())
							resize(anotherX1 - a, d);
						a = anotherX1 - width();
					}
				}

				//на всякий случай, приоритет выше у того, что проверяется позже
			}
		}

		foreach(QRect it, otherWindows)
		{
			anotherX1 = it.x();
			anotherY1 = it.y();
			anotherX2 = it.x() + it.width();
			anotherY2 = it.y() + it.height();

			if ((x() < anotherX2 + SNAP)&&(x() > anotherX1 - width() - SNAP)) //область сравнения по горизонтали
			{
				//это у нас одинаковые стороны сближаются

				if (abs(anotherY2 - (b + height())) < SNAP)//это нижняя сторона
				{
					if (position.bottom())
						resize(c, anotherY2 - b);
					b = anotherY2 - height();
				}
				if (abs(b - anotherY1) < SNAP)//а это верхняя
				{
					if (position.top())
						resize(c, b - anotherY1 + height());
					b = anotherY1;
				}

				//а тут типа противоположные

				if (abs(b - anotherY2) < SNAP)//верх
				{
					if (position.top())
						resize(c, b - anotherY2 + height());
					b = anotherY2;
				}
				if (abs(anotherY1 - (b + height())) < SNAP)//низ
				{
					if (position.bottom())
						resize(c, anotherY1 - b);
					b = anotherY1 - height();
				}
			}

			if ((y() < anotherY2 + SNAP)&&(y() > anotherY1 - height() - SNAP)) //область сравнения по вертикали
			{
				//одинаковые

				if (abs(anotherX2 - (a + width())) < SNAP)//правая сторона
				{
					if (position.right())
						resize(anotherX2 - a, d);
					a = anotherX2 - width();
				}
				if (abs(a - anotherX1) < SNAP)//левая
				{
					if (position.left())
						resize(a - anotherX1 + width(), d);
					a = anotherX1;
				}

				//противоположные

				if (abs(a - anotherX2) < SNAP)//левая
				{
					if (position.left())
						resize(a - anotherX2 + width(), d);
					a = anotherX2;
				}
				if (abs(anotherX1 - (a + width())) < SNAP)//правая
				{
					if (position.right())
						resize(anotherX1 - a, d);
					a = anotherX1 - width();
				}
			}

			    //на всякий случай, приоритет выше у того, что проверяется позже
		}

		//resize (c, d);
		move(a, b);
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

void Notes::slotNewForm()
{
	Notes* note = new Notes();
	note->show();
	allMyNotes.append(note);
	note->getPos(pos(), width());
	resetNoteInstances();
}

void Notes::slotTopForm()
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

void Notes::slotCloseForm()
{
	allMyNotes.remove(instance);
	resetNoteInstances();
	close();
}

//это ужасно

void Notes::slotSetColor1()
{
	color1.setRgb(210,255,204);
	color2.setRgb(177,232,174);
	update ();
}

void Notes::slotSetColor2()
{
	color1.setRgb(222,218,254);
	color2.setRgb(198,184,254);
	update ();
}

void Notes::slotSetColor3()
{
	color1.setRgb(217,243,251);
	color2.setRgb(184,219,244);
	update ();
}

void Notes::slotSetColor4()
{
	color1.setRgb(246,211,246);
	color2.setRgb(235,174,235);
	update ();
}

void Notes::slotSetColor5()
{
	color1.setRgb(255,255,255);
	color2.setRgb(235,235,235);
	update ();
}

void Notes::slotSetColor6()
{
	color1.setRgb(254,254,204);
	color2.setRgb(252,249,161);
	update ();
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

//#ifdef Q_WS_X11
//    //linux code goes here
//#elif Q_WS_WIN32
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
		if (width != 0 && height != 0)
		{
			if(GetWindowText(hwnd, title, 255))
				otherWindowsNames.append(QString::fromWCharArray(title));

			otherWindows.append(QRect(x, y, width, height));
		}
	}

	return TRUE;
}

//#else

void Notes::getOSWindows()
{
	otherWindows.clear();
	otherWindowsNames.clear();

//#ifdef Q_WS_X11
//	//linux code goes here
//#elif Q_WS_WIN32
	::EnumWindows(StaticEnumWindowsProc, reinterpret_cast<LPARAM>(this));
//#else

	for (int i = 0; i < otherWindowsNames.size() && i < otherWindows.size(); i++)
		qDebug() << otherWindowsNames.at(i) << otherWindows.at(i);
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
