#include "converter.h"
#include "protocol/waveclient-rpc.pb.h"
#include "model/waveletdelta.h"

Converter::Converter()
{
}

/**
  * Convert the Qt-ified representation to the protocol buffer representation.
  */
void Converter::convert(protocol::ProtocolWaveletDelta* result, const WaveletDelta& delta )
{
    // Author
    result->set_author(delta.author().toStdString());
    // HashedVersion
    result->mutable_hashed_version()->set_version( delta.version().version );
    result->mutable_hashed_version()->set_history_hash( std::string( delta.version().hash.constData(), delta.version().hash.length() ) );

    for( int i = 0; i < delta.operations().length(); ++i )
    {
        const WaveletDeltaOperation& op = delta.operations()[i];
        protocol::ProtocolWaveletOperation* o = result->add_operation();
        if ( op.hasAddParticipant() )
            o->set_add_participant( op.addParticipant().toStdString() );
        if ( op.hasRemoveParticipant() )
            o->set_remove_participant( op.removeParticipant().toStdString() );
        if ( op.hasMutation() )
        {
            const DocumentMutation& mutation = op.mutation();
            protocol::ProtocolWaveletOperation_MutateDocument* m = o->mutable_mutate_document();
            m->set_document_id( op.documentId().toStdString() );
            protocol::ProtocolDocumentOperation* mo = m->mutable_document_operation();
            for( QList<DocumentMutation::Item>::const_iterator it = mutation.begin(); it != mutation.end(); ++it )
            {
                protocol::ProtocolDocumentOperation_Component* comp = mo->add_component();
                switch( (*it).type )
                {
                    case DocumentMutation::ElementStart:
                        comp->mutable_element_start()->set_type( (*it).text.toStdString() );
                        foreach( QString key, (*it).attributes.keys() )
                        {
                            protocol::ProtocolDocumentOperation_Component_KeyValuePair* pair = comp->mutable_element_start()->add_attribute();
                            pair->set_key( key.toStdString() );
                            pair->set_value( (*it).attributes[key].toStdString() );
                        }
                        break;
                    case DocumentMutation::ElementEnd:
                        comp->set_element_end(true);
                        break;
                    case DocumentMutation::Retain:
                        comp->set_retain_item_count((*it).count);
                        break;
                    case DocumentMutation::InsertChars:
                        comp->set_characters( (*it).text.toStdString() );
                        break;
                    case DocumentMutation::DeleteStart:
                        comp->mutable_delete_element_start()->set_type( (*it).text.toStdString() );
                        foreach( QString key, (*it).attributes.keys() )
                        {
                            protocol::ProtocolDocumentOperation_Component_KeyValuePair* pair = comp->mutable_element_start()->add_attribute();
                            pair->set_key( key.toStdString() );
                            pair->set_value( (*it).attributes[key].toStdString() );
                        }
                        break;
                    case DocumentMutation::DeleteEnd:
                        comp->set_delete_element_end(true);
                        break;
                    case DocumentMutation::DeleteChars:
                        comp->set_delete_characters( (*it).text.toStdString() );
                        break;
                    case DocumentMutation::AnnotationBoundary:
                            foreach( QString key, (*it).annotations.keys() )
                            {
                                protocol::ProtocolDocumentOperation_Component_KeyValueUpdate* pair = comp->mutable_annotation_boundary()->add_change();
                                pair->set_key( key.toStdString() );
                                QString oldvalue = (*it).annotations[key].first;
                                if ( !oldvalue.isNull() )
                                    pair->set_old_value( oldvalue.toStdString() );
                                QString newvalue = (*it).annotations[key].second;
                                if ( !newvalue.isNull() )
                                    pair->set_new_value( newvalue.toStdString() );
                            }
                            foreach( QString ek, (*it).endKeys)
                            {
                                comp->mutable_annotation_boundary()->add_end( ek.toStdString() );
                            }
                        break;
                    case DocumentMutation::ReplaceAttributes:
                        {
                            protocol::ProtocolDocumentOperation_Component_ReplaceAttributes* repl = comp->mutable_replace_attributes();
                            foreach( QString key, (*it).attributes.keys() )
                            {
                                if ( key[0] == '-' )
                                {
                                    protocol::ProtocolDocumentOperation_Component_KeyValuePair* pair = repl->add_old_attribute();
                                    pair->set_key( key.mid(1).toStdString() );
                                    pair->set_value( (*it).attributes[key].toStdString() );
                                }
                                else
                                {
                                    protocol::ProtocolDocumentOperation_Component_KeyValuePair* pair = repl->add_new_attribute();
                                    pair->set_key( key.mid(1).toStdString() );
                                    pair->set_value( (*it).attributes[key].toStdString() );
                                }
                            }
                        }
                        break;
                    case DocumentMutation::UpdateAttributes:
                        {
                            protocol::ProtocolDocumentOperation_Component_UpdateAttributes* update = comp->mutable_update_attributes();
                            foreach( QString key, (*it).attributes.keys() )
                            {
                                if ( key[0] == '-' )
                                    continue;
                                protocol::ProtocolDocumentOperation_Component_KeyValueUpdate* kv = update->add_attribute_update();
                                kv->set_key( key.toStdString() );
                                kv->set_new_value( (*it).attributes[key].toStdString() );
                                kv->set_old_value( (*it).attributes["-" + key].toStdString() );
                            }
                        }
                        break;
                }
            }
        }
    }
}

/**
  * Convert the protocol buffer representation to the Qt-ified representation.
  */
