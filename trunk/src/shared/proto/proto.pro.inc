#This file will build the protocol buffer code

message(Compiling Protocol Buffer CPP Sources from .proto Files...)
system(protoc --cpp_out=$$PWD/cpp --proto_path=$$PWD $$PWD/*.proto):HAS_PROTOBUF = TRUE

isEmpty(HAS_PROTOBUF) {
  warning(Is the google protocol buffer compiler 'protoc' installed?)
}
