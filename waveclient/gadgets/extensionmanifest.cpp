#include "extensionmanifest.h"
#include "network/networkadapter.h"
#include "app/environment.h"
#include <QNetworkReply>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>

ExtensionManifest::ExtensionManifest(const QUrl& url, Environment* environment)
        : m_url( url ), m_reply(0), m_environment(environment), m_malformed(false)
{
}

void ExtensionManifest::load()
{
    QNetworkAccessManager* net = m_environment->networkAdapter();
    m_reply = net->get( QNetworkRequest(m_url));
    connect( m_reply, SIGNAL(finished()), SLOT(parse()));
}

void ExtensionManifest::parse()
{
    QDomDocument doc( "extension" );
    if ( !doc.setContent( m_reply->readAll() ) )
    {
        m_malformed = true;
        emit finished();
    }

    // TDOO
    emit finished();
}
