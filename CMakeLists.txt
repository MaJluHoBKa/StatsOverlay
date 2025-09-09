cmake_minimum_required(VERSION 3.16)
project(MyOverlayApp LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

# Найти Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets)
qt_standard_project_setup()

qt_add_resources(RESOURCES modules/main_overlay/resources.qrc)

# Создаём исполняемый файл (только cpp)
add_executable(MyOverlayApp
    main.cpp
    modules/main_overlay/main_overlay.cpp
    ${RESOURCES}
)

# Подключаем include-директорию, чтобы хедер был виден
target_include_directories(MyOverlayApp PRIVATE ${CMAKE_SOURCE_DIR}/modules)

# Линкуем Qt
target_link_libraries(MyOverlayApp PRIVATE Qt6::Widgets)
