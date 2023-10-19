cmake_minimum_required(VERSION 3.16)

project(RoniaStyle VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Quick)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Quick)

qt_add_qml_module(RoniaStyle
    URI Ronia
    STATIC
    VERSION 1.0
    IMPORTS QtQuick.Controls.Material
    QML_FILES
        ItemDelegate.qml
)

target_link_libraries(RoniaStyle PUBLIC
    Qt${QT_VERSION_MAJOR}::Quick
)