WaveletDelta Converter::convert( const protocol::ProtocolWaveletDelta& delta )
{
    WaveletDelta result;
    result.setAuthor( QString::fromStdString(delta.author()));
    result.version().version = delta.hashed_version().version();
    std::string hash = delta.hashed_version().history_hash();
    result.version().hash = QByteArray( hash.data(), hash.length() );

    for( int o = 0; o < delta.operation_size(); ++o )
    {
        const protocol::ProtocolWaveletOperation op = delta.operation(o);
        if ( op.has_add_participant() )
        {
            WaveletDeltaOperation wo;
            wo.setAddParticipant( QString::fromStdString(op.add_participant()));
            result.addOperation(wo);
        }
        if ( op.has_remove_participant() )
        {
            WaveletDeltaOperation wo;
            wo.setRemoveParticipant( QString::fromStdString(op.remove_participant()));
            result.addOperation(wo);
        }
        if ( op.has_mutate_document() )
        {
            DocumentMutation m;
            const protocol::ProtocolWaveletOperation_MutateDocument& mut = op.mutate_document();
            const protocol::ProtocolDocumentOperation& dop = mut.document_operation();
            for( int c = 0; c < dop.component_size(); ++c )
            {
                const protocol::ProtocolDocumentOperation_Component& comp = dop.component(c);
                if ( comp.has_characters() )
                {
                    QString chars = QString::fromStdString(comp.characters());
                    m.insertChars(chars);
                }
                else if ( comp.has_retain_item_count() )
                {
                    int retain = comp.retain_item_count();
                    m.retain(retain);
                }
                else if ( comp.has_delete_characters() )
                {
                    QString chars = QString::fromStdString(comp.delete_characters());
                    m.deleteChars(chars);
                }
                else if ( comp.has_element_end() )
                {
                    m.insertEnd();
                }
                else if ( comp.has_element_start() )
                {
                    QString type = QString::fromStdString(comp.element_start().type());
                    StructuredDocument::AttributeList attribs;
                    for( int a = 0; a < comp.element_start().attribute_size(); ++a )
                        attribs[ QString::fromStdString(comp.element_start().attribute(a).key()) ] = QString::fromStdString(comp.element_start().attribute(a).value() );
                    m.insertStart(type, attribs );
                }
                else if ( comp.has_annotation_boundary() )
                {
                    QList<QString> endKeys;
                    StructuredDocument::AnnotationChange changes;
                    for( int e = 0; e < comp.annotation_boundary().end_size(); ++e )
                        endKeys.append( QString::fromStdString( comp.annotation_boundary().end(e) ) );
                    for( int a = 0; a < comp.annotation_boundary().change_size(); ++a )
                    {
                        if ( comp.annotation_boundary().change(a).has_old_value() )
                            changes[ QString::fromStdString(comp.annotation_boundary().change(a).key()) ].first = QString::fromStdString(comp.annotation_boundary().change(a).old_value() );
                        else
                            changes[ QString::fromStdString(comp.annotation_boundary().change(a).key()) ].first = QString::null;
                        if ( comp.annotation_boundary().change(a).has_new_value() )
                            changes[ QString::fromStdString(comp.annotation_boundary().change(a).key()) ].second = QString::fromStdString(comp.annotation_boundary().change(a).new_value() );
                        else
                            changes[ QString::fromStdString(comp.annotation_boundary().change(a).key()) ].second = QString::null;
                    }
                    m.annotationBoundary(endKeys, changes);
                }
                else if ( comp.delete_element_end() )
                {
                    m.deleteEnd();
                }
                else if ( comp.has_delete_element_start() )
                {
                    StructuredDocument::AttributeList attribs;
                    for( int a = 0; a < comp.delete_element_start().attribute_size(); ++a )
                        attribs[ QString::fromStdString(comp.delete_element_start().attribute(a).key()) ] = QString::fromStdString(comp.delete_element_start().attribute(a).value() );
                    m.deleteStart( QString::fromStdString(comp.delete_element_start().type()), attribs );
                }
                else if ( comp.has_replace_attributes() )
                {
                    StructuredDocument::AttributeList oldAttribs;
                    StructuredDocument::AttributeList newAttribs;
                    for( int a = 0; a < comp.replace_attributes().old_attribute_size(); ++a )
                        oldAttribs[ QString::fromStdString( comp.replace_attributes().old_attribute(a).key() ) ] = QString::fromStdString(comp.replace_attributes().old_attribute(a).value() );
                    for( int a = 0; a < comp.replace_attributes().new_attribute_size(); ++a )
                        newAttribs[ QString::fromStdString( comp.replace_attributes().new_attribute(a).key() ) ] = QString::fromStdString(comp.replace_attributes().new_attribute(a).value() );
                    m.replaceAttributes( oldAttribs, newAttribs );
                }
                else if ( comp.has_update_attributes() )
                {
                    QHash<QString,StructuredDocument::StringPair> updates;
                    for( int a = 0; a < comp.update_attributes().attribute_update_size(); ++a )
                    {
                        QString key = QString::fromStdString( comp.update_attributes().attribute_update(a).key() );
                        StructuredDocument::StringPair pair;
                        if ( comp.update_attributes().attribute_update(a).has_new_value() )
                            pair.second = QString::fromStdString(comp.update_attributes().attribute_update(a).new_value() );
                        else
                            pair.second = QString::null;
                        if ( comp.update_attributes().attribute_update(a).has_old_value() )
                            pair.first = QString::fromStdString(comp.update_attributes().attribute_update(a).old_value() );
                        else
                            pair.first = QString::null;
                        updates[key] = pair;
                    }
                    m.updateAttributes(updates);
                }
            }
            WaveletDeltaOperation wo;
            wo.setMutation(m);
            wo.setDocumentId( QString::fromStdString( mut.document_id() ) );
            result.addOperation(wo);
        }
    }
    return result;
}
