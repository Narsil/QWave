#ifndef NETWORKADAPTER_H
#define NETWORKADAPTER_H

#include <QObject>
#include <QList>
#include <QNetworkAccessManager>

class QByteArray;
class DocumentMutation;
class Environment;
class RPC;
class Wavelet;
class WaveletDelta;
class Participant;

class NetworkAdapter : public QNetworkAccessManager
{
    Q_OBJECT
public:
    NetworkAdapter(QObject* parent = 0);

    Environment* environment() const;

    void setServer( const QString& serverName, quint32 serverPort );
    QString serverName() const { return m_serverName; }
    quint32 serverPort() const { return m_serverPort; }
    bool isOnline() const { return m_isOnline; }

    bool openWavelet(Wavelet* wavelet);
    void submit(const WaveletDelta& delta, Wavelet* wavelet);

signals:
    void connectionStatus( const QString& status );

private slots:
    void getOnline();
    void getOffline();
    void networkError();
    void messageReceived(const QString& methodName, const QByteArray& data);

private:
    void sendOpenWave(const QString& waveId, const QString& waveletId);

    RPC* m_rpc;
    QList<QString> m_openWaves;
    bool m_isOnline;
    QString m_serverName;
    quint32 m_serverPort;
};

#endif // NETWORKADAPTER_H
