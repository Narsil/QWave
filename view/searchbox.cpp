#include "searchbox.h"
#include <QLineEdit>
#include <QPainter>
#include <QColor>
#include <QBrush>

SearchBox::SearchBox(QWidget* parent)
        : QWidget(parent)
{
    setMinimumHeight( 42 + 2*5 );
    setMaximumHeight( minimumHeight() );

    m_edit = new QLineEdit(this);
    m_edit->setFrame(false);

    connect( m_edit, SIGNAL(textChanged(QString)), SIGNAL(searchChanged(QString)));
}

void SearchBox::setSearchText(const QString& search)
{
    m_edit->setText(search);
}

QString SearchBox::searchText() const
{
    return m_edit->text();
}

void SearchBox::resizeEvent(QResizeEvent*)
{
    m_edit->setGeometry( 10, ( 42 + 2*5 - m_edit->height()) / 2, width() - 2*10, m_edit->height() );
}

void SearchBox::paintEvent(QPaintEvent*)
{
    QPainter painter(this);

    QBrush brush2(QColor(0xc9,0xe2,0xfc));
    painter.fillRect(0, 0, width(), height(), brush2);
}
