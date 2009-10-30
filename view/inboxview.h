#ifndef INBOXVIEW_H
#define INBOXVIEW_H

#include <QWidget>

class Environment;
class TitleBar;
class WaveListView;
class SearchBox;
class Wave;
class BigBar;
class QVBoxLayout;

class InboxView : public QWidget
{
    Q_OBJECT
public:
    InboxView(Environment * environment, QWidget* parent = 0);

    void select( Wave* wave );

signals:
    void selected(Wave* wave);

private:
    WaveListView* m_listView;
    SearchBox* m_searchBox;
    TitleBar* m_titleBar;
    BigBar* m_bigBar;
    QVBoxLayout* m_verticalLayout;
    Environment* m_environment;
};

#endif // INBOXVIEW_H
