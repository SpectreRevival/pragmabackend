set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

file(GLOB_RECURSE LAUNCHER_SOURCES "${CMAKE_CURRENT_LIST_DIR}/Implementation/*.cpp")

qt_add_executable(SpectreLauncher ${LAUNCHER_SOURCES})

target_link_libraries(SpectreLauncher PRIVATE Qt6::Widgets)
target_include_directories(SpectreLauncher PRIVATE "${CMAKE_CURRENT_LIST_DIR}/Model")

qt_add_resources(SpectreLauncher "resources" PREFIX "/" FILES "assets/appicon.png")
