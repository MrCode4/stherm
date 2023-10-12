cmake_minimum_required(VERSION 3.16)

# ##################################################################################################
# Project Settings
# ##################################################################################################
project(Stherm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui Qml QuickControls2 opengl Multimedia Concurrent)

qt_standard_project_setup()

# ##################################################################################################
# Dependencies
# ##################################################################################################
# Flatten generation of all sub-dependent libraries
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend)

# Add any include directories
#include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend)

# Dependencies
add_subdirectory(QtQuickStream)

# Extra QML File properties
set_source_files_properties(
    Core/AppCore.qml
    Core/AppSpec.qml
    UiCore/AppStyle.qml
    UiCore/AppSettings.qml

    PROPERTIES
        QT_QML_SINGLETON_TYPE True
)

# ##################################################################################################
# Project Files
# ##################################################################################################
# Executable Definition
qt_add_executable(appStherm
    main.cpp
)

# Project Files
qt6_add_qml_module(appStherm
    URI Stherm
    VERSION 1.0
    QML_FILES
        Main.qml

        # Core
        Core/AppCore.qml
        Core/AppSpec.qml
        Core/I_Device.qml
        Core/Device.qml
        Core/SimDevice.qml
        Core/SelectionModel.qml


        # Primitive Components
        UiCore/Components/RoniaFlatButton.qml
        UiCore/Components/RoniaTextIcon.qml
        UiCore/Components/RoniaMenuItem.qml
        UiCore/Components/RoniaThreeWaySwitch.qml
        UiCore/Components/RoniaExpandableItem.qml
        UiCore/Components/RoniaUserInputBase.qml
        UiCore/Components/RoniaUserInputCombo.qml
        UiCore/Components/RoniaUserInputNumberEditor.qml
        UiCore/Components/RoniaUserInputTextField.qml
        UiCore/Components/RoniaUserInputToggle.qml
        UiCore/Components/RoniaToolTip.qml
        UiCore/Components/RoniaNumberEditor.qml
        UiCore/Components/RoniaComboBox.qml
        UiCore/Components/RoniaColorButton.qml
        UiCore/Components/RoniaContextMenu.qml
        UiCore/Components/RoniaContextMenuItem.qml
        UiCore/Components/RoniaRoundMouseArea.qml
        UiCore/Components/Buttons/RoniaBaseButton.qml
        UiCore/Components/Buttons/RoniaButton.qml
        UiCore/Components/Buttons/RoniaIconButton.qml
        UiCore/Components/Buttons/RoniaIconButtonRound.qml
        UiCore/Components/Buttons/RoniaIconTextButton.qml
        UiCore/Components/Buttons/RoniaSideMenuButton.qml
        UiCore/Components/Buttons/RoniaControlToolButton.qml


        # UiSession+Popups
        UiCore/AppStyle.qml
        UiCore/AppSettings.qml
        UiCore/UiSession.qml
        UiCore/UiSessionPopups.qml
        UiCore/UiPreferences.qml
        UiCore/PopUpLayout.qml
        UiCore/PanelLayout.qml
        UiCore/I_PopUp.qml
        UiCore/I_Panel.qml
        UiCore/PopUps/AboutPopUp.qml
        UiCore/PopUps/ConfirmPopup.qml
        UiCore/PopUps/SaveDialoge.qml
        UiCore/PopUps/LoadDialoge.qml
        UiCore/ShortcutManager.qml

        # Widgets
        View/MainView.qml

    SOURCES
        Backend/AppSpecCPP.h
        Backend/AppSpecCPP.cpp
        Backend/AppSettingsCPP.h
        Backend/AppSettingsCPP.cpp

    RESOURCES
        Fonts/Font\ Awesome\ 6\ Pro-Thin-100.otf
        Fonts/Font\ Awesome\ 6\ Pro-Solid-900.otf
        Fonts/Font\ Awesome\ 6\ Pro-Regular-400.otf
        Fonts/Font\ Awesome\ 6\ Pro-Light-300.otf
        Fonts/RobotoMono-Regular.ttf
        QML_FILES Core/Wifi.qml
        QML_FILES Core/WifiController.qml
        QML_FILES Core/Fan.qml
        QML_FILES Core/Backlight.qml
        QML_FILES Core/WifiDevice.qml
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(appStherm PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appStherm
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(appStherm
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Core
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::OpenGL
    Qt6::Concurrent
    QtQuickStreamplugin
)

include(GNUInstallDirs)
install(TARGETS appStherm
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)


# ##################################################################################################
# Qt Creator Fixes
# ##################################################################################################
# Make Qt Creator aware of where the QML modules live
set(QML_IMPORT_PATH ${QT_QML_OUTPUT_DIRECTORY} CACHE STRING "QtCreator QML Modules Lookup")
