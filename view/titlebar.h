#ifndef TITLEBAR_H
#define TITLEBAR_H

#include <QWidget>
#include <QString>

class TitleBar : public QWidget
{
public:
    TitleBar(QWidget* parent);

    void setText( const QString& text );
    QString text() const { return m_text; }

protected:
    void paintEvent( QPaintEvent* event );

private:
    QString m_text;
};

#endif // TITLEBAR_H
