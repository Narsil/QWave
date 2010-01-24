#include "attachment.h"
#include <QByteArray>

Attachment::Attachment(QObject* parent)
        : StructuredDocument(parent)
{
}

QImage Attachment::thumbnail() const
{
    int index = findTag("thumbnail");
    if ( index == -1 )
        return QImage();
    QString str = "";
    for( int i = index + 1; i < count(); ++i )
    {
        if ( typeAt(i) != Char )
            break;
        str += this->charAt(i);
    }
    QByteArray ba = QByteArray::fromBase64( str.toAscii() );
    return QImage::fromData(ba);
}

QSize Attachment::thumbnailSize() const
{
    int index = findTag("thumbnail");
    if ( index == -1 )
        return QSize();
    AttributeList attribs = this->attributesAt(index);
    bool ok = true;
    int w = attribs["width"].toInt( &ok );
    if ( !ok )
        return QSize();
    int h = attribs["height"].toInt( &ok );
    if ( !ok )
        return QSize();
    return QSize(w, h);
}

QSize Attachment::imageSize() const
{
    int index = findTag("image");
    if ( index == -1 )
        return QSize();
    AttributeList attribs = this->attributesAt(index);
    bool ok = true;
    int w = attribs["width"].toInt( &ok );
    if ( !ok )
        return QSize();
    int h = attribs["height"].toInt( &ok );
    if ( !ok )
        return QSize();
    return QSize(w, h);
}

QUrl Attachment::srcUrl() const
{
    int index = findTag("attachment");
    if ( index == -1 )
        return QUrl();
    AttributeList attribs = this->attributesAt(index);
    return QUrl( attribs["src"] );
}

QString Attachment::id() const
{
    int index = findTag("attachment");
    if ( index == -1 )
        return QString();
    AttributeList attribs = this->attributesAt(index);
    return attribs["attachmentId"];
}

int Attachment::findTag( const QString& tag ) const
{
    for( int i = 0; i < count(); ++i )
    {
        if ( typeAt(i) == Start )
            if ( tagAt(i) == tag )
                return i;
    }
    return -1;
}

