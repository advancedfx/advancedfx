cmake_minimum_required(VERSION 3.24)

get_filename_component(AFX_SCRIPT_SOURCE_DIR "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
include("${CMAKE_CURRENT_LIST_DIR}/AfxBuildScript.cmake")

if(NOT DEFINED AFX_STAGE_CONFIG)
    set(AFX_STAGE_CONFIG Debug)
endif()

if(NOT DEFINED AFX_STAGE_CONFIGURE)
    set(AFX_STAGE_CONFIGURE AUTO)
endif()

if(NOT DEFINED AFX_STAGE_X64)
    set(AFX_STAGE_X64 all)
endif()

string(TOLOWER "${AFX_STAGE_CONFIG}" AFX_STAGE_CONFIG_LOWER)
string(TOLOWER "${AFX_STAGE_X64}" AFX_STAGE_X64_LOWER)

if(NOT DEFINED AFX_STAGE_PREFIX)
    set(AFX_STAGE_PREFIX "${AFX_SCRIPT_SOURCE_DIR}/build/dev-dist/${AFX_STAGE_CONFIG_LOWER}")
endif()
get_filename_component(AFX_STAGE_PREFIX "${AFX_STAGE_PREFIX}" ABSOLUTE BASE_DIR "${AFX_SCRIPT_SOURCE_DIR}")

message(STATUS "Stage config: ${AFX_STAGE_CONFIG}")
message(STATUS "Stage prefix: ${AFX_STAGE_PREFIX}")
message(STATUS "Stage configure mode: ${AFX_STAGE_CONFIGURE}")
message(STATUS "Stage x64 mode: ${AFX_STAGE_X64_LOWER}")

afx_stage_architectures(
    "${AFX_STAGE_CONFIG}"
    "${AFX_STAGE_PREFIX}"
    "${AFX_STAGE_CONFIGURE}"
    "${AFX_STAGE_X64_LOWER}"
)

message(STATUS "Stage ready: ${AFX_STAGE_PREFIX}")
