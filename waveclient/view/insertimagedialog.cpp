#include "insertimagedialog.h"
#include "network/networkadapter.h"
#include "app/environment.h"

#include <QGraphicsWidget>
#include <QGraphicsTextItem>
#include <QGraphicsPixmapItem>
#include <QPushButton>
#include <QGraphicsLineItem>
#include <QPen>
#include <QPixmap>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QLineEdit>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QImageReader>

#define IMAGE_SIZE 120

InsertImageDialog::InsertImageDialog(Environment* environment, QWidget* parent)
        : PopupDialog( parent ), m_reply(0), m_environment(environment)
{
    setMinimumWidth(500);
    setMinimumHeight(200);
    this->setSizeGripEnabled(false);

    m_pixmapItem = new QGraphicsPixmapItem( QPixmap("images/unknown.png"), widget() );
    m_pixmapItem->setPos( 8, 8 );

    QGraphicsLineItem* line = new QGraphicsLineItem( widget() );
    line->setPen( QPen( QColor( 100, 100, 100 ) ) );
    line->setLine( 8, 150, width() - 2 * 8, 150 );

    QGraphicsTextItem*  text = new QGraphicsTextItem( tr("Image URL"), widget() );
    text->setPos( 150, 10 );

    m_urlEdit = new QLineEdit();
    m_urlEdit->setText(tr("http://"));
    m_urlEdit->setMinimumWidth( 340 );
    QGraphicsProxyWidget* item3 = scene()->addWidget(m_urlEdit);
    item3->setParentItem( widget() );
    item3->setPos( 150, 32 );

    text = new QGraphicsTextItem( tr("Press <Return> to preview the image"), widget() );
    text->setFont( QFont("Arial", 8) );
    text->setPos( 150, 54 );

    text = new QGraphicsTextItem( tr("Image caption"), widget() );
    text->setPos( 150, 80 );

    m_captionEdit = new QLineEdit();
    m_captionEdit->setMinimumWidth( 340 );
    QGraphicsProxyWidget* item4 = scene()->addWidget(m_captionEdit);
    item4->setParentItem( widget() );
    item4->setPos( 150, 102 );

    QPushButton* button = new QPushButton();
    button->setText(tr("Cancel"));
    QGraphicsProxyWidget* item = scene()->addWidget(button);
    item->setParentItem( widget() );
    item->setPos( width() - item->preferredWidth() - 10, height() - 10 - item->preferredHeight() );
    connect( button, SIGNAL(clicked()), SLOT(reject()));

    m_insertButton = new QPushButton();
    m_insertButton->setText(tr("Insert"));
    m_insertButton->setEnabled(false);
    QGraphicsProxyWidget* item2 = scene()->addWidget(m_insertButton);
    item2->setParentItem( widget() );
    item2->setPos( width() - item->preferredWidth() - 10 - item2->preferredWidth() - 8, height() - 10 - item->preferredHeight() );

    connect( m_insertButton, SIGNAL(clicked()), SLOT(accept()));
    connect( m_urlEdit, SIGNAL(textEdited(QString)), SLOT(urlEdited(QString)));
    connect( m_urlEdit, SIGNAL(returnPressed()), SLOT(loadImage()));
}

void InsertImageDialog::loadImage()
{
    m_url = QUrl( m_urlEdit->text() );
    if ( !m_url.isValid() || m_url.isEmpty() )
        return;

    m_reply = m_environment->networkAdapter()->get( QNetworkRequest(m_url));
    connect( m_reply, SIGNAL(finished()), SLOT(showImage()));
}

void InsertImageDialog::showImage()
{
    QImageReader reader( m_reply );
    m_image = reader.read();
    m_reply->deleteLater();
    m_reply = 0;

    if ( m_image.isNull() )
    {
        m_url = QUrl();
        m_image = QImage();
        m_pixmapItem->setPixmap( QPixmap("images/unknown.png") );
        return;
    }

    QImage img;
    if ( m_image.height() > m_image.width() && m_image.height() > 120 )
        img = m_image.scaledToHeight(120, Qt::SmoothTransformation );
    else if ( m_image.width() > m_image.height() && m_image.width() > 120 )
        img = m_image.scaledToWidth(120, Qt::SmoothTransformation );
    else
        img = m_image;

    m_pixmapItem->setPixmap( QPixmap::fromImage( img ) );
    m_insertButton->setEnabled(true);
}

void InsertImageDialog::urlEdited(const QString&)
{
    if ( m_reply )
    {
        m_reply->abort();
        delete m_reply;
        m_reply = 0;
    }
    m_url = QUrl();
    if ( !m_image.isNull() )
    {
        m_image = QImage();
        m_pixmapItem->setPixmap( QPixmap("images/unknown.png") );
    }
    m_insertButton->setEnabled(false);
}

QString InsertImageDialog::caption() const
{
    return m_captionEdit->text();
}

QImage InsertImageDialog::thumbnail() const
{
    QImage image( m_image );
    if ( image.width() > 90 )
        image = image.scaledToWidth(90, Qt::SmoothTransformation );
    if ( image.height() > 60 )
        image = image.scaledToHeight(60, Qt::SmoothTransformation );
    return image;
}

