#ifndef POPUPDIALOG_H
#define POPUPDIALOG_H

#include <QDialog>

class QGraphicsView;
class QGraphicsScene;
class QGraphicsWidget;

/**
  * An abstract base class for popup dialogs which use a QGraphicsView to show
  * the contents of the dialog.
  */
class PopupDialog : public QDialog
{
public:
    PopupDialog(QWidget* parent = 0);

    QGraphicsWidget* widget() const { return m_widget; }
    QGraphicsScene* scene() const { return m_scene; }

protected:
    virtual void resizeEvent( QResizeEvent* event );

private:
    QGraphicsView* m_view;
    QGraphicsScene* m_scene;
    QGraphicsWidget* m_widget;
};

#endif // POPUPDIALOG_H
