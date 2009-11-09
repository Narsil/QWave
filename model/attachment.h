#ifndef ATTACHMENT_H
#define ATTACHMENT_H

#include <QSize>
#include <QImage>
#include <QUrl>
#include <QString>

#include "structureddocument.h"

class Attachment : public StructuredDocument
{
public:
    Attachment(QObject* parent = 0);

    QImage thumbnail() const;
    QSize thumbnailSize() const;
    QSize imageSize() const;
    QUrl srcUrl() const;
    QString id() const;

private:
    int findTag( const QString& tag ) const;
};

#endif // ATTACHMENT_H
