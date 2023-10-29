cmake_minimum_required(VERSION 3.16)

project(RoniaStyle VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick QuickControls2 Core5Compat)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set_source_files_properties(
    Style.qml

    PROPERTIES
        QT_QML_SINGLETON_TYPE True
)

qt_add_qml_module(RoniaStyle
    URI Ronia
    STATIC
    VERSION 1.0
    IMPORTS QtQuick.Controls.Material
    QML_FILES
        ItemDelegate.qml
        Button.qml
        Style.qml
)

qt_add_qml_module(RoniaStyleImpl
    URI Ronia.impl
    STATIC
    VERSION 1.0
    QML_FILES
        impl/ButtonStyle.qml
        impl/ButtonLabel.qml

    RESOURCES
        res/check.png
)

target_link_libraries(RoniaStyleImpl PUBLIC
    Qt6::Core5Compat
)

target_link_libraries(RoniaStyle PUBLIC
    Qt6::Quick
    Qt6::QuickControls2
    RoniaStyleImpl
)
