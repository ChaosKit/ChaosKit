find_package(Protobuf REQUIRED)
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS chaoskit.proto)

add_library(io_proto ${PROTO_HDRS} ${PROTO_SRCS})
target_include_directories(io_proto
        PUBLIC ${Protobuf_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(io_proto PUBLIC ${Protobuf_LIBRARIES})

add_library(io io.h io.cpp)
target_link_libraries(io
        PRIVATE io_proto library magic_enum
        PUBLIC core)
