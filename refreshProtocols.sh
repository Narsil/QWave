#!/bin/sh
cd tools/protojs && make
cd ../../core/protocol && ../../tools/protojs/protojs *.proto --cpp_out=. --cppjson_out=.
cd ../../waveserver/protocol && protoc *.proto -I. -I../../core/protocol --cpp_out .
