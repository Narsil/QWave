#ifndef COMMITLOG_H
#define COMMITLOG_H

#include <QObject>
#include <QFile>

class Store;

/**
  * Writes deltas in a log file and re-applies them after a crash.
  */
class CommitLog : public QObject
{
public:    
    CommitLog( Store* parent );
    ~CommitLog();

    QByteArray read();
    bool write( const QByteArray& data );
    void close();

private:
    QFile m_file;
};

#endif // COMMITLOG_H
