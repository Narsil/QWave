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

    h << "#ifndef " << ident(file->name()) << endl;
    h << "#define " << ident(file->name()) << endl << endl;
    h << "#include <QByteArray>"  << endl << endl;

    if ( !file->package().empty() )
        // TODO: replace . with ::
        h << "namespace " << nspace(file->package()) << endl << "{" << endl;
    for( int i = 0; i < file->message_type_count(); ++i )
    {
        bool ok = GenerateMessageType( file->message_type(i), nspace(file->package()), "", cpp, h, error );
        if ( !ok )
            return false;
    }

    if ( !file->package().empty() )
        h << "}" << endl;

    h << "#endif " << file->name() << endl;

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

bool CppJSONGenerator::GenerateMessageType( const Descriptor* descriptor, const string& nspace, const string& prefix, ostream& cpp, ostream& h, std::string* error) const
{
    h << "class " << prefix << ident(descriptor->name()) + "_JSON : public ::JSONMessage" << endl << "{" << endl;
    h << "public:" << endl;
    h << "\tstatic bool SerializeToArray(const " << prefix << ident(descriptor->name()) << "* msg, QByteArray& data);" << endl;
    h << "\tstatic bool ParseFromArray(" << prefix << ident(descriptor->name()) << "* msg, const QByteArray& data);" << endl;
    h << "private:" << endl;
    h << "\t" << ident(descriptor->name()) << "_JSON() { }" << endl;
    h << "};" << endl << endl;

    cpp << "bool " << nspace << "::" << prefix << ident(descriptor->name()) << "_JSON::SerializeToArray(const " << prefix << ident(descriptor->name()) << "* msg, QByteArray& data)" << endl;
    cpp << "{" << endl;
    cpp << "\tdata.append( \"{\" );" << endl;

    for( int i = 0; i < descriptor->field_count(); ++i )
    {        
        const FieldDescriptor* field = descriptor->field(i);
        if ( field->is_required() || field->is_optional() )
        {
            if ( field->is_optional() )
                cpp << "\tif ( msg->has_" << ident(field->name()) << "() ) {" << endl;
            switch( field->cpp_type() )
            {
            case FieldDescriptor::CPPTYPE_INT32:
            case FieldDescriptor::CPPTYPE_INT64:
            case FieldDescriptor::CPPTYPE_UINT32:
            case FieldDescriptor::CPPTYPE_UINT64:
            case FieldDescriptor::CPPTYPE_DOUBLE:
            case FieldDescriptor::CPPTYPE_FLOAT:
                if ( i == 0 )
                    cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":\" );" << endl;
                else
                    cpp << "\tdata.append( \",\\\"" << field->number() << "\\\":\" );" << endl;
                cpp << "\tdata.append( QByteArray::number( msg->" << ident(field->name()) << "() );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_BOOL:
                if ( i == 0 )
                {
                    cpp << "\tif( msg->" << ident(field->name()) << "() )" << endl;
                    cpp << "\t\tdata.append( \"\\\"" << field->number() << "\\\":\\\"true\\\"\" );" << endl;
                    cpp << "\telse" << endl;
                    cpp << "\t\tdata.append( \"\\\"" << field->number() << "\\\":\\\"false\\\"\" );" << endl;
                }
                else
                {
                    cpp << "\tif( msg->" << ident(field->name()) << ")" << endl;
                    cpp << "\t\tdata.append( \",\\\"" << field->number() << "\\\":\\\"true\\\"\" );" << endl;
                    cpp << "\telse" << endl;
                    cpp << "\t\tdata.append( \",\\\"" << field->number() << "\\\":\\\"false\\\"\" );" << endl;
                }
            case FieldDescriptor::CPPTYPE_ENUM:
            case FieldDescriptor::CPPTYPE_STRING:
                if ( i == 0 )
                    cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":\" );" << endl;
                else
                    cpp << "\tdata.append( \",\\\"" << field->number() << "\\\":\" );" << endl;
                cpp << "\tdata.append( toJSONString( msg->" << ident(field->name()) << "() ) );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                if ( i == 0 )
                    cpp << "\tdata.append( \"\\\"" << field->number() << "\\\":\" );" << endl;
                else
                    cpp << "\tdata.append( \",\\\"" << field->number() << "\\\":\" );" << endl;
                cpp << "\tif ( !" << absIdent(field->message_type()) << "_JSON::SerializeToArray( &msg->" << ident(field->name()) << "(), data ) ) return false;" << endl;
                break;
            default:
                error->append("Unknown type");
                return false;
            }
            if ( field->is_optional() )
                cpp << "\t}" << endl;
        }
    }
    cpp << "\tdata.append( \"}\" );" << endl;

    cpp << "}" << endl << endl;

    cpp << "bool " << nspace << "::" << prefix << ident(descriptor->name()) << "_JSON::ParseFromArray(" << prefix << ident(descriptor->name()) << "* msg, const QByteArray& data)" << endl;
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
        bool ok = this->GenerateMessageType( nested, nspace, ident(descriptor->name()) + "_", cpp, h, error );
        if ( !ok )
            return false;
    }

    return true;
}

string CppJSONGenerator::absIdent(const Descriptor* descriptor ) const
{
    if ( descriptor->containing_type() == 0 )
        return nspace( descriptor->file()->package() ) + "::" + ident(descriptor->name() );
    return absIdent( descriptor->containing_type() ) + "_" + ident(descriptor->name() );
}

//// TODO: Proper escaping. This is a hack
//string CppJSONGenerator::escape(const string& str ) const
//{
//    return "\"" + str + "\"";
//}
