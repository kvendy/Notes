#ifndef _notes_h_
#define __notes_h_

#include "geometry.h"
#include <QSystemTrayIcon>
#include <QDialog>
#include <QPushButton>
#include <QTextEdit>

#ifdef Q_OS_WIN32
#include <Windows.h>
#elif defined(Q_OS_LINUX)
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QTextEdit;
class QMenu;
class QSystemTrayIcon;
QT_END_NAMESPACE

class NotesData
{
public:
	NotesData();
	NotesData(QString inText, QColor inColor, QRect inPlace, bool inTop);
protected:
	QString text;
	QColor color;
	QRect place;
	bool onTop;

	friend QDataStream &operator <<(QDataStream &stream, const NotesData &note);
	friend QDataStream &operator <<(QDataStream &stream, NotesData *note);
	friend QDataStream &operator >>(QDataStream &stream, NotesData &note);
};

class Notes : public QWidget, public NotesData
{
	Q_OBJECT
private:
	QPushButton cmdNew;
	QPushButton cmdTop;
	QPushButton cmdClose;
	QTextEdit ctrlTxt;
	QMenu* pmnu;
	int mousePressedX, mousePressedY;
	bool isPressed;
	Position position;
	SnapManager sm;
	QList<QRect> otherWindows;
	QList<QString> otherWindowsNames;
	QMap<QString, QColor> colors;
	void init();
	void adjustMyRect();
public:
	Notes();
	Notes(NotesData nData);
	Notes(QString inText, QColor inColor, QRect inPlace, bool inTop);
	virtual ~Notes();
	bool isEmpty();

#ifdef Q_OS_WIN32
	static BOOL CALLBACK StaticEnumWindowsProc(HWND hwnd, LPARAM lParam);
	BOOL EnumWindowsProc(HWND hwnd);
#elif defined(Q_OS_LINUX)
	void enumerateWindows(Display *display, Window rootWindow);
#endif

	void getOSWindows();
	void getMyWindows();

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent (QMouseEvent*);
	virtual void mouseReleaseEvent (QMouseEvent*);
	virtual void mouseMoveEvent (QMouseEvent*);
	virtual void contextMenuEvent (QContextMenuEvent*);
	virtual void closeEvent(QCloseEvent *event);
public slots:
	void newForm();
	void topForm();
	void makeBold();
	void makeItalic();
	void makePlain();
	void makeUnderline();
	void makeStrike();
	void updateText();
	void setColorByAction(QAction*act);
};
//------------------------------------------------------------------------------

class Medium : public QObject
{
	Q_OBJECT
public:
	Medium(QObject* pobj = nullptr);
	virtual ~Medium();
	QMenu* pmnu;
	QSystemTrayIcon *trayIcon;
private:
	bool notesHide;
public slots:
	void slotSaveNotes();
	void slotHideNotes();
	void slotNewNote();
	void slotTrayAct(QSystemTrayIcon::ActivationReason reason);
};
#endif //_notes_h_
