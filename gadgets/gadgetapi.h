#ifndef GADGETAPI_H
#define GADGETAPI_H

#include <QObject>
#include <QVariant>
#include <QHash>

class QWebFrame;
class GadgetView;

class GadgetAPI : public QObject
{
    Q_OBJECT
public:
    GadgetAPI(GadgetView* view, QWebFrame* frame, QObject* parent = 0);

    void setState( const QString& key, const QString& value );

    bool isInitialized() const { return m_initialized; }

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

    void gadgets_adjustHeight( int height );

    QVariantMap participants_getAll();

signals:
    void log( const QString& message );
    void stateReset();

private slots:
    void registerAtFrame();
    void loadFinished( bool ok );

private:
    void updateState();
    void updateParticipants();

    QWebFrame* m_frame;
    QHash<QString,QString> m_state;
    GadgetView* m_view;
    bool m_initialized;

    static QString* s_gadgetLib;
    static QString* s_ostemplatesLib;
};

#endif // GADGETAPI_H
