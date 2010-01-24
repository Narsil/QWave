#include "popupdialog.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include <QGraphicsWidget>

PopupDialog::PopupDialog(QWidget* parent)
        : QDialog(parent)
{
    m_view = new QGraphicsView(this);
    m_view->setFrameShape(QFrame::NoFrame);
    m_view->setFrameShadow(QFrame::Plain);
    m_view->setLineWidth(0);
    m_view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_view->setInteractive(true);
    m_view->setAttribute(Qt::WA_Hover, true);
    m_view->setAlignment(Qt::AlignLeft | Qt::AlignTop);

    m_scene = new QGraphicsScene(this);
    m_view->setScene(m_scene);
    m_widget = new QGraphicsWidget();
    m_scene->addItem(m_widget);
}

void PopupDialog::resizeEvent( QResizeEvent* event )
{
    QDialog::resizeEvent(event);
    m_view->setSceneRect( this->rect() );
    m_view->setGeometry( this->rect() );
    m_widget->setGeometry( 0, 0, m_view->frameRect().width(), m_view->frameRect().height() );
}
