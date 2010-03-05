#include "fcgirequest.h"
#include "fcgiprotocol.h"
#include "fcgi.h"
#include "fcgiclientconnection.h"
#include "network/clientactorfolk.h"
#include "protocol/webclient.pbjson.h"
#include "model/jid.h"
#include "actor/pbmessage.h"

#include <QByteArray>

qint64 FCGI::FCGIRequest::s_id = 0;

FCGI::FCGIRequest::FCGIRequest(FCGIProtocol* driver, quint16 id, Role role, bool kc)
  : ActorGroup( QString::number( s_id++ ), driver ), m_id(id), m_role(role), m_keepConnection(kc), m_stdinEOF(false), m_aborted(false), m_driver(driver)
{
}

FCGI::FCGIRequest::~FCGIRequest()
{
}

bool FCGI::FCGIRequest::write(const std::string& buf, OStreamType stream)
{
    return write(buf.data(), buf.size(), stream);
}

bool FCGI::FCGIRequest::write(const char* buf, size_t count, OStreamType stream)
{
    if (count > 0xffff)
        return false;
    else if (count == 0)
        return true;

    // Construct message.
    Header h(stream == STDOUT ? TYPE_STDOUT : TYPE_STDERR, m_id, count);
    bool ok = m_driver->write((const char*)&h, sizeof(Header));
    if ( !ok ) return false;
    ok = m_driver->write(buf, count);
    return ok;
}

void FCGI::FCGIRequest::endRequest(quint32 appStatus, FCGI::FCGIRequest::ProtocolStatus protStatus)
{
    // Terminate the stdout and stderr stream, and send the
    // end-request message.

    quint8 buf[64];
    quint8* p = buf;

    new(p) Header(TYPE_STDOUT, m_id, 0);
    p += sizeof(Header);
    new(p) Header(TYPE_STDERR, m_id, 0);
    p += sizeof(Header);
    new(p) EndRequestMsg(m_id, appStatus, protStatus);
    p += sizeof(EndRequestMsg);
    m_driver->write((const char*)buf, p - buf);
    m_driver->terminateRequest(m_id);
}

void FCGI::FCGIRequest::onAbort()
{
    m_aborted = true;
}

void FCGI::FCGIRequest::appendStdin( const char* data, size_t len )
{
    if ( len == 0 )
        m_stdinEOF = true;
    else
        m_stdinStream.append( data, len );
}

void FCGI::FCGIRequest::appendParam( const std::string& name, const std::string& value )
{
    m_params[name] = value;
}

#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/socket.h>

void FCGI::FCGIRequest::process()
{
    qDebug("Starting to handle request #%i", m_id );

    // Try to parse the data sent as a JSON protobuf
    QByteArray ba = QByteArray::fromRawData( m_stdinStream.data(), m_stdinStream.length() );
    webclient::Request r;
    if ( !webclient::Request_JSON::ParseFromArray( &r, ba ) )
    {
        errorReply("Could not parse request");
        return;
    }

    if ( r.has_login() )
    {
        // TODO: Need better session IDs
        // Create a dummy session ID
        QString sid = QString::number( qrand(), 16 );
        JID jid( QString::fromStdString( r.login().jid() ) );
        if ( !jid.isValid() )
        {
            errorReply("JID is not valid");
            return;
        }
        if ( !jid.isLocal() )
        {
            errorReply("Not a local JID");
            return;
        }
        if ( r.client_ack() != 0 )
        {
            errorReply("Upon login, the client cannot ack any server message.");
            return;
        }
        if ( r.client_sequence_number() != 1 )
        {
            errorReply("Upon login, the client sequence number must be 1.");
            return;
        }

        FCGIClientConnection* con = ClientActorFolk::instance()->newFCGIClientConnection( sid, jid.toString() );
        if ( !con )
        {
            errorReply("Internal server error");
            return;
        }

        webclient::Response ret;
        ret.mutable_login()->set_session_id( sid.toStdString() );
        ret.set_server_sequence_number(0);
        ret.set_server_ack( r.client_sequence_number() );
        reply( ret );
        return;
    }

    PBMessage<webclient::Request>* request = new PBMessage<webclient::Request>( ActorId("client", QString::fromStdString( r.session_id() ) ) );
    request->MergeFrom( r );
    bool ok = post( request );
    if ( !ok )
    {
        errorReply("Session ID is invalid");
        return;
    }

//    else if ( r.has_open() )
//    {
//        PBMessage<waveserver::ProtocolOpenRequest>* open = new PBMessage<waveserver::ProtocolOpenRequest>( ActorId("client", QString::fromStdString( r.session_id() ) ) );
//        open->MergeFrom( r.open() );
//        bool ok = ActorDispatcher::dispatcher()->send( open );
//        if ( !ok )
//        {
//            errorReply("Session ID is invalid");
//            return;
//        }
//
//        webclient::Response ret;
//        ret.set_ack( true );
//        reply( ret );
//        return;
//    }
//    else if ( r.has_pull() )
//    {
//
//    }

//    std::ostringstream os;
//    os << "Content-type: text/html\r\n"
//       << "\r\n"
//       << "<title>FastCGI Test Program</title>" << std::endl
//       << "<h1 align=center>FastCGI Test Program</h1>" << std::endl
//       << "<h3>FastCGI Status</h3>" << std::endl
//       << "Test Program Compile Time = " << __DATE__ " " __TIME__ << "<br>" << std::endl
//       << "Process id                = " << getpid() << "<br>" << std::endl
//       << "Request id                = " << m_id << "<br>" << std::endl
//       << "<h3>Request Environment</h3>" << std::endl;
//    for (std::map<std::string,std::string>::const_iterator i = m_params.begin(); i != m_params.end(); ++i)
//        os << i->first << "&nbsp;=&nbsp;" << i->second << "<br>" << std::endl;
//    write(os.str().data(), os.str().size());
//
//    // Make sure we read the entire standard input stream, then
//    // echo it back.
//
//    write("<h3>Input Stream</h3>\n<pre>\n");
//    if (!m_stdinStream.empty())
//    {
//        write(m_stdinStream);
//        m_stdinStream.erase();
//    }
//    write("</pre>\n");
//
//    // Terminate the request.
//
//    qDebug("Request #%i handled successfully.", m_id );
//    endRequest(0, FCGIRequest::REQUEST_COMPLETE);
}

void FCGI::FCGIRequest::errorReply(const std::string& str)
{
    std::ostringstream os;
    os << "Content-type: text/html\r\n"
            << "\r\n"
            << str;
    write(os.str().data(), os.str().size());
    endRequest(0, FCGIRequest::REQUEST_COMPLETE);
}

void FCGI::FCGIRequest::reply(webclient::Response& response)
{
    QByteArray ba;
    ba.append( "Content-type: text/html\r\n\r\n" );
    bool ok = webclient::Response_JSON::SerializeToArray( &response, ba );
    if ( !ok )
    {
        errorReply("Internal server error");
        return;
    }
    write(ba.constData(), ba.length());
    endRequest(0, FCGIRequest::REQUEST_COMPLETE);
}

void FCGI::FCGIRequest::customEvent( QEvent* event )
{
    PBMessage<webclient::Response>* response = dynamic_cast<PBMessage<webclient::Response>*>( event );
    if ( response )
    {
        reply( *response );
        return;
    }

    this->ActorGroup::customEvent( event );
}
