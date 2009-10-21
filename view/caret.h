#ifndef CARET_H
#define CARET_H

#include <QObject>
#include <QTextObjectInterface>
#include <QTextFormat>
#include <QFont>

class CaretInterface : public QObject, public QTextObjectInterface
{
    Q_OBJECT
    Q_INTERFACES(QTextObjectInterface)
public:
    enum
    {
        CaretFormat = QTextFormat::UserObject + 1
    };
    enum CaretData
    {
        Text = 1,
        Size = 2,
        Color = 3
    };

    CaretInterface(QObject* parent = 0);

    QSizeF intrinsicSize(QTextDocument *doc, int posInDocument, const QTextFormat &format);
    void drawObject(QPainter *painter, const QRectF &rect, QTextDocument *doc, int posInDocument, const QTextFormat &format);

    void insertCaret(QTextCursor* cursor, const QString& text, const QColor& color);
    QSizeF size(QString& text);

    static CaretInterface* initialize(QTextDocument* doc, QObject* parent = 0);

private:
    QSizeF textSize(const QString& text);

    QFont m_font;
};

#endif // CARET_H
