set(protobuf_MODULE_COMPATIBLE TRUE)
find_package(Protobuf CONFIG QUIET)
if(NOT Protobuf_FOUND)
  find_package(Protobuf REQUIRED)
endif()
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS chaoskit.proto)

add_library(ui_storage_proto ${PROTO_HDRS} ${PROTO_SRCS})
target_include_directories(ui_storage_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ui_storage_proto PUBLIC protobuf::libprotobuf)

add_library(ui_storage conversions.cpp conversions.h)
target_link_libraries(
  ui_storage
  PRIVATE library magic_enum
  PUBLIC core_interface ui_storage_proto)

add_executable(ui_storage_test conversions_test.cpp)
target_link_libraries(ui_storage_test PRIVATE core gmock gmock_main library
                                              ui_storage)
add_test(NAME ui_storage_test COMMAND ui_storage_test)
