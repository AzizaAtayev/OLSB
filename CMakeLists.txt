build_lib(
  LIBNAME olsb
  SOURCE_FILES
    helper/olsb-helper.cc
    model/olsb-packet-queue.cc
    model/olsb-packet.cc
    model/olsb-routing-protocol.cc
    model/olsb-rtable.cc
  HEADER_FILES
    helper/olsb-helper.h
    model/olsb-packet-queue.h
    model/olsb-packet.h
    model/olsb-routing-protocol.h
    model/olsb-rtable.h
  LIBRARIES_TO_LINK ${libinternet}
  TEST_SOURCES test/olsb-testcase.cc
)
