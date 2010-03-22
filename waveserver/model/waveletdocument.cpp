#include "waveletdocument.h"
#include "protocol/common.pb.h"
#include "protocol/waveclient-rpc.pb.h"
#include "wavelet.h"
#include "waveurl.h"

WaveletDocument::WaveletDocument(Wavelet* wavelet, const QString& name)
        : StructuredDocument(wavelet), m_wavelet(wavelet), m_name( name )
{
}

void WaveletDocument::toDocumentOperation( protocol::ProtocolDocumentOperation* op )
{
    qDebug("DOC = %s", this->toString().toAscii().constData() );

    QString str;

    Annotation anno;

    for( int i = 0; i < this->count(); ++i )
    {
        const Annotation& annotation = annotationAt(i);
        if ( annotation != anno )
        {
            if ( !str.isEmpty() )
            {
                protocol::ProtocolDocumentOperation_Component* c = op->add_component();
                c->set_characters( str.toStdString() );
                str = "";
            }
            protocol::ProtocolDocumentOperation_Component* c = op->add_component();
            protocol::ProtocolDocumentOperation_Component_AnnotationBoundary* b = c->mutable_annotation_boundary();
            foreach( QString key, annotation.keys() )
            {
                protocol::ProtocolDocumentOperation_Component_KeyValueUpdate* k = b->add_change();
                k->set_key( key.toStdString() );
                k->set_new_value( annotation.value(key).toStdString() );
                if ( anno.contains( key ) )
                    k->set_old_value( anno.value(key).toStdString() );
            }
            foreach( QString key, anno.keys() )
            {
                if ( !annotation.contains( key ) )
                {
                    b->add_end( key.toStdString() );
                }
            }

            anno = annotation;
        }

        switch( typeAt(i) )
        {
        case Start:
            if ( !str.isEmpty() )
            {
                protocol::ProtocolDocumentOperation_Component* c = op->add_component();
                c->set_characters( str.toStdString() );
                str = "";
            }
            {
                protocol::ProtocolDocumentOperation_Component* c = op->add_component();
                protocol::ProtocolDocumentOperation_Component_ElementStart* start = c->mutable_element_start();
                start->set_type( tagAt(i).toStdString() );
                const AttributeList& attributes = attributesAt(i);
                foreach( QString key, attributes.keys() )
                {
                    if ( key.mid(0,2) == "**" )
                        continue;
                    QString val = attributes[key];
                    protocol::ProtocolDocumentOperation_Component_KeyValuePair* pair = start->add_attribute();
                    pair->set_key( key.toStdString() );
                    pair->set_value( val.toStdString() );
                }            
            }
            break;
        case End:
            if ( !str.isEmpty() )
            {
                protocol::ProtocolDocumentOperation_Component* c = op->add_component();
                c->set_characters( str.toStdString() );
                str = "";
            }
            {
                protocol::ProtocolDocumentOperation_Component* c = op->add_component();
                c->set_element_end(true);
            }
            break;
        case Char:
            str += charAt(i);
            break;
        }
    }

    if ( !str.isEmpty() )
    {
        protocol::ProtocolDocumentOperation_Component* c = op->add_component();
        c->set_characters( str.toStdString() );
    }
}
