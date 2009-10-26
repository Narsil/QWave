#ifndef NETWORKADAPTER_H
#define NETWORKADAPTER_H

#include <QObject>
#include <QList>

class QByteArray;
class DocumentMutation;
class Environment;
class RPC;
class Wavelet;
class Participant;

class NetworkAdapter : public QObject
{
    Q_OBJECT
public:
    NetworkAdapter(QObject* parent = 0);

    bool openWavelet(Wavelet* wavelet);
    void send( const DocumentMutation& mutation, const QString& waveletId, const QString& docId );
    void receive( const DocumentMutation& mutation, const QString& waveletId, const QString& docId );

    Environment* environment() const;

    void setServer( const QString& serverName, quint32 serverPort );
    QString serverName() const { return m_serverName; }
    quint32 serverPort() const { return m_serverPort; }
    bool isOnline() const { return m_isOnline; }

    void sendAddParticipant(Wavelet* wavelet, Participant* participant);

private slots:
    void getOnline();
    void getOffline();
    void messageReceived(const QString& methodName, const QByteArray& data);

private:
    void sendOpenWave(const QString& waveId, const QString& waveletId);

    RPC* m_rpc;
    QList<QString> m_openWaves;
    bool m_isOnline;
    QString m_serverName;
    quint32 m_serverPort;

    static NetworkAdapter* s1;
    static NetworkAdapter* s2;
};

#endif // NETWORKADAPTER_H
