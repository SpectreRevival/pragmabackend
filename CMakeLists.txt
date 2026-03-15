cmake_minimum_required(VERSION 4.0)
project(pragmabackend_core)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(WIN32)
    add_compile_definitions(_WIN32_WINNT=0x0A00) # target at least win10
    add_compile_definitions(_WIN32=1)
endif()
if(UNIX)
    add_compile_definitions(__linux__=1)
endif ()
add_compile_definitions(FMT_UNICODE=0)

file(GLOB_RECURSE RESOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/resources/*")

add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/resources ${CMAKE_CURRENT_BINARY_DIR}/resources
    COMMENT "Copying resources directory to binary dir"
)

add_custom_target(copy_auth ALL
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/auth.json
        ${CMAKE_CURRENT_BINARY_DIR}/auth.json
    COMMENT "copying auth.json to binary dir"
)

add_executable(pragmabackend main.cpp)
add_library(pragmabackend_core)
target_link_libraries(pragmabackend PUBLIC pragmabackend_core)
add_dependencies(pragmabackend_core copy_resources copy_auth)

find_package(Boost REQUIRED COMPONENTS beast bimap uuid)
find_package(boost_asio CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(SQLiteCpp CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_program(PROTOC_COMPILER protoc REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
find_package(stduuid CONFIG REQUIRED)
target_compile_definitions(pragmabackend_core PUBLIC BOOST_ERROR_CODE_HEADER_ONLY)
file(GLOB_RECURSE packetimplementors CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Packets/Implementation/*.cpp")
target_sources(pragmabackend_core PRIVATE ${packetimplementors})
file(GLOB_RECURSE persistenceimplementors CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Persistence/Implementation/*.cpp")
target_sources(pragmabackend_core PRIVATE ${persistenceimplementors})
target_link_libraries(pragmabackend_core PUBLIC Boost::beast Boost::bimap Boost::asio Boost::uuid spdlog::spdlog_header_only)
target_link_libraries(pragmabackend_core PUBLIC OpenSSL::SSL OpenSSL::Crypto)
target_link_libraries(pragmabackend_core PUBLIC SQLiteCpp)
target_link_libraries(pragmabackend_core PUBLIC protobuf::libprotoc protobuf::libprotobuf)
target_link_libraries(pragmabackend_core PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(pragmabackend_core PUBLIC stduuid)
target_include_directories(pragmabackend_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Packets/Model")
target_include_directories(pragmabackend_core PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Persistence/Model")

# Define import roots for protoc
set(PROTOBUF_IMPORT_DIRS
    "${CMAKE_CURRENT_SOURCE_DIR}/Persistence/Protoc"
    "${CMAKE_CURRENT_SOURCE_DIR}/Packets/Protoc"
)

# Convert to -I flags
set(PROTOBUF_FLAGS "")
foreach(dir IN LISTS PROTOBUF_IMPORT_DIRS)
    list(APPEND PROTOBUF_FLAGS "-I${dir}")
endforeach()

# Collect all .proto files
file(GLOB_RECURSE PERSISTENCE_PROTOBUF_FILES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Persistence/Protoc/*.proto")
file(GLOB_RECURSE PACKETS_PROTOBUF_FILES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Packets/Protoc/*.proto")

# Generate output files
set(GENERATED_PROTO_SRCS "")
set(GENERATED_PROTO_HDRS "")

foreach(proto_file IN LISTS PACKETS_PROTOBUF_FILES PERSISTENCE_PROTOBUF_FILES)
    get_filename_component(proto_name ${proto_file} NAME_WE)
    set(proto_src_abs "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.cc")
    set(proto_head_abs "${CMAKE_CURRENT_BINARY_DIR}/${proto_name}.pb.h")
    add_custom_command(
        OUTPUT ${proto_src_abs} ${proto_head_abs}
        COMMAND ${Protobuf_PROTOC_EXECUTABLE}
        ARGS
        ${PROTOBUF_FLAGS}
        --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
        ${proto_file}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${proto_file}
        COMMENT "Compiling protobuf file: ${proto_name}.proto"
    )

    list(APPEND GENERATED_PROTO_SRCS ${proto_src_abs})
    list(APPEND GENERATED_PROTO_HDRS ${proto_head_abs})
endforeach()

add_custom_target(generate_protos DEPENDS ${GENERATED_PROTO_SRCS} ${GENERATED_PROTO_HDRS})
set_source_files_properties(${GENERATED_PROTO_SRCS} ${GENERATED_PROTO_HDRS} PROPERTIES SKIP_CLANG_TIDY ON)
add_dependencies(pragmabackend_core generate_protos)

target_sources(pragmabackend_core PRIVATE ${GENERATED_PROTO_SRCS})
target_include_directories(pragmabackend_core PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(tests)
#add_subdirectory(Launcher)
if(MSVC)
    add_compile_options(/W4 /WX /permissive-)
else ()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif ()
