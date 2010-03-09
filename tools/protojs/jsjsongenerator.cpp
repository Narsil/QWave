#include "jsjsongenerator.h"
#include <google/protobuf/io/coded_stream.h>

#include <sstream>
#include <ctype.h>

JsJSONGenerator::JsJSONGenerator()
{
}

bool JsJSONGenerator::Generate( const FileDescriptor* file, const std::string&, OutputDirectory* output_directory, std::string* error) const
{
    ZeroCopyOutputStream* jsOut;
    jsOut = output_directory->Open( file->name().substr(0, file->name().length() - 6) + ".pbjson.js" );

    ostringstream js( ostringstream::out );

    js << "if (!window." << nspace(file->package()) << ") " << nspace(file->package()) << " = {};" << endl;

    for( int i = 0; i < file->message_type_count(); ++i )
    {
        bool ok = GenerateMessageType( file->message_type(i), nspace(file->package()), js, error );
        if ( !ok )
            return false;
    }

    CodedOutputStream* tmp = new CodedOutputStream( jsOut );
    tmp->WriteString( js.str() );
    delete tmp;

    delete jsOut;
    return true;
}

bool JsJSONGenerator::GenerateMessageType( const Descriptor* descriptor, const string& nspace, ostream& js, std::string* error) const
{
    /////////////////////////////////
    // Class & Constructor
    /////////////////////////////////

    js << absIdent(descriptor, true) << " = function() { ";
    for( int i = 0; i < descriptor->field_count(); ++i )
    {
        const FieldDescriptor* field = descriptor->field(i);
        if ( field->is_repeated() )
            js << "this." << ident(field->name()) << " = []; ";
    }
    js << " };" << endl << endl;

    /////////////////////////////////
    // has_xxxx()
    /////////////////////////////////

    for( int i = 0; i < descriptor->field_count(); ++i )
    {
        const FieldDescriptor* field = descriptor->field(i);
        if ( field->is_optional() )
        {
            js << absIdent(descriptor, true) << ".prototype.has_" << ident(field->name()) << " = function() { ";
            js << "return ( this." << ident(field->name()) << " != null );";
            js << " };" << endl << endl;
        }
    }

    /////////////////////////////////
    // Enums
    /////////////////////////////////

    for( int i = 0; i < descriptor->enum_type_count(); ++i )
    {
        const EnumDescriptor* e = descriptor->enum_type(i);
        js << absIdent(descriptor, true) << "." << ident(e->name()) << " = {";
        for( int v = 0; v < e->value_count(); ++v )
        {
            if ( v != 0 )
                js << ",";
            const EnumValueDescriptor* ev = e->value(v);
            js << ident(ev->name()) << ":" << ev->number();
        }
        js << "};" << endl << endl;
    }

    /////////////////////////////////
    // serialize()
    /////////////////////////////////

    js << absIdent(descriptor, true) << ".prototype.serialize = function()" << endl << "{" << endl;
    js << "\tvar data = {};" << endl;
    for( int i = 0; i < descriptor->field_count(); ++i )
    {        
        const FieldDescriptor* field = descriptor->field(i);
        if ( field->is_required() || field->is_optional() )
        {
            if ( field->is_optional() )
                js << "\tif ( this.has_" << ident(field->name()) << "() ) {" << endl;
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
                js << "\tdata[\"" << field->number() << "\"] = this." << ident(field->name()) << ";" << endl;
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                js << "\tdata[\"" << field->number() << "\"] = this." << ident(field->name()) << ".serialize();" << endl;
                break;
            default:
                error->append("Unknown type");
                return false;
            }
            if ( field->is_optional() )
                js << "\t}" << endl;
        }
        else if ( field->is_repeated() )
        {
            js << "\tvar arr_" << field->number() << " = [];" << endl;
            js << "\tfor( var i = 0; i < this." << ident(field->name()) << ".length; ++i )" << endl << "\t{" << endl;

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
                js << "\t\tarr_" << field->number() << ".push( this." << ident(field->name()) << "[i] );" << endl;
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                js << "\t\tarr_" << field->number() << ".push( this." << ident(field->name()) << "[i].serialize() );" << endl;
                break;
            default:
                error->append("Unknown type");
                return false;
            }
            js << "\t}" << endl;
            js << "\tdata[\"" << field->number() << "\"] = arr_" << field->number() << ";" << endl;
        }
    }
    js << "\treturn data;" << endl;
    js << "};" << endl << endl;

    /////////////////////////////////
    // parse()
    /////////////////////////////////

    js << absIdent(descriptor, true) << ".parse = function(data)" << endl << "{" << endl;
    js << "\tif ( !data ) return null;" << endl;
    js << "\tvar obj = new " << absIdent(descriptor, true) << "();" << endl;

    for( int i = 0; i < descriptor->field_count(); ++i )
    {
        const FieldDescriptor* field = descriptor->field(i);

        if ( field->is_repeated() )
        {
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
                js << "\tobj."<< ident(field->name()) << " = data[\"" << field->number() << "\"];" << endl;
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                js << "\tvar arr = [];" << endl;
                js << "\tvar d = data[\"" << field->number() << "\"];" << endl;
                js << "\tfor( var i = 0; i < d.length; ++i )" << endl << "\t{" << endl;
                js << "\t\tarr.push( " << absIdent(field->message_type(), true) << ".parse( d[i] ) );" << endl;
                js << "\t}" << endl;
                js << "\tobj."<< ident(field->name()) << " = arr;" << endl;
                break;
            default:
                error->append("Unknown type");
                return false;
            }
        }
        else
        {
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
                js << "\tobj."<< ident(field->name()) << " = data[\"" << field->number() << "\"];" << endl;
                break;
            case FieldDescriptor::CPPTYPE_MESSAGE:
                js << "\tobj."<< ident(field->name()) << " = " << absIdent(field->message_type(), true) << ".parse( data[\"" << field->number() << "\"] );" << endl;
                break;
            default:
                error->append("Unknown type");
                return false;
            }
        }
    }

    for( int i = 0; i < descriptor->field_count(); ++i )
    {
        const FieldDescriptor* field = descriptor->field(i);
        if ( field->is_required() )
            js << "\tif ( !obj." << ident(field->name()) << " == null ) throw \"Missing field " << ident(field->name()) << "\";" << endl;
    }

    js << "\treturn obj;" << endl;
    js << "};" << endl << endl;

    for( int i = 0; i < descriptor->nested_type_count(); ++i )
    {
        const Descriptor* nested = descriptor->nested_type(i);
        bool ok = this->GenerateMessageType( nested, nspace, js, error );
        if ( !ok )
            return false;
    }

    return true;
}

string JsJSONGenerator::absIdent(const Descriptor* descriptor, bool with_namespace ) const
{
    if ( descriptor->containing_type() == 0 )
    {
        if ( with_namespace )
            return nspace( descriptor->file()->package() ) + "." + ident(descriptor->name() );
        return ident(descriptor->name() );
    }
    return absIdent( descriptor->containing_type(), with_namespace ) + "_" + ident(descriptor->name() );
}

string JsJSONGenerator::absIdent(const EnumDescriptor* descriptor, bool with_namespace ) const
{
    if ( descriptor->containing_type() == 0 )
    {
        if ( with_namespace )
            return nspace( descriptor->file()->package() ) + "." + ident(descriptor->name() );
        return ident(descriptor->name() );
    }
    return absIdent( descriptor->containing_type(), with_namespace ) + "_" + ident(descriptor->name() );
}
