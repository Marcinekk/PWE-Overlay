cmake_minimum_required(VERSION 3.16)
project(PWEOverlay VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/utf-8)
endif()

# ----------------------------------------
# Directories
# ----------------------------------------
set(FRONTEND_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../frontend")
set(FRONTEND_DIST_DIR "${FRONTEND_DIR}/dist")
set(OUTPUT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../scripts/Release")

# ----------------------------------------
# Frontend build (Node.js)
# ----------------------------------------
if(EXISTS "${FRONTEND_DIR}/package.json")
    find_program(NPM_EXECUTABLE NAMES npm.cmd npm)
    if(NOT NPM_EXECUTABLE)
        message(FATAL_ERROR "npm not found. Install Node.js/npm to build embedded frontend.")
    endif()

    add_custom_target(PWEOverlayFrontend ALL
        COMMAND ${NPM_EXECUTABLE} --prefix "${FRONTEND_DIR}" run build
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        COMMENT "Building PWEOverlay frontend"
        VERBATIM
    )
endif()

# ----------------------------------------
# WebView2
# ----------------------------------------
include(FetchContent)
FetchContent_Declare(
    webview2
    URL "https://www.nuget.org/api/v2/package/Microsoft.Web.WebView2"
)
FetchContent_MakeAvailable(webview2)

# ----------------------------------------
# PWEOverlay
# ----------------------------------------
add_library(PWEOverlay SHARED
    PWEOverlay.cpp
    State/OverlayState.cpp
    Telemetry/TelemetryPublisher.cpp
    Internal/WebViewCommandBridge.cpp
    Events/Events.cpp
    Hooks/Hooks.cpp
    Hooks/bank_economy/BootstrapHook.cpp
    Hooks/bank_economy/Context.cpp
    Hooks/bank_deposit/DepositHook.cpp
    Hooks/bank_withdraw/WithdrawHook.cpp
    Logitech/G27LEDManager.cpp
    PWEWebView.cpp
)

if(TARGET PWEOverlayFrontend)
    add_dependencies(PWEOverlay PWEOverlayFrontend)
endif()

# ----------------------------------------
# Include directories
# ----------------------------------------
target_include_directories(PWEOverlay PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/External/hidapi/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/../Framework/include/SPF"
    "${webview2_SOURCE_DIR}/build/native/include"
)

# ----------------------------------------
# HIDAPI
# ----------------------------------------
target_link_libraries(PWEOverlay PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/External/hidapi/hidapi.lib"
)

# ----------------------------------------
# OUTPUT
# ----------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIR})

foreach(CONFIG_TYPE Debug Release RelWithDebInfo MinSizeRel)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${OUTPUT_DIR})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${OUTPUT_DIR})
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CONFIG_TYPE} ${OUTPUT_DIR})
endforeach()

# ----------------------------------------
# WebView2Loader.dll & frontend & PWEOverlay.dll
# ----------------------------------------
add_custom_command(TARGET PWEOverlay POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:PWEOverlay>"
        "${OUTPUT_DIR}/$<TARGET_FILE_NAME:PWEOverlay>"

    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${webview2_SOURCE_DIR}/build/native/x64/WebView2Loader.dll"
        "${OUTPUT_DIR}/WebView2Loader.dll"

    COMMAND ${CMAKE_COMMAND} -E remove_directory "${OUTPUT_DIR}/ui"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${OUTPUT_DIR}/ui"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${FRONTEND_DIST_DIR}"
        "${OUTPUT_DIR}/ui"
)