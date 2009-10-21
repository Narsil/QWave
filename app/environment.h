#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <QObject>
#include <QHash>
#include <QString>

class Wavelet;
class Participant;
class NetworkAdapter;

class Environment : public QObject
{
public:
    Environment(const QString& address, const QString& name);

    Participant* localUser() const;
    NetworkAdapter* networkAdapter() const;
    Wavelet* wavelet( const QString& id ) const { return m_wavelets[id]; }

    void addWavelet( Wavelet* wavelet );
    void removeWavelet( Wavelet* wavelet );

private:
    Participant* m_localUser;
    NetworkAdapter* m_networkAdapter;
    QHash<QString,Wavelet*> m_wavelets;
};

#endif // ENVIRONMENT_H
