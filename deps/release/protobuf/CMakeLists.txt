cmake_minimum_required (VERSION 3.24)

project ("protobuf")

#
# protobuf
#

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /permissive-") # Fix internal compiler error in debug builds: https://developercommunity.visualstudio.com/t/fatal--error-C1001:-Internal-compiler-er/10906076?sort=newest

set(protobuf_INSTALL OFF)
set(protobuf_BUILD_TESTS OFF)
set(protobuf_BUILD_CONFORMANCE OFF)
set(protobuf_BUILD_EXAMPLES OFF)
set(protobuf_BUILD_PROTOBUF_BINARIES ON)
set(protobuf_BUILD_PROTOC_BINARIES ON)
set(protobuf_BUILD_SHARED_LIBS OFF)
set(protobuf_MSVC_STATIC_RUNTIME OFF)
FetchContent_Declare(
	protobuf
	GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG 74211c0dfc2777318ab53c2cd2c317a2ef9012de #v31.1
)
FetchContent_MakeAvailable(protobuf)
