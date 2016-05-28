#ifndef _notes_h_
#define __notes_h_

#include "geometry.h"
#include <QSystemTrayIcon>
#include <QDialog>
#include <QPushButton>
#include <QTextEdit>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

QT_BEGIN_NAMESPACE
class QPushButton;
class QLabel;
class QTextEdit;
class QMenu;
class QSystemTrayIcon;
QT_END_NAMESPACE

class SaveContent;

class Notes : public QWidget
{
	Q_OBJECT
private:
	QPushButton cmdNew;
	QPushButton cmdTop;
	QPushButton cmdClose;
	QMenu* pmnu;
	int mousePressedX, mousePressedY;
	bool isPressed;
	Position position;
	QList<QRect> otherWindows;
	QList<QString> otherWindowsNames;
	QMap<QString, QColor> colors;
	Line horLines, vertLines;
	void init();
public:
	Notes();
	Notes(QColor, QColor, QPoint, QSize, QString, bool);
	//~Notes();
	void getPos (QPoint, int);
	QTextEdit txtl;
	QColor color1, color2;
	int instance;
	bool onTop;

#ifdef Q_OS_WIN32
	static BOOL CALLBACK StaticEnumWindowsProc(HWND hwnd, LPARAM lParam);
	BOOL EnumWindowsProc(HWND hwnd);
#endif

	void getOSWindows();
	void snap(int &x, int &y, int &width, int &height);

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent (QMouseEvent*);
	virtual void mouseReleaseEvent (QMouseEvent*);
	virtual void mouseMoveEvent (QMouseEvent*);
	virtual void contextMenuEvent (QContextMenuEvent*);
public slots:
	void newForm();
	void closeForm();
	void topForm();
	void setColorByAction(QAction*act);
};
//------------------------------------------------------------------------------

class Medium : public QObject
{
	Q_OBJECT
public:
	Medium(QObject* pobj = 0);
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
