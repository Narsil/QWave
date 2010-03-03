#ifndef JSONMESSAGE_H
#define JSONMESSAGE_H

#include "libprotojs_global.h"
#include <string>

using namespace std;

class QByteArray;

class LIBPROTOJSSHARED_EXPORT JSONMessage
{
protected:
    JSONMessage() { }

    static QByteArray toJSONString( const string& str );
};

#endif // JSONMESSAGE_H
