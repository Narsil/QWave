#include <QtCore/QCoreApplication>
#include <google/protobuf/compiler/command_line_interface.h>
#include <google/protobuf/compiler/cpp/cpp_generator.h>

#include "cppjsongenerator.h"

using namespace google::protobuf::compiler;

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    CommandLineInterface cli;

    // Support generation of C++ source and headers.
    google::protobuf::compiler::cpp::CppGenerator cpp_generator;
    cli.RegisterGenerator("--cpp_out", &cpp_generator, "Generate C++ source and header.");

    // Support generation of C++ source and headers.
    CppJSONGenerator cppjson_generator;
    cli.RegisterGenerator("--cppjson_out", &cppjson_generator,
                          "Generate C++ source and header for the JSON format.");

    return cli.Run(argc, argv);
}
