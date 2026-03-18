cmake_minimum_required(VERSION 3.19)
project(SpectreRevivalLauncher LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

file(GLOB LAUNCHER_SOURCE CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.cpp")
file(GLOB LAUNCHER_HEADERS CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.h")
file(GLOB LAUNCHER_UI_FILES CMAKE_CONFIGURE_DEPENDS "${CMAKE_CURRENT_LIST_DIR}/*.ui")

qt_add_executable(SpectreRevivalLauncher
    WIN32 MACOSX_BUNDLE
        ${LAUNCHER_SOURCE}
        ${LAUNCHER_HEADERS}
        ${LAUNCHER_UI_FILES}
)

qt_add_translations(
    TARGETS SpectreRevivalLauncher
    TS_FILES SpectreRevivalLauncher_en_US.ts
)

target_link_libraries(SpectreRevivalLauncher
    PRIVATE
        Qt::Core
        Qt::Widgets
)

target_include_directories(SpectreRevivalLauncher PRIVATE "${CMAKE_CURRENT_LIST_DIR}")

install(TARGETS SpectreRevivalLauncher
    BUNDLE  DESTINATION SpectreRevivalLauncher
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET SpectreRevivalLauncher
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})