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

    protocol::ProtocolSignature sig;
    sig.set_signature_algorithm( protocol::ProtocolSignature_SignatureAlgorithm_SHA1_RSA );
    sig.set_signer_id( "foo" );
    sig.set_signature_bytes( "barx" );

    ba.clear();
    ok = protocol::ProtocolSignature_JSON::SerializeToArray( &sig, ba );
    Q_ASSERT(ok);

    qDebug("JSON=%s", ba.constData() );

    protocol::ProtocolAppliedWaveletDelta delta;
    delta.mutable_signed_original_delta()->add_signature()->MergeFrom( sig );
    delta.mutable_signed_original_delta()->set_delta("abcde");
    delta.mutable_hashed_version_applied_at()->MergeFrom( v );
    delta.set_application_timestamp( 1234 );
    delta.set_operations_applied( 2 );

    ba.clear();
    ok = protocol::ProtocolAppliedWaveletDelta_JSON::SerializeToArray( &delta, ba );
    Q_ASSERT(ok);

    qDebug("JSON=%s", ba.constData() );
}
