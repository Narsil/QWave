#ifndef CPPJSONGENERATOR_H
#define CPPJSONGENERATOR_H

#include <google/protobuf/compiler/code_generator.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <string>

using namespace google::protobuf::compiler;
using namespace google::protobuf;
using namespace google::protobuf::io;
using namespace std;

class CppJSONGenerator : public CodeGenerator
{
public:
    CppJSONGenerator();

    virtual bool Generate( const FileDescriptor* file, const std::string& parameter, OutputDirectory* output_directory, std::string* error) const;

private:
    bool GenerateMessageType( const Descriptor* descriptor, const string& nspace, const string& prefix, ostream& cpp, ostream& h, std::string* error) const;
    string ident(const string& str ) const { return str; }
    string nspace(const string& str ) const { return str; }
    string absIdent(const Descriptor* descriptor ) const;
//    string escape(const string& str ) const;
};

#endif // CPPJSONGENERATOR_H
