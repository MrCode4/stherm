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
        Style.qml
        ItemDelegate.qml
        Button.qml
        Switch.qml
        ToolButton.qml
        Popup.qml
        RadioButton.qml
        CheckBox.qml
        ApplicationWindow.qml
        TextField.qml
        Slider.qml
        ScrollIndicator.qml
        LimitedRangeSlider.qml
)

qt_add_qml_module(RoniaStyleImpl
    URI Ronia.impl
    STATIC
    VERSION 1.0

    RESOURCES
        res/check.png
        res/check-dark.png

    QML_FILES
        impl/ButtonStyle.qml
        impl/RadioButtonStyle.qml
        impl/SwitchStyle.qml
        impl/TextFieldStyle.qml

        impl/ButtonLabel.qml
        impl/SwitchIndicator.qml
        impl/FontIconsSize.qml
        impl/RadioIndicator.qml
        impl/CheckIndicator.qml
        impl/CursorDelegate.qml
        impl/SliderHandle.qml
        impl/RangeSliderHandles.qml

    SOURCES
        impl/src/BindinableQReal.h
        impl/src/BindinableQReal.cpp
        impl/src/RangeSliderHandleData.h
        impl/src/RangeSliderHandleData.cpp
)

target_include_directories(RoniaStyleImpl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/impl/src
)

target_link_libraries(RoniaStyleImpl PUBLIC
    Qt6::Core5Compat
    Qt6::Quick
)

target_link_libraries(RoniaStyle PUBLIC
    Qt6::Quick
    Qt6::QuickControls2
    RoniaStyleImpl
)
