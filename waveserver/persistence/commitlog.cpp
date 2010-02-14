#include "commitlog.h"
#include "model/wave.h"
#include "model/wavelet.h"
#include "model/localwavelet.h"
#include "model/remotewavelet.h"
#include "model/appliedwaveletdelta.h"
#include "network/converter.h"
#include "app/settings.h"
#include "protocol/commitlog.pb.h"

#include <string>

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

    commitlog::CommitLog lst;
    bool ok = lst.ParseFromFileDescriptor( m_file.handle() );
    if ( !ok )
    {
        qDebug("Error reading commit log");
        return false;
    }

    for( int i = 0; i < lst.entry_size(); ++i )
    {
        const commitlog::Entry& entry = lst.entry(i);
        // Read a delta from the commit log
        QString waveletId = QString::fromStdString( entry.wavelet_name() );
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

        Wavelet* wavelet = wave->wavelet( url.waveletDomain(), url.waveletId(), (url.waveletDomain() == Settings::settings()->domain()) );
        if ( !wavelet )
            continue;

        if ( wavelet->isLocal() )
        {
            LocalWavelet* localWavelet = dynamic_cast<LocalWavelet*>( wavelet );
            // Apply the delta
            QString err = "";
            int version = localWavelet->apply( entry.delta().signed_original_delta(), &err );
            if ( version == -1 || !err.isEmpty() )
            {
                qDebug("FAILED to apply commit log: %s", err.toAscii().constData() );
                return false;
            }
        }
        else
        {
            RemoteWavelet* remoteWavelet = dynamic_cast<RemoteWavelet*>( wavelet );

            AppliedWaveletDelta d( &entry.delta(), &ok );
            if ( !ok )
            {
                qDebug("FAILED to apply commit log");
                return false;
            }
            // Apply the delta
            QString err = "";
            ok = remoteWavelet->apply( d, &err );
            if ( !ok )
            {
                qDebug("FAILED to apply commit log: %s", err.toAscii().constData() );
                return false;
            }
        }
    }

    return true;
}

bool CommitLog::write( Wavelet* wavelet, const AppliedWaveletDelta& delta )
{
    if ( !m_file.isOpen() )
        return false;

    commitlog::CommitLog lst;
    commitlog::Entry* entry = lst.add_entry();
    std::string url = wavelet->url().toString().toStdString();
    entry->set_wavelet_name( url.data(), url.length() );
    protocol::ProtocolAppliedWaveletDelta* d = entry->mutable_delta();
    delta.toProtobuf( d );

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
