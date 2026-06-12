function(afx_script_run)
    message(STATUS "Running: ${ARGV}")
    execute_process(
        COMMAND ${ARGV}
        WORKING_DIRECTORY "${AFX_SCRIPT_SOURCE_DIR}"
        RESULT_VARIABLE AFX_SCRIPT_RESULT
    )
    if(NOT AFX_SCRIPT_RESULT EQUAL 0)
        message(FATAL_ERROR "Command failed with exit code ${AFX_SCRIPT_RESULT}: ${ARGV}")
    endif()
endfunction()

function(afx_script_run_in WORKING_DIR)
    message(STATUS "Running in ${WORKING_DIR}: ${ARGN}")
    execute_process(
        COMMAND ${ARGN}
        WORKING_DIRECTORY "${WORKING_DIR}"
        RESULT_VARIABLE AFX_SCRIPT_RESULT
    )
    if(NOT AFX_SCRIPT_RESULT EQUAL 0)
        message(FATAL_ERROR "Command failed with exit code ${AFX_SCRIPT_RESULT}: ${ARGN}")
    endif()
endfunction()

function(afx_configure_preset PRESET CONFIGURE_MODE)
    set(AFX_SCRIPT_BUILD_DIR "${AFX_SCRIPT_SOURCE_DIR}/build/${PRESET}")
    if(CONFIGURE_MODE STREQUAL "ON"
        OR CONFIGURE_MODE STREQUAL "TRUE"
        OR NOT EXISTS "${AFX_SCRIPT_BUILD_DIR}/CMakeCache.txt")
        afx_script_run("${CMAKE_COMMAND}" --preset "${PRESET}")
    else()
        message(STATUS "Using existing configure: ${AFX_SCRIPT_BUILD_DIR}")
    endif()
endfunction()

function(afx_stage_architecture_builds CONFIG CONFIGURE_MODE X64_MODE)
    string(TOLOWER "${CONFIG}" AFX_SCRIPT_CONFIG_LOWER)
    set(AFX_SCRIPT_WIN32_PRESET "win32-${AFX_SCRIPT_CONFIG_LOWER}")
    set(AFX_SCRIPT_X64_PRESET "x64-${AFX_SCRIPT_CONFIG_LOWER}")

    afx_configure_preset("${AFX_SCRIPT_WIN32_PRESET}" "${CONFIGURE_MODE}")
    afx_configure_preset("${AFX_SCRIPT_X64_PRESET}" "${CONFIGURE_MODE}")

    afx_script_run("${CMAKE_COMMAND}" --build --preset "${AFX_SCRIPT_WIN32_PRESET}")

    if(X64_MODE STREQUAL "all")
        afx_script_run("${CMAKE_COMMAND}" --build --preset "${AFX_SCRIPT_X64_PRESET}")
    elseif(X64_MODE STREQUAL "source1")
        afx_script_run("${CMAKE_COMMAND}" --build --preset "${AFX_SCRIPT_X64_PRESET}" --target AfxHookSource)
    elseif(X64_MODE STREQUAL "source2")
        afx_script_run("${CMAKE_COMMAND}" --build --preset "${AFX_SCRIPT_X64_PRESET}" --target AfxHookSource2)
    else()
        message(FATAL_ERROR "Unsupported x64 stage mode: ${X64_MODE}")
    endif()
endfunction()

function(afx_stage_architecture_installs CONFIG PREFIX X64_MODE)
    string(TOLOWER "${CONFIG}" AFX_SCRIPT_CONFIG_LOWER)
    set(AFX_SCRIPT_WIN32_PRESET "win32-${AFX_SCRIPT_CONFIG_LOWER}")
    set(AFX_SCRIPT_X64_PRESET "x64-${AFX_SCRIPT_CONFIG_LOWER}")

    afx_script_run("${CMAKE_COMMAND}" --install "build/${AFX_SCRIPT_WIN32_PRESET}" --config "${CONFIG}" --prefix "${PREFIX}")

    if(X64_MODE STREQUAL "all")
        afx_script_run("${CMAKE_COMMAND}" --install "build/${AFX_SCRIPT_X64_PRESET}" --config "${CONFIG}" --prefix "${PREFIX}")
    elseif(X64_MODE STREQUAL "source1")
        afx_script_run("${CMAKE_COMMAND}" --install "build/${AFX_SCRIPT_X64_PRESET}" --config "${CONFIG}" --prefix "${PREFIX}" --component x64-runtime)
        afx_script_run("${CMAKE_COMMAND}" --install "build/${AFX_SCRIPT_X64_PRESET}" --config "${CONFIG}" --prefix "${PREFIX}" --component x64-source1)
    elseif(X64_MODE STREQUAL "source2")
        afx_script_run("${CMAKE_COMMAND}" --install "build/${AFX_SCRIPT_X64_PRESET}" --config "${CONFIG}" --prefix "${PREFIX}" --component x64-runtime)
        afx_script_run("${CMAKE_COMMAND}" --install "build/${AFX_SCRIPT_X64_PRESET}" --config "${CONFIG}" --prefix "${PREFIX}" --component x64-source2)
    else()
        message(FATAL_ERROR "Unsupported x64 stage mode: ${X64_MODE}")
    endif()
endfunction()

function(afx_stage_architectures CONFIG PREFIX CONFIGURE_MODE X64_MODE)
    afx_stage_architecture_builds("${CONFIG}" "${CONFIGURE_MODE}" "${X64_MODE}")
    afx_stage_architecture_installs("${CONFIG}" "${PREFIX}" "${X64_MODE}")
endfunction()
