#include "cppjsongenerator.h"
#include <google/protobuf/io/coded_stream.h>

#include <sstream>
CppJSONGenerator::CppJSONGenerator()
{
}

bool CppJSONGenerator::Generate( const FileDescriptor* file, const std::string& parameter, OutputDirectory* output_directory, std::string* error) const
{
    ZeroCopyOutputStream* cppOut;
    ZeroCopyOutputStream* hOut;
    cppOut = output_directory->Open( file->name().substr(0, file->name().length() - 6) + ".pbjson.cpp" );
    hOut = output_directory->Open( file->name().substr(0, file->name().length() - 6) + ".pbjson.h" );

    ostringstream cpp( ostringstream::out );
    ostringstream h( ostringstream::out );

    cpp << "#include \"" << file->name().substr(0, file->name().length() - 6) << ".pbjson.h\"" << endl << endl;

    string label = ident(file->name());
    for( string::size_type i = 0; i < label.length(); ++i )
        if ( label[i] == '.' )
            label[i] = '_';
    h << "#ifndef " <<  label << endl;
    h << "#define " << label << endl << endl;
    h << "#include <QByteArray>"  << endl;
    h << "#include \"jsonmessage.h\""  << endl;
    h << "#include \"" << file->name().substr(0, file->name().length() - 6) << ".pb.h\"" << endl << endl;

    if ( !file->package().empty() )
        // TODO: replace . with ::
        h << "namespace " << nspace(file->package()) << endl << "{" << endl;
    for( int i = 0; i < file->message_type_count(); ++i )
    {
        bool ok = GenerateMessageType( file->message_type(i), nspace(file->package()), cpp, h, error );
        if ( !ok )
            return false;
    }

    if ( !file->package().empty() )
        h << "}" << endl;

    h << "#endif " << endl;

    CodedOutputStream* tmp = new CodedOutputStream( hOut );
    tmp->WriteString( h.str() );
    delete tmp;

    tmp = new CodedOutputStream( cppOut );
    tmp->WriteString( cpp.str() );
    delete tmp;

    delete cppOut;
    delete hOut;
    return true;
}

bool CppJSONGenerator::GenerateMessageType( const Descriptor* descriptor, const string& nspace, ostream& cpp, ostream& h, std::string* error) const
{
    h << "class " << absIdent(descriptor) + "_JSON : public ::JSONMessage" << endl << "{" << endl;
    h << "public:" << endl;
    h << "\tstatic bool SerializeToArray(const " << absIdent(descriptor) << "* msg, QByteArray& data);" << endl;
    h << "\tstatic bool ParseFromArray(" << absIdent(descriptor) << "* msg, const QByteArray& data);" << endl;
    h << "private:" << endl;
    h << "\t" << absIdent(descriptor) << "_JSON() { }" << endl;
    h << "};" << endl << endl;

    cpp << "bool " << nspace << "::" << absIdent(descriptor) << "_JSON::SerializeToArray(const " << absIdent(descriptor) << "* msg, QByteArray& data)" << endl;
    cpp << "{" << endl;
    cpp << "\tdata.append( \"{\" );" << endl;
    if ( descriptor->field_count() > 0 )
        cpp << "\tint count = 0;" << endl;
    for( int i = 0; i < descriptor->field_count(); ++i )
    {        
        const FieldDescriptor* field = descriptor->field(i);
        if ( field->is_required() || field->is_optional() )
        {
            if ( field->is_optional() )
                cpp << "\tif ( msg->has_" << ident(field->name()) << "() ) {" << endl;
            cpp << "\tif ( count++ > 0 ) data.append(\",\");" << endl;
            switch( field->cpp_type() )
            {
            case FieldDescriptor::CPPTYPE_INT32:
            case FieldDescriptor::CPPTYPE_INT64:
            case FieldDescriptor::CPPTYPE_UINT32:
            case FieldDescriptor::CPPTYPE_UINT64:
            case FieldDescriptor::CPPTYPE_DOUBLE:
            case FieldDescriptor::CPPTYPE_FLOAT:
                cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":\" );" << endl;
                cpp << "\tdata.append( QByteArray::number( msg->" << ident(field->name()) << "() ) );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_BOOL:
                cpp << "\tif( msg->" << ident(field->name()) << "() )" << endl;
                cpp << "\t\tdata.append( \"\\\"" << field->number() << "\\\":\\\"true\\\"\" );" << endl;
                cpp << "\telse" << endl;
                cpp << "\t\tdata.append( \"\\\"" << field->number() << "\\\":\\\"false\\\"\" );" << endl;
            case FieldDescriptor::CPPTYPE_ENUM:
                cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":\" );" << endl;
                cpp << "\tdata.append( QByteArray::number( (int)msg->" << ident(field->name()) << "() ) );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_STRING:
                if ( field->type() == FieldDescriptor::TYPE_BYTES )
                {
                    cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":[\" );" << endl;
                    cpp << "\tfor( ::std::string::size_type b = 0; b < msg->" << ident(field->name()) << "().length(); ++b )" << endl << "\t{" << endl;
                    cpp << "\t\tif ( b > 0 ) data.append( \",\" );" << endl;
                    cpp << "\t\tdata.append( QByteArray::number( (int)msg->" << ident(field->name()) << "()[b] ) );" << endl;
                    cpp << "\t}" << endl;
                    cpp << "\tdata.append( \"]\" );" << endl;
                }
                else
                {
                    cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":\" );" << endl;
                    cpp << "\tdata.append( toJSONString( msg->" << ident(field->name()) << "() ) );" << endl;
                }
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":\" );" << endl;
                cpp << "\tif ( !" << absIdent(field->message_type()) << "_JSON::SerializeToArray( &msg->" << ident(field->name()) << "(), data ) ) return false;" << endl;
                break;
            default:
                error->append("Unknown type");
                return false;
            }
            if ( field->is_optional() )
                cpp << "\t}" << endl;
        }
        else if ( field->is_repeated() )
        {
            cpp << "\tif ( count++ > 0 ) data.append(\",\");" << endl;
            cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":[\" );" << endl;
            cpp << "\tfor( int i = 0; i < msg->" << ident(field->name()) << "_size(); ++i )" << endl << "\t{" << endl;
            cpp << "\t\tif ( i > 0 ) data.append( \",\" );" << endl;

            switch( field->cpp_type() )
            {
            case FieldDescriptor::CPPTYPE_INT32:
            case FieldDescriptor::CPPTYPE_INT64:
            case FieldDescriptor::CPPTYPE_UINT32:
            case FieldDescriptor::CPPTYPE_UINT64:
            case FieldDescriptor::CPPTYPE_DOUBLE:
            case FieldDescriptor::CPPTYPE_FLOAT:
                cpp << "\t\tdata.append( QByteArray::number( msg->" << ident(field->name()) << "(i) ) );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_BOOL:
                cpp << "\t\tif( msg->" << ident(field->name()) << "(i) )" << endl;
                cpp << "\t\t\tdata.append( \"\\\"true\\\"\" );" << endl;
                cpp << "\t\telse" << endl;
                cpp << "\t\t\tdata.append( \"\\\"false\\\"\" );" << endl;
            case FieldDescriptor::CPPTYPE_ENUM:
                cpp << "\t\tdata.append( QByteArray::number( (int)msg->" << ident(field->name()) << "(i) ) );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_STRING:
                if ( field->type() == FieldDescriptor::TYPE_BYTES )
                {
                    cpp << "\tdata.append( \"[\" );" << endl;
                    cpp << "\tfor( ::std::string::size_type b = 0; b < msg->" << ident(field->name()) << "(i).length(); ++b )" << endl << "\t{" << endl;
                    cpp << "\t\tif ( b > 0 ) data.append( \",\" );" << endl;
                    cpp << "\t\tdata.append( QByteArray::number( (int)msg->" << ident(field->name()) << "(i)[b] ) );" << endl;
                    cpp << "\t}" << endl;
                    cpp << "\tdata.append( \"]\" );" << endl;
                }
                else
                    cpp << "\t\tdata.append( toJSONString( msg->" << ident(field->name()) << "(i) ) );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                cpp << "\t\tif ( !" << absIdent(field->message_type()) << "_JSON::SerializeToArray( &msg->" << ident(field->name()) << "(i), data ) ) return false;" << endl;
                break;
            default:
                error->append("Unknown type");
                return false;
            }

            cpp << "\t}" << endl << "\tdata.append( \"]\" );" << endl;
        }
    }
    cpp << "\tdata.append( \"}\" );" << endl;
    cpp << "\treturn true;" << endl << endl;
    cpp << "}" << endl << endl;

    cpp << "bool " << nspace << "::" << absIdent(descriptor) << "_JSON::ParseFromArray(" << absIdent(descriptor) << "* msg, const QByteArray& data)" << endl;
    cpp << "{" << endl;

        for( int i = 0; i < descriptor->field_count(); ++i )
    {
        const FieldDescriptor* field = descriptor->field(i);
        switch( field->cpp_type() )
        {
        case FieldDescriptor::CPPTYPE_INT32:
        case FieldDescriptor::CPPTYPE_INT64:
        case FieldDescriptor::CPPTYPE_UINT32:
        case FieldDescriptor::CPPTYPE_UINT64:
        case FieldDescriptor::CPPTYPE_DOUBLE:
        case FieldDescriptor::CPPTYPE_FLOAT:
        case FieldDescriptor::CPPTYPE_BOOL:
        case FieldDescriptor::CPPTYPE_ENUM:
        case FieldDescriptor::CPPTYPE_STRING:
        case FieldDescriptor::CPPTYPE_MESSAGE:
            break;
        default:
            error->append("Unknown type");
            return false;
        }
    }

    cpp << "}" << endl << endl;

    for( int i = 0; i < descriptor->nested_type_count(); ++i )
    {
        const Descriptor* nested = descriptor->nested_type(i);
        bool ok = this->GenerateMessageType( nested, nspace, cpp, h, error );
        if ( !ok )
            return false;
    }

    return true;
}

string CppJSONGenerator::absIdent(const Descriptor* descriptor, bool with_namespace ) const
{
    if ( descriptor->containing_type() == 0 )
    {
        if ( with_namespace )
            return nspace( descriptor->file()->package() ) + "::" + ident(descriptor->name() );
        return ident(descriptor->name() );
    }
    return absIdent( descriptor->containing_type(), with_namespace ) + "_" + ident(descriptor->name() );
}
