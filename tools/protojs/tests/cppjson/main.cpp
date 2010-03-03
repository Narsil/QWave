#include <QtCore/QCoreApplication>

#include "common.pbjson.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    protocol::ProtocolHashedVersion v;
    v.set_version(123);
    v.set_history_hash("wave://doof");

    QByteArray ba;
    bool ok = protocol::ProtocolHashedVersion_JSON::SerializeToArray( &v, ba );
    Q_ASSERT(ok);

    qDebug("JSON=%s", ba.constData() );
}
