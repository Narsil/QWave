#ifndef GADGETAPI_H
#define GADGETAPI_H

#include <QObject>
#include <QVariant>
#include <QHash>

class QWebFrame;

class GadgetAPI : public QObject
{
    Q_OBJECT
public:
    GadgetAPI(QWebFrame* frame, QObject* parent = 0);

public slots:
    void testme(const QVariant& text);
    void testme2(const QVariantMap& text);

    QVariantMap state_getAll();
    QVariantList state_getKeys();
    QString state_get( const QString& key, const QString& optional );
    void state_submitDelta( const QVariantMap& object );
    void state_submitValue( const QString& key, const QString& value );
    QString state_toString();

    int wave_getMode();
    bool wave_isInWaveContainer();
    void wave_log( const QString& message );

    QVariantMap participants_getAll();

signals:
    void log( const QString& message );
    void submit( const QString& key, const QString& value );
    void submit( const QHash<QString,QString>& delta );
    void stateReset();

private slots:
    void registerAtFrame();
    void loadFinished( bool ok );

private:
    void updateState();
    void updateParticipants();

    QWebFrame* m_frame;
    QHash<QString,QString> m_state;

    static QString* s_gadgetLib;
    static QString* s_ostemplatesLib;
};

#endif // GADGETAPI_H
