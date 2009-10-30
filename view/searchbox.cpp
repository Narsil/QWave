#include "searchbox.h"
#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPixmap>

QPixmap* SearchBox::s_pixmapLeft = 0;
QPixmap* SearchBox::s_pixmapRight = 0;

SearchBox::SearchBox(QWidget* parent)
        : QLineEdit(parent)
{
    if ( !s_pixmapLeft )
        s_pixmapLeft = new QPixmap("images/searchbox_left.png");
    if ( !s_pixmapRight )
        s_pixmapRight = new QPixmap("images/searchbox_right.png");

    setFrame(false);
    this->setTextMargins(11, 3, 22, 3);
}

void SearchBox::paintEvent(QPaintEvent* event)
{
    QLineEdit::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, *s_pixmapLeft);
    painter.drawPixmap(width() - 22, 0, *s_pixmapRight);

    painter.setPen(QColor(0xff,0xff,0xff));
    painter.drawLine(11, height() - 1, width() - 22, height() - 1);
    painter.setPen(QColor(195,195,195));
    painter.drawLine(11, height() - 2, width() - 22, height() - 2);

    painter.setPen(QColor(167,167,167));
    painter.drawLine(11, 0, width() - 22, 0);
    painter.setPen(QColor(221,221,221));
    painter.drawLine(11, 1, width() - 22, 1);
    painter.setPen(QColor(230,230,230));
    painter.drawLine(11, 2, width() - 22, 2);
    painter.setPen(QColor(242,242,242));
    painter.drawLine(11, 3, width() - 22, 3);
    painter.setPen(QColor(251,251,251));
    painter.drawLine(11, 4, width() - 22, 4);
}
