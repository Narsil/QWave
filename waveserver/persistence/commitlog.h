#ifndef COMMITLOG_H
#define COMMITLOG_H

#include <QObject>
#include <QFile>
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"

class WaveletDelta;

/**
  * Writes deltas in a log file and re-applies them after a crash.
  */
class CommitLog : public QObject
{
public:    
    ~CommitLog();

    bool write( const waveserver::ProtocolSubmitRequest& request );
    bool applyAll();
    void close();

    /**
      * @return the commit log and creates one if it does not yet exist.
      */
    static CommitLog* commitLog();

private:
    CommitLog();

    QFile m_file;

    static CommitLog* s_commitLog;
};

#endif // COMMITLOG_H
