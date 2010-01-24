#include "gadgetmanifest.h"
#include "app/environment.h"
#include "network/networkadapter.h"
#include <QDomDocument>
#include <QDomNodeList>
#include <QDomCDATASection>
#include <QDomElement>
#include <QNetworkReply>
#include <QNetworkAccessManager>
// #include <QTextStream>

GadgetManifest::GadgetManifest(const QUrl& url, Environment* environment, QObject* parent)
        : QObject( parent ), m_url(url), m_reply(0), m_malformed(false), m_environment(environment)
{
}

void GadgetManifest::load()
{
    QNetworkAccessManager* net = m_environment->networkAdapter();
    m_reply = net->get( QNetworkRequest(m_url));
    connect( m_reply, SIGNAL(finished()), SLOT(parse()));
}

GadgetManifest::~GadgetManifest()
{
    if ( m_reply )
        delete m_reply;
}

void GadgetManifest::parse()
{
    QDomDocument doc("gadget" );
    if ( !doc.setContent( m_reply->readAll() ) )
    {
        m_malformed = true;
        emit finished();
    }

    QDomNodeList lst = doc.elementsByTagName("Module");
    if ( lst.count() != 1 )
    {
        m_malformed = true;
        emit finished();
    }
    QDomElement module = lst.item(0).toElement();

    lst = module.elementsByTagName("ModulePrefs");
    if ( lst.count() != 1 )
    {
        m_malformed = true;
        emit finished();
    }
    QDomElement modulePrefs = lst.item(0).toElement();
    if ( modulePrefs.hasAttribute("width" ) )
        m_width = modulePrefs.attribute("width").toInt();
    if ( modulePrefs.hasAttribute("height" ) )
        m_height = modulePrefs.attribute("height").toInt();
    if ( modulePrefs.hasAttribute("title" ) )
        m_title = modulePrefs.attribute("title").toInt();

    lst = modulePrefs.elementsByTagName("Require");
    for( int i = 0; i < lst.count(); ++i )
    {
        QDomElement r = lst.item(i).toElement();
        if ( r.hasAttribute("feature") )
            m_requiredFeatures.append(r.attribute("feature"));
    }

    lst = module.elementsByTagName("Content");
    if ( lst.count() != 1 )
    {
        m_malformed = true;
        emit finished();
    }
    QDomElement content = lst.item(0).toElement();
    if ( content.hasAttribute("type") )
        m_contentType = content.attribute("type");
    else
        m_contentType = "html";

    m_content = "";
    lst = content.childNodes();
    for( int i = 0; i < lst.count(); ++i )
    {
        QDomCDATASection cdata = lst.item(i).toCDATASection();
        if ( !cdata.isNull() )
            m_content.append( cdata.data() );
    }

    emit finished();
}
