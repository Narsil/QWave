#include "bigbar.h"
#include <QPainter>
#include <QColor>
#include <QBrush>

BigBar::BigBar(QWidget* parent)
        : QWidget( parent )
{
    setMinimumHeight( 42 + 2*5 );
    setMaximumHeight( minimumHeight() );
}

void BigBar::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QBrush brush2(QColor(0xc9,0xe2,0xfc));
    painter.fillRect(0, 0, width(), height(), brush2);
}

