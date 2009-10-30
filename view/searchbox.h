#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QLineEdit>

class QPixmap;

class SearchBox : public QLineEdit
{
    Q_OBJECT
public:
    SearchBox(QWidget* parent = 0);

protected:
    virtual void paintEvent(QPaintEvent* event);

private:
    static QPixmap* s_pixmapLeft;
    static QPixmap* s_pixmapRight;
};

#endif // SEARCHBOX_H
