cmake_minimum_required(VERSION 3.16)

# ##################################################################################################
# Project Settings
# ##################################################################################################
project(Stherm VERSION 0.4.2 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable (ON) / Disable (OFF) debug mode to write log file in run time.
option (DEBUG_MODE "Debug mode" OFF)

find_package(Qt6 REQUIRED COMPONENTS Quick Core Gui Qml
    QuickControls2 Multimedia Concurrent
    Core5Compat SerialPort )

#qt_standard_project_setup()

# ##################################################################################################
# Dependencies
# ##################################################################################################
# Flatten generation of all sub-dependent libraries
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Core)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/php)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Device)

# Add any include directories
#include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend)

# Dependencies
add_subdirectory(QtQuickStream)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/QtQuickStream/include)

add_subdirectory(Ronia ${QT_QML_OUTPUT_DIRECTORY}/Ronia)

# Extra QML File properties
set_source_files_properties(
    Core/AppCore.qml
    Core/AppSpec.qml
    Core/Utils.qml
    UiCore/AppStyle.qml
    UiCore/FAIcons.qml
    Core/DateTimeManager.qml

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
        Core/Fan.qml
        Core/Backlight.qml
        Core/DeviceController.qml
        Core/I_DeviceController.qml
        Core/SchedulesController.qml
        Core/Sensor.qml
        Core/SensorController.qml
        Core/Message.qml
        Core/MessageController.qml
        Core/Wiring.qml
        Core/Vacation.qml
        Core/Setting.qml
        Core/Utils.qml
        Core/DateTimeManager.qml
        Core/ContactContractor.qml
        Core/NightMode.qml


        # Primitive Components
        UiCore/Components/RoniaTextIcon.qml


        # UiSession+Popups
        UiCore/FAIcons.qml
        UiCore/AppStyle.qml
        UiCore/UiSession.qml
        UiCore/UiSessionPopups.qml
        UiCore/UiPreferences.qml
        UiCore/PopUpLayout.qml
        UiCore/PanelLayout.qml
        UiCore/I_PopUp.qml
        UiCore/I_Panel.qml
        UiCore/I_Dialog.qml
        UiCore/PopUps/AboutPopUp.qml
        UiCore/PopUps/ConfirmPopup.qml
        UiCore/PopUps/SaveDialoge.qml
        UiCore/PopUps/LoadDialoge.qml
        UiCore/PopUps/UpdatePopup.qml
        UiCore/ShortcutManager.qml
        UiCore/Components/SemiCircleSlider.qml
        UiCore/Components/SystemModeButton.qml
        UiCore/Components/DateTimeLabel.qml
        UiCore/Components/CurrentTempratureLabel.qml
        UiCore/Components/DesiredTempratureItem.qml
        UiCore/Components/AirConditionItem.qml
        UiCore/Components/FanButton.qml
        UiCore/Components/CurrentHumidityButton.qml
        UiCore/Components/HoldButton.qml
        UiCore/Components/WifiIcon.qml
        UiCore/Components/WifiButton.qml
        UiCore/Components/MenuButton.qml
        UiCore/Components/ScheduleButton.qml
        UiCore/Components/ColorSlider.qml
        UiCore/Components/BrightnessSlider.qml
        UiCore/Components/TickedSlider.qml
        UiCore/Components/RangeSliderLabeled.qml
        UiCore/Components/ButtonInverted.qml
        UiCore/Components/OnScheduleLabel.qml
        UiCore/Components/RangeSlider.qml
        UiCore/Components/SemiCircleSliderDoubleHandle.qml


        # Views and Widgets
        View/Home.qml
        View/SplashScreen.qml
        View/ApplicationMenu.qml
        View/ApplicationMenuList.qml
        View/BasePageView.qml
        View/ScheduleView.qml
        View/BacklightPage.qml
        View/MainView.qml
        View/OrganizationIcon.qml
        View/ScreenSaver.qml
        View/FanPage.qml
        View/SystemInfoPage.qml
        View/AddSchedulePage.qml
        View/SystemUpdatePage.qml
        View/Schedule/ScheduleTypePage.qml
        View/Schedule/ScheduleTempraturePage.qml
        View/Schedule/ScheduleTimePage.qml
        View/Schedule/ScheduleRepeatPage.qml
        View/Schedule/SchedulePreviewPage.qml
        View/Schedule/ScheduleNamePage.qml
        View/Schedule/ScheduleDataSourcePage.qml
        View/SystemSetupPage.qml
        View/SystemSetup/SystemTypePage.qml
        View/SystemSetup/SystemTypeTraditionPage.qml
        View/SystemSetup/SystemTypeHeatPumpPage.qml
        View/SystemSetup/SystemTypeCoolOnlyPage.qml
        View/SystemSetup/SystemTypeHeatOnlyPage.qml
        View/SystemSetup/SystemAccessoriesPage.qml
        View/SystemSetup/SystemRunDelayPage.qml
        View/HumidityPage.qml
        View/WifiPage.qml
        View/RequestTechPriorityPage.qml
        View/Wifi/WifiConnectPage.qml
        View/Wifi/WifiManualConnectPage.qml
        View/SystemModePage.qml
        View/SystemMode/VacationModePage.qml
        View/SettingsPage.qml
        View/Sensor/AddSensorPage.qml
        View/Sensor/SensorLocationPage.qml
        View/Sensor/SensorPairPage.qml
        View/Sensor/SensorNamePage.qml
        View/Sensor/SensorInfoPage.qml
        View/SensorsPage.qml
        View/AlertsNotificationsPage.qml
        View/WiringPage.qml
        View/MessagePopupView.qml
        View/VacationModeView.qml
        View/AboutDevicePage.qml
        View/UserGuidePage.qml
        View/ContactContractorPage.qml
        View/NightModePage.qml
        View/BackdoorUpdatePage.qml

        # Date and Time
        View/DateTime/DateTimePage.qml
        View/DateTime/SelectTimePage.qml
        View/DateTime/SelectTimezonePage.qml
        View/DateTime/SelectDatePage.qml
        View/DateTime/MonthDayDelegate.qml

        # Test
        View/Test/TouchTestPage.qml
        View/Test/ColorTestPage.qml
        View/Test/BacklightTestPage.qml
        View/Test/AudioTestPage.qml
        View/Test/InternalSensorTestPage.qml
        View/Test/RelayTestPage.qml
        View/Test/VersionInformationPage.qml
        View/Test/SystemUpdateOnTestModePage.qml
        View/Test/StartTestPage.qml
        View/Test/QRCodeTestPage.qml

        # Delegates
        View/Delegates/ApplicationMenuDelegate.qml
        View/Delegates/ShadeButtonDelegate.qml
        View/Delegates/ScheduleDelegate.qml
        View/Delegates/WifiDelegate.qml
        View/Delegates/SensorDelegate.qml
        View/Delegates/AlertNotificationDelegate.qml
        View/Delegates/SensorLocationDelegate.qml

        # Popups
        UiCore/PopUps/AlertNotifPopup.qml
        UiCore/PopUps/HoldPopup.qml
        UiCore/PopUps/TempratureUnitPopup.qml
        UiCore/PopUps/TimeFormatPopup.qml
        UiCore/PopUps/ExitConfirmPopup.qml
        UiCore/PopUps/ScheduleOverlapPopup.qml
        UiCore/PopUps/DownloadingPopup.qml
        UiCore/PopUps/UpdateInterruptionPopup.qml
        UiCore/PopUps/InstallConfirmationPopup.qml
        UiCore/PopUps/ErrorPopup.qml
        UiCore/PopUps/RebootDevicePopup.qml
        UiCore/PopUps/ResetDevicePopup.qml
        UiCore/PopUps/UpdateNotificationPopup.qml

        # Dialogs
        UiCore/Dialogs/ResetSettingsDialog.qml
        UiCore/Dialogs/ForgetWifiDialog.qml

    SOURCES
        Backend/AppSpecCPP.h
        Backend/AppSpecCPP.cpp
        Backend/ScreenSaverManager.h
        Backend/ScreenSaverManager.cpp
        Backend/DateTimeManagerCPP.h
        Backend/DateTimeManagerCPP.cpp
        Backend/TimezonesDSTMap.h

        # php
        Backend/PhpApi.h
        Backend/PhpApi.cpp
        Backend/php/php_hardware.h
        Backend/php/php_hardware.cpp
        Backend/php/php_system.h
        Backend/php/php_system.cpp
        Backend/php/php_sensors.h
        Backend/php/php_sensors.cpp
        Backend/php/php_wifi.h
        Backend/php/php_wifi.cpp
        Backend/php/php_schedule.h
        Backend/php/php_schedule.cpp
        Backend/php/php_update.h
        Backend/php/php_update.cpp

        Backend/php/include/timing.h
        Backend/php/include/timing.cpp
        Backend/php/include/deviceconfig.h
        Backend/php/include/deviceconfig.cpp
        Backend/php/include/currentstage.h
        Backend/php/include/currentstage.cpp
        Backend/php/include/nuveTypes.h
        Backend/php/include/sensors.h
        Backend/php/include/sensors.cpp
        Backend/php/include/wirings.h
        Backend/php/include/wirings.cpp
        Backend/php/include/parameter_definitions.h

        Backend/DeviceControllerCPP.h
        Backend/DeviceControllerCPP.cpp
        Backend/UARTConnection.h
        Backend/UARTConnection.cpp

        Backend/deviceIOController.h
        Backend/deviceIOController.cpp

        Backend/DataParser.h
        Backend/DataParser.cpp
        Backend/UtilityHelper.h
        Backend/UtilityHelper.cpp

        Backend/Core/Relay.h
        Backend/Core/Relay.cpp
        Backend/Core/Scheme.h
        Backend/Core/Scheme.cpp
        Backend/Core/ScheduleCPP.h
        Backend/Core/ScheduleCPP.cpp
        Backend/Core/System.h
        Backend/Core/System.cpp
        Backend/Core/Sync.h
        Backend/Core/Sync.cpp
        Backend/Core/NetworkWorker.h
        Backend/Core/NetworkWorker.cpp
        Backend/LogHelper.h

        Backend/DeviceAPI.h
        Backend/DeviceAPI.cpp
        Backend/Device/nuve_types.h
        Backend/Device/device_config.h
        Backend/Device/device_config.cpp
        Backend/Device/timing.h
        Backend/Device/timing.cpp
        Backend/Device/current_stage.h
        Backend/Device/current_stage.cpp
        Backend/Device/sensors.h
        Backend/Device/sensors.cpp
        Backend/Core/hardware.h
        Backend/Core/hardware.cpp
        Backend/GPIO/GpioHandler.h
        Backend/GPIO/GpioHandler.cpp


        Backend/QrCode/qrcodegen.hpp
        Backend/QrCode/qrcodegen.cpp
        Backend/QRCodeGenerator.h
        Backend/Device/SystemSetup.h
        Backend/Device/SystemSetup.cpp
        Backend/Device/SystemAccessories.h
        Backend/Device/SystemAccessories.cpp

        Backend/Network/Nmcli/Nmcli.h
        Backend/Network/Nmcli/NmcliObserver.h
        Backend/Network/Nmcli/NmcliObserver.cpp
        Backend/Network/Nmcli/NmcliInterface.h
        Backend/Network/Nmcli/NmcliInterface.cpp
        Backend/Network/NetworkInterface.h
        Backend/Network/NetworkInterface.cpp

    RESOURCES
        Fonts/Font\ Awesome\ 6\ Pro-Thin-100.otf
        Fonts/Font\ Awesome\ 6\ Pro-Solid-900.otf
        Fonts/Font\ Awesome\ 6\ Pro-Regular-400.otf
        Fonts/Font\ Awesome\ 6\ Pro-Light-300.otf
        Fonts/Montserrat-Regular.ttf
        Fonts/RobotoMono-Regular.ttf
        Fonts/Roboto-Regular.ttf
        Images/auto.png
        Images/cool.png
        Images/sun.png
        Images/condition-good.png
        Images/condition-moderate.png
        Images/condition-poor.png
        Images/fan-off.png
        Images/fan-on.png
        Images/fan-cool.svg
        Images/fan-heat.svg
        Images/water.png
        Images/Wifi/disconnected.svg
        Images/nexgen.png
        Images/nuve-icon.png
        Images/medley.png
        Images/lee_s.png
        Images/Wifi/wifi.png
        Images/Wifi/wifi-good.png
        Images/Wifi/wifi-fair.png
        Images/Wifi/wifi-weak.png
        Images/Wifi/wifi-off.png
        Images/Wifi/wifi-no-internet.png
        Images/Locations/basement.svg
        Images/Locations/bathroom.svg
        Images/Locations/bedroom.svg
        Images/Locations/dining_room.svg
        Images/Locations/downstairs.svg
        Images/Locations/guesthouse.svg
        Images/Locations/kids_room.svg
        Images/Locations/kitchen.svg
        Images/Locations/living_room.svg
        Images/Locations/main_floor.svg
        Images/Locations/master_bedroom.svg
        Images/Locations/office.svg
        Images/Locations/other.svg
        Images/Locations/upstairs.svg

        # Update file
        update.sh
        update_fw_nrf.sh
        update_fw_nrf_seamless.sh
        nrf_fw_update.zip
        SOURCES Backend/Network/WifiInfo.h Backend/Network/WifiInfo.cpp
)

if (DEBUG_MODE)
    target_compile_definitions(appStherm PRIVATE DEBUG_MODE=${DEBUG_MODE})
endif()

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

# Set a compile definition to be able to pass app version to cpp
target_compile_definitions(appStherm PRIVATE
    PROJECT_VERSION_STRING="${PROJECT_VERSION}"
)

target_link_directories(appStherm PRIVATE
    ${QT_QML_OUTPUT_DIRECTORY}/Ronia
)

target_include_directories(appStherm PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Network
)

target_link_libraries(appStherm
    PRIVATE
    Qt6::Quick
    Qt6::Qml
    Qt6::Gui
    Qt6::Core
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::Concurrent
    Qt6::SerialPort

    QtQuickStreamplugin
    RoniaStyle
    RoniaStyleplugin
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
