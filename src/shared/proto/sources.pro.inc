#messages defined through google protocol buffers (as compiled by protoc)

include( $${SHARED_DIR}/proto/proto.pro.inc )

PREFIX = $${SHARED_DIR}/proto/cpp

INCLUDEPATH += $${PREFIX}

LIST = $$system(ls $${PWD}/cpp/*.pb.cc)
for(item, LIST) {
  SOURCES += $${item}
}

LIST = $$system(ls $${PWD}/cpp/*.pb.h)
for(item, LIST) {
  HEADERS += $${item}
}

