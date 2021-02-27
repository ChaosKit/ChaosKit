find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS chaoskit.proto)

add_library(ui_storage_proto ${PROTO_HDRS} ${PROTO_SRCS})
target_include_directories(ui_storage_proto PUBLIC ${Protobuf_INCLUDE_DIR}
                                                   ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(ui_storage_proto PUBLIC ${Protobuf_LIBRARIES})

add_library(ui_storage storage.cpp storage.h)
target_link_libraries(
  ui_storage
  PRIVATE ui_storage_proto library magic_enum
  PUBLIC flame)
