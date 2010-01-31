#include "commitlog.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/waveletdelta.h"
#include "network/converter.h"
#include "app/settings.h"
#include "protocol/commitlog.pb.h"

CommitLog* CommitLog::s_commitLog = 0;

CommitLog::CommitLog()         
{
    m_file.setFileName( Settings::settings()->logFile() );
    bool ok = m_file.open( QFile::ReadWrite );
    // TODO: Better error handling
    Q_ASSERT(ok);
}

CommitLog::~CommitLog()
{
    close();
}

bool CommitLog::applyAll()
{
    if ( !m_file.isOpen() )
        return false;

    commitlog::ProtocolSubmitRequestList lst;
    bool ok = lst.ParseFromFileDescriptor( m_file.handle() );
    if ( !ok )
    {
        qDebug("Error reading commit log");
        return false;
    }

    for( int i = 0; i < lst.requests_size(); ++i )
    {
        // Read a delta from the commit log
        const waveserver::ProtocolSubmitRequest& request = lst.requests(i);
        QString waveletId = QString::fromStdString( request.wavelet_name() );
        // WaveletDelta delta = Converter::convert( request.delta() );

        WaveUrl url( waveletId );
        if ( url.isNull() )
        {
            qDebug("Error reading commit log");
            return false;
        }

        // Find the wave
        Wave* wave = Wave::wave( url.waveDomain(), url.waveId(), (url.waveDomain() == Settings::settings()->domain()) );
        if ( !wave )
            continue;

        // TODO: What about remote wavelets?
        Wavelet* wavelet = wave->wavelet( url.waveletDomain(), url.waveletId(), (url.waveletDomain() == Settings::settings()->domain()) );
        if ( !wavelet )
            continue;

        // Apply the delta
        QString err = "";
        wavelet->apply(request.delta(), &err );
    }

    return true;
}

bool CommitLog::write( const waveserver::ProtocolSubmitRequest& request )
{
    if ( !m_file.isOpen() )
        return false;

    commitlog::ProtocolSubmitRequestList lst;
    waveserver::ProtocolSubmitRequest* r = lst.add_requests();
    r->MergeFrom( request );

    bool ok = lst.SerializeToFileDescriptor( m_file.handle() );
    return ok;
}

void CommitLog::close()
{
    m_file.close();
}

CommitLog* CommitLog::commitLog()
{
    if ( !s_commitLog )
        s_commitLog = new CommitLog();
    return s_commitLog;
}
