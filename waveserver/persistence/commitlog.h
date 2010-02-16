#ifndef COMMITLOG_H
#define COMMITLOG_H

#include <QObject>
#include <QFile>
#include "protocol/common.pb.h"

class AppliedWaveletDelta;
class Wavelet;

/**
  * Writes deltas in a log file and re-applies them after a crash.
  */
class CommitLog : public QObject
{
public:    
    ~CommitLog();

    bool write( Wavelet* wavelet, const AppliedWaveletDelta& delta );
    bool applyAll();
    void close();

    /**
      * @return the commit log and creates one if it does not yet exist.
      */
    static CommitLog* commitLog();

private:
    CommitLog();

    QFile m_file;
    bool m_applying;

    static CommitLog* s_commitLog;
};

#endif // COMMITLOG_H
