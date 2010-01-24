#ifndef IMAGEHANDLER_H
#define IMAGEHANDLER_H

#include <QObject>
#include <QTextObjectInterface>
#include <QTextFormat>
#include <QFont>
#include <QPixmap>
#include <QString>

class QImage;

class ImageHandler : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    enum
    {
        ImageFormat = QTextFormat::UserObject +2
    };
    enum ImageData
    {
        Caption = 1,
        Id = 2,
        Size = 3
    };

    ImageHandler(QObject* parent = 0);

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);

    void insertImage(QTextCursor* cursor, const QString& id, const QImage& image, const QString& caption);

    static ImageHandler* initialize(QTextDocument* doc, QObject* parent = 0);

private:
    QSizeF textSize(const QString& text);

    QFont m_font;
    QHash<QString,QPixmap> m_pixmaps;
    QPixmap m_leftPixmap;
    QPixmap m_rightPixmap;
    QPixmap m_topPixmap;
    QPixmap m_bottomPixmap;
};

#endif // IMAGEHANDLER_H
