#include "gadgetapi.h"
#include "gadgetview.h"
#include "app/environment.h"
#include "model/participant.h"
#include "model/blip.h"
#include "model/wavelet.h"
#include <QWebFrame>
#include <QWebPage>
#include <QFile>
#include <QTextStream>

QString* GadgetAPI::s_gadgetLib = 0;
QString* GadgetAPI::s_ostemplatesLib = 0;

GadgetAPI::GadgetAPI(GadgetView* view, QWebFrame* frame, QObject* parent)
        : QObject( parent ), m_frame(frame), m_view(view), m_initialized(false)
{
    if ( s_gadgetLib == 0 )
    {
        QFile file( "javascript/gadget.js" );
        bool check = file.open( QIODevice::ReadOnly );
        Q_ASSERT(check);
        QTextStream stream( &file );
        s_gadgetLib = new QString( stream.readAll() );
    }
    if ( s_ostemplatesLib == 0 )
    {
        QFile file( "javascript/ostemplates.js" );
        bool check = file.open( QIODevice::ReadOnly );
        Q_ASSERT(check);
        QTextStream stream( &file );
        s_ostemplatesLib = new QString( stream.readAll() );
    }

    connect( frame->page(), SIGNAL(loadFinished(bool)), SLOT(loadFinished(bool)));
    connect( frame, SIGNAL(javaScriptWindowObjectCleared()), SLOT(registerAtFrame()));
    registerAtFrame();
}

void GadgetAPI::registerAtFrame()
{
    m_frame->addToJavaScriptWindowObject( "gadgetAPI", this );
    m_frame->evaluateJavaScript( *s_gadgetLib );
    m_frame->evaluateJavaScript( *s_ostemplatesLib );
}

void GadgetAPI::loadFinished( bool ok )
{
    if ( !ok )
    {
        qDebug("Load FAILED");
        return;
    }
    qDebug("Load succeeded");
    m_frame->evaluateJavaScript("gadgets.util.callOnLoadHandlers_();");
    updateParticipants();
    updateState();
    m_initialized = true;
}

void GadgetAPI::setState( const QString& key, const QString& value )
{
    if ( value.isNull() )
        m_state.remove(key);
    else
        m_state[key] = value;
    if ( isInitialized() )
        updateState();
}

void GadgetAPI::updateState()
{
    m_frame->evaluateJavaScript("wave.updateState_();");
}

void GadgetAPI::addParticipant(Participant* participant)
{
    Q_UNUSED(participant)
    updateParticipants();
}

void GadgetAPI::removeParticipant(Participant* participant)
{
    Q_UNUSED(participant)
    updateParticipants();
}

void GadgetAPI::updateParticipants()
{
    m_frame->evaluateJavaScript("wave.updateWaveParticipants_();");
}

// TODO: Remove
void GadgetAPI::testme(const QVariant& obj)
{
    if ( obj.type() == QVariant::List )
    {
        QList<QVariant> lst = obj.toList();
        foreach( QVariant v, lst )
        {
            qDebug("TESTME %s", v.toString().toAscii().constData());
        }
    }
    else if ( obj.type() == QVariant::Map )
    {
        QMap<QString,QVariant> lst = obj.toMap();
        foreach( QString s, lst.keys() )
        {
            QVariant v = lst[s];
            qDebug("TESTME %s=%s", s.toAscii().constData(), v.toString().toAscii().constData());
        }
    }
}

// TODO: Remove
void GadgetAPI::testme2(const QVariantMap& lst)
{
        foreach( QString s, lst.keys() )
        {
            QVariant v = lst[s];
            qDebug("TESTME2 %s=%s", s.toAscii().constData(), v.toString().toAscii().constData());
        }
}

QVariantMap GadgetAPI::state_getAll()
{
    QVariantMap result;
    foreach( QString key, m_state.keys() )
    {
        QString v = m_state[key];
        result[key] = QVariant(v);
    }
    return result;
}

QVariantList GadgetAPI::state_getKeys()
{
    QVariantList result;
    foreach( QString key, m_state.keys() )
    {
        result.append( QVariant( key ) );
    }
    return result;
}

QString GadgetAPI::state_get( const QString& key, const QString& optional )
{
    if ( m_state.contains(key) )
        return m_state[key];
    return optional;
}

void GadgetAPI::state_submitDelta( const QVariantMap& object )
{
    QHash<QString,QString> delta;
    foreach( QString key, object.keys() )
    {
        delta[key] = object[key].toString();
        m_state[key] = delta[key];
    }
    m_view->onSubmit( delta );

    updateState();
}

void GadgetAPI::state_submitValue( const QString& key, const QString& value )
{
    m_state[key] = value;
    m_view->onSubmit( key, value );

    updateState();
}

QString GadgetAPI::state_toString()
{
    QString result = "{";
    foreach( QString key, m_state.keys() )
    {
        result += key + ":'" + m_state[key] + "',";
    }
    result += "}";
    return result;
}

int GadgetAPI::wave_getMode()
{
    return 0; // Unknown
}

bool GadgetAPI::wave_isInWaveContainer()
{
    return true;
}

void GadgetAPI::wave_log( const QString& message )
{
    emit log(message);
}

QVariantMap GadgetAPI::participants_getAll()
{
    Environment* en = m_view->environment();

    QVariantMap result;
    // TODO
    result["authorId"] = QVariant("Autor");
    // Add the local user
    result["myId"] = QVariant(en->localUser()->address());
    QVariantMap participants;
//    participants.append( en->localUser()->address() );
//    QVariantMap lmap;
//    lmap["id"] = QVariant(en->localUser()->address());
//    lmap["displayName"] = QVariant(en->localUser()->name());
//    lmap["thumbnailUrl"] = QVariant("http://secowela.googlecode.com/svn/trunk/Web/Gadget3/unknown.jpg");
//    result[en->localUser()->address()] = lmap;

    foreach( Participant* p, m_view->blip()->wavelet()->participants() )
    {
        // participants.append( p->address() );
        QVariantMap map;
        map["id"] = QVariant(p->address());
        map["displayName"] = QVariant(p->name());
        map["thumbnailUrl"] = QVariant("http://secowela.googlecode.com/svn/trunk/Web/Gadget3/unknown.jpg");
        // participants.append( QVariant(map) );
        participants[p->address()] = QVariant(map);
        // result[p->address()] = map;
    }
    result["participants"] = QVariant( participants );
    return result;
}

void GadgetAPI::gadgets_adjustHeight( int height )
{
    m_view->adjustHeight( height );
}
