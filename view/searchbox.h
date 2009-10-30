#ifndef SEARCHBOX_H
#define SEARCHBOX_H

#include <QWidget>

class QLineEdit;

class SearchBox : public QWidget
{
    Q_OBJECT
public:
    SearchBox(QWidget* parent = 0);

    void setSearchText(const QString& search);
    QString searchText() const;

signals:
    void searchChanged( const QString& search );

protected:
    virtual void resizeEvent(QResizeEvent* event);
    virtual void paintEvent(QPaintEvent* event);

private:
    QLineEdit* m_edit;
};

#endif // SEARCHBOX_H
