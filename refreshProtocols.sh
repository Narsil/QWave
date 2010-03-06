#!/bin/sh
cd tools/protojs && qmake && make
cd ../../core/protocol && ../../tools/protojs/protojs *.proto --cpp_out=. --cppjson_out=. --jsjson_out=../../webclient/javascript
cd ../../waveserver/protocol && protoc messages.proto -I. -I../../core/protocol --cpp_out . && ../../tools/protojs/protojs webclient.proto -I. -I../../core/protocol --cpp_out=. --cppjson_out=. --jsjson_out=../../webclient/javascript
