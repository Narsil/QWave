#include "jsonmessage.h"
#include <QByteArray>

// TODO: Does this properly escape all kind of non-printable ASCII characters?
QByteArray JSONMessage::toJSONString( const string& str )
{
    QByteArray result;
    result.reserve( str.length() + 2 );
    result.append("x");
    result.append( str.data(), str.length() );
    result.replace( '\\', "\\\\" );
    result.replace( '"', "\\\"" );
    result.replace( '\b', "\\b" );
    result.replace( '\f', "\\f" );
    result.replace( '\n', "\\n" );
    result.replace( '\r', "\\r" );
    result.replace( '\t', "\\t" );
    result.append("\"");
    result[0] = '"';
    return result;
}
