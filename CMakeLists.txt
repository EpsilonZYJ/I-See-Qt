cmake_minimum_required(VERSION 3.16)

project(VideoGenClient VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6 组件
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Multimedia MultimediaWidgets Sql)

# 自动处理 Qt 的 MOC, UIC, RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 包含头文件目录
include_directories(
        src
        src/ui
        src/viewmodel
        src/services
        src/utils
        src/models
)

# 添加源文件
set(SOURCES
        src/main.cpp
        src/const/AppConfig.h
        src/models/TaskItem.h
        src/services/ApiService.h src/services/ApiService.cpp
        src/services/HistoryService.h src/services/HistoryService.cpp
        src/services/TaskDatabaseService.h src/services/TaskDatabaseService.cpp
        src/viewmodel/MainViewModel.h src/viewmodel/MainViewModel.cpp
        src/ui/SetupDialog.h src/ui/SetupDialog.cpp
        src/ui/MainWindow.h src/ui/MainWindow.cpp
        src/ui/TaskHistoryWindow.h src/ui/TaskHistoryWindow.cpp
        resources/ui.qrc
        src/const/QtHeaders.h
)

# 生成可执行文件
if(APPLE)
    # macOS Bundle 设置
    set(MACOSX_BUNDLE_BUNDLE_NAME "ISee")
    set(MACOSX_BUNDLE_GUI_IDENTIFIER "com.EpsilonZYJ.ISee")
    add_executable(VideoGenClient MACOSX_BUNDLE ${SOURCES})
else()
    add_executable(VideoGenClient ${SOURCES})
endif()

# 链接 Qt 库
target_link_libraries(VideoGenClient PRIVATE
        Qt6::Widgets
        Qt6::Network
        Qt6::Multimedia
        Qt6::MultimediaWidgets
        Qt6::Sql
)