cmake_minimum_required(VERSION 3.16)
project(StatsOverlay LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

set(Qt6_DIR "C:/Qt/6.9.2/mingw_64/lib/cmake/Qt6")

# Найти Qt6 Widgets
find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
qt_standard_project_setup()

set(CURL_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/external/curl/include")
set(CURL_LIB_DIR "${CMAKE_SOURCE_DIR}/external/curl/lib")
set(CURL_DLL_DIR "${CMAKE_SOURCE_DIR}/external/curl/bin")

include_directories(${CURL_INCLUDE_DIR})
link_directories(${CURL_LIB_DIR})

qt_add_resources(RESOURCES
    modules/main_overlay/main_resources.qrc
    modules/sub_overlay/sub_resources.qrc
    modules/main_overlay/widgets/main_stats/main_stats_resources.qrc
    modules/main_overlay/widgets/rating_stats/rating_stats_resources.qrc
    modules/main_overlay/widgets/vehicles_stats/vehicles_resources.qrc
    modules/main_overlay/widgets/other_stats/other_stats_resources.qrc
    modules/main_overlay/widgets/player_stats/player_stats.qrc
    modules/main_overlay/widgets/player_stats/split_players/players_window.qrc
    modules/main_overlay/controller/data/vehicles_data_resources.qrc
    modules/main_overlay/widgets/gun_marks/gun_marks.qrc
    modules/main_overlay/widgets/info_page/info_resources.qrc
    modules/main_overlay/controller/cacert_resources.qrc
)
# Создаём исполняемый файл (только cpp)
qt_add_executable(StatsOverlay
    main.cpp
    modules/main_overlay/main_overlay.cpp
    modules/main_overlay/widgets/main_stats/main_stats.cpp
    modules/main_overlay/widgets/rating_stats/rating_stats.cpp
    modules/main_overlay/widgets/other_stats/other_stats.cpp
    modules/main_overlay/widgets/vehicles_stats/vehicles_stats.cpp
    modules/main_overlay/widgets/player_stats/player_stats.cpp
    modules/main_overlay/widgets/player_stats/split_players/players_window.cpp
    modules/main_overlay/widgets/gun_marks/gun_marks.cpp
    modules/main_overlay/widgets/gun_marks/gunmarks_window/gunmarks_window.cpp
    modules/main_overlay/widgets/info_page/info_page.cpp
    modules/sub_overlay/sub_overlay.cpp
    modules/main_overlay/activate/activation_window.cpp
    ${RESOURCES}
)

# Подключаем include-директорию, чтобы хедер был виден
target_include_directories(StatsOverlay PRIVATE ${CMAKE_SOURCE_DIR}/modules)

# Линкуем Qt
target_link_libraries(StatsOverlay PRIVATE Qt6::Widgets Qt6::Network curl ws2_32 bcrypt)

add_custom_command(TARGET StatsOverlay POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CURL_DLL_DIR}/libcurl-x64.dll"
        $<TARGET_FILE_DIR:StatsOverlay>)
