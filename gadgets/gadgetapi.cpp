#include "gadgetapi.h"
#include <QWebFrame>
#include <QWebPage>
#include <QFile>
#include <QTextStream>

QString* GadgetAPI::s_gadgetLib = 0;
QString* GadgetAPI::s_ostemplatesLib = 0;

GadgetAPI::GadgetAPI(QWebFrame* frame, QObject* parent)
        : QObject( parent ), m_frame(frame)
{
    if ( s_gadgetLib == 0 )
    {
        QFile file( "gadget.js" );
        bool check = file.open( QIODevice::ReadOnly );
        Q_ASSERT(check);
        QTextStream stream( &file );
        s_gadgetLib = new QString( stream.readAll() );
    }
    if ( s_ostemplatesLib == 0 )
    {
        QFile file( "ostemplates.js" );
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
    updateParticipants();
    m_frame->evaluateJavaScript("gadgets.util.callOnLoadHandlers_();");
}

void GadgetAPI::updateState()
{
    m_frame->evaluateJavaScript("wave.updateState_();");
}

void GadgetAPI::updateParticipants()
{
    m_frame->evaluateJavaScript("wave.updateWaveParticipants_();");
}

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
    emit submit( delta );
    // HACK
    updateState();
}

void GadgetAPI::state_submitValue( const QString& key, const QString& value )
{
    m_state[key] = value;
    emit submit( key, value );
    // HACK
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
    QVariantMap result;
    result["myId"] = QVariant("Horst");
    result["authorId"] = QVariant("Autor");
    QVariantList participants;
    participants.append( "Horst" );
    participants.append( "Autor" );
    result["participants"] = QVariant( participants );
    QVariantMap horst;
    horst["id"] = QVariant("Horst");
    horst["displayName"] = QVariant("Horst aus Duisburg");
    horst["thumbnailUrl"] = QVariant("http://secowela.googlecode.com/svn/trunk/Web/Gadget3/unknown.jpg");
    result["Horst"] = horst;
    QVariantMap autor;
    autor["id"] = QVariant("Autor");
    autor["displayName"] = QVariant("Gott");
    autor["thumbnailUrl"] = QVariant("http://secowela.googlecode.com/svn/trunk/Web/Gadget3/unknown.jpg");
    result["Horst"] = autor;
    return result;
}
