#ifndef BIGBAR_H
#define BIGBAR_H

#include <QWidget>

class BigBar : public QWidget
{
public:
    BigBar(QWidget* parent = 0);

protected:
    virtual void paintEvent(QPaintEvent*);
};

#endif // BIGBAR_H
