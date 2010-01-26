#!/bin/sh
cd core/protocol && protoc *.proto --cpp_out .
cd ../../waveserver/protocol && protoc *.proto -I. -I../../core/protocol --cpp_out .
