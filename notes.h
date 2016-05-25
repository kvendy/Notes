#ifndef _notes_h_
#define __notes_h_

#include <QSystemTrayIcon>
#include <QDialog>
#include <QPushButton>
#include <QTextEdit>

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
	bool isPressed, moveTop, moveBottom, moveRight, moveLeft;
	void init();
	Qt::CursorShape chooseCursorShape(int x, int y, int width, int height);
public:
	Notes();
	Notes(QColor, QColor, QPoint, QSize, QString, bool);
	//~Notes();
	void getPos (QPoint, int);
	QTextEdit txtl;
	QColor color1, color2;
	int instance;
	bool onTop;
protected:
	virtual void paintEvent(QPaintEvent *);
	virtual void mousePressEvent (QMouseEvent*);
	virtual void mouseReleaseEvent (QMouseEvent*);
	virtual void mouseMoveEvent (QMouseEvent*);
	virtual void contextMenuEvent (QContextMenuEvent*);
public slots:
	void slotNewForm();
	void slotCloseForm();
	void slotTopForm();
	void slotSetColor1();
	void slotSetColor2();
	void slotSetColor3();
	void slotSetColor4();
	void slotSetColor5();
	void slotSetColor6();
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
