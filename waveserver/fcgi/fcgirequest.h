#ifndef FCGIREQUEST_H
#define FCGIREQUEST_H

#include <QtGlobal>
#include <string>
#include <map>

namespace FCGI
{
    class FCGIProtocol;

    class FCGIRequest
    {
    public:
        enum Role
        {
            RESPONDER  = 1,
            AUTHORIZER = 2,
            FILTER     = 3
        };

        enum ProtocolStatus
        {
            REQUEST_COMPLETE = 0,
            CANT_MPX_CONN    = 1,
            OVERLOADED       = 2,
            UNKNOWN_ROLE     = 3
        };

        FCGIRequest(FCGIProtocol& driver, quint16 id, Role role, bool keepConnection);
        ~FCGIRequest();

    protected:
        enum OStreamType
        {
            STDOUT,
            STDERR
        };

        bool write(const std::string& buf, OStreamType stream = STDOUT);
        bool write(const char* buf, size_t count, OStreamType stream = STDOUT);
        void endRequest(quint32 appStatus, ProtocolStatus protStatus);

    protected:
        friend class FCGIProtocol;

        /**
          * Invoked if the web server aborts the request. Clean up all allocated resources here.
          */
        void onAbort();
        void appendStdin( const char* data, size_t len );
        void appendParam( const std::string& name, const std::string& value );
        void process();

    private:
        quint16 const m_id;
        Role const m_role;
        bool const m_keepConnection;
        std::map<std::string,std::string> m_params;
        std::string m_stdinStream;
        bool m_stdinEOF;
        bool m_aborted;
        FCGIProtocol& m_driver;
    };
}

#endif // FCGIREQUEST_H