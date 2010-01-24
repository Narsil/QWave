#include "titlebar.h"

#include <QPainter>
#include <QBrush>
#include <QColor>

TitleBar::TitleBar(QWidget* parent)
        : QWidget(parent)
{
    setMinimumHeight(20);
    setMaximumHeight( minimumHeight() );
}

void TitleBar::setText( const QString& text )
{
    if ( m_text == text )
        return;
    m_text = text;
    if ( m_text.isEmpty() )
        m_text = tr("(no title)");
    this->repaint();
}

void TitleBar::paintEvent( QPaintEvent* )
{
    QPainter painter(this);
    QBrush brush(QColor(0x55,0x90,0xd2));

    painter.fillRect( 0, 0, width(), height(), brush );
    painter.setPen( QColor(0xff,0xff,0xff) );
    painter.drawText( 5, height() - 6, m_text );
}
