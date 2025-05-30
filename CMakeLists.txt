cmake_minimum_required(VERSION 3.16)

# ##################################################################################################
# Project Settings
# ##################################################################################################
project(Stherm VERSION 1.4.13.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 17)
# Set CMP0071 policy
cmake_policy(SET CMP0071 NEW)

# Temperature scheme version and update date.
set(TEMPERATURE_SCHEME_VERSION_C "V.3.1.1 (31.12.2024)")
set(FAKE_UID "" CACHE STRING "The fake UID used for testing purposes")
set(FAKE_SERIALID "" CACHE STRING "The fake SERIAL ID used for testing purposes")
set(SERIAL_TEST_DELAY_COUNTER 3 CACHE STRING "Delay counter for serial test")

# Enable (ON) / Disable (OFF) debug mode to write log file in run time.
option (DEBUG_MODE "Debug mode" OFF)
option (INITIAL_SETUP_MODE "Force the app to run in the initial setup pages" OFF)
option (FAKE_UID_MODE "Run the app with a fake UID set in FAKE_UID" OFF)
option (TROUBLESHOOTING_MODE "Run the app in troubleshooting device mode" OFF)
option (SERIAL_TEST_MODE "Run the app in serial number testing mode" OFF)
option (DISABLE_PROTOBUF "Disable Protobuf on unsupported platforms" OFF)

find_package(Qt6 REQUIRED COMPONENTS
    Quick
    Core
    Gui
    Qml
    QuickControls2
    Multimedia
    Concurrent
    Core5Compat
    SerialPort
)

# Flatten generation of all sub-dependent libraries
set(QT_QML_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/qml)

add_subdirectory(QtQuickStream)
add_subdirectory(Ronia ${QT_QML_OUTPUT_DIRECTORY}/Ronia)

include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Core)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Data)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/php)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Device)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Services)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/Backend/Proto)
include_directories( ${CMAKE_CURRENT_SOURCE_DIR}/QtQuickStream/include)

# ##################################################################################################
# Dependencies
# ##################################################################################################


if (NOT DISABLE_PROTOBUF)
    message( "Protocol Buffers Configuring...")
    include(protobuf.cmake)

    if (UNIX)
        set(PROTOBUF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/unix/include")
        set(PROTOBUF_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/unix/lib")

        set(protobuf_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/unix/lib/cmake/protobuf")
        list(APPEND Protobuf_LIBRARIES ${PROTOBUF_LIBRARY_DIR}/libprotobuf.a)
        list(APPEND Protobuf_LIBRARIES ${PROTOBUF_LIBRARY_DIR}/libprotobuf-lite.a)
        list(APPEND Protobuf_LIBRARIES ${PROTOBUF_LIBRARY_DIR}/libprotoc.a)

        set(absl_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/unix/lib/cmake/absl")
        set(utf8_range_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/unix/lib/cmake/utf8_range")
    else ()
        set(PROTOBUF_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/win/include")
        set(PROTOBUF_LIBRARY_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/win/lib")

        set(protobuf_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/win/lib/cmake/protobuf")
        list(APPEND Protobuf_LIBRARIES ${PROTOBUF_LIBRARY_DIR}/libprotobuf.lib)
        list(APPEND Protobuf_LIBRARIES ${PROTOBUF_LIBRARY_DIR}/libprotobuf-lite.lib)
        list(APPEND Protobuf_LIBRARIES ${PROTOBUF_LIBRARY_DIR}/libprotoc.lib)

        set(absl_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/win/lib/cmake/absl")
        set(utf8_range_DIR "${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/protobufLibs/win/lib/cmake/utf8_range")
    endif ()

    find_package(Protobuf CONFIG)
    find_package(Protobuf REQUIRED)
    find_package(absl REQUIRED)
    find_package(utf8_range REQUIRED)

    include_directories(${PROTOBUF_INCLUDE_DIR})
    include_directories(${CMAKE_CURRENT_BINARY_DIR})

    message("Protocol Buffers found: " ${Protobuf_FOUND} " - " ${Protobuf_VERSION})
    if (Protobuf_FOUND)
        set (PROTO_SRCS Backend/Proto/streamdata.pb.cc)
        set (PROTO_HDRS Backend/Proto/streamdata.pb.h)
        # set_source_files_properties(PROTO_SRCS PROTO_HDRS PROPERTIES GENERATED TRUE)
        # PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS  ${CMAKE_CURRENT_SOURCE_DIR}/Resources/streamdata.proto)
        message("protobuf files generated in " ${PROTO_SRCS} " - " ${PROTO_HDRS})

    else ()
        message(FATAL_ERROR "ERROR: Protobuf not found.")
    endif()
endif()


# Extra QML File properties
set_source_files_properties(
    Core/AppCore.qml
    Core/AppSpec.qml
    Core/Utils.qml
    UiCore/AppStyle.qml
    UiCore/FAIcons.qml
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
        Core/ContactContractor.qml
        Core/NightMode.qml
        Core/ToastManager.qml
        Core/UserPolicyTerms.qml
        Core/Lock.qml
        Core/ServiceTitan.qml


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
        UiCore/PopUps/UpdatePopup.qml
        UiCore/PopUps/CriticalErrorDiagnosticsPopup.qml
        UiCore/PopUps/SkipWIFIConnectionPopup.qml
        UiCore/PopUps/LimitedInitialSetupPopup.qml
        UiCore/PopUps/SendingLogPopup.qml
        UiCore/PopUps/InvalidZipCodePopup.qml

        UiCore/ShortcutManager.qml
        UiCore/SimpleStackView.qml
        UiCore/Components/SemiCircleSlider.qml
        UiCore/Components/SystemModeButton.qml
        UiCore/Components/DateTimeLabel.qml
        UiCore/Components/TempratureLabel.qml
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
        UiCore/Components/Toast.qml
        UiCore/Components/RoniaTumbler.qml
        UiCore/Components/ExpandableItem.qml
        UiCore/Components/PINKeyboard.qml
        UiCore/Components/PINTextField.qml
        UiCore/Components/InfoToolButton.qml
        UiCore/Components/HorizontalTimeTumbler.qml
        UiCore/Components/TemperatureFlatRangeSlider.qml
        UiCore/Components/SingleIconSlider.qml
        UiCore/Components/ContactNuveSupportLabel.qml
        UiCore/Components/CautionRectangle.qml


        # Views and Widgets
        View/Home.qml
        View/SplashScreen.qml

        View/Menu/MenuListView.qml
        View/Menu/AppMenuPage.qml
        View/Menu/SettingsMenuPage.qml
        View/Menu/NotificationBasePage.qml
        View/Menu/MessagesPage.qml
        View/Menu/AlertsPage.qml
        View/Menu/SystemAlertsPage.qml
        View/Menu/ManageEndpoint.qml
        View/Menu/LimitedModeRemainigTimePage.qml
        View/Menu/StorageManagerPage.qml
        View/Menu/ZipCodeEditPage.qml

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
        View/Schedule/ScheduleHumidityPage.qml
        View/SystemSetupPage.qml
        View/SystemSetup/SystemTypePage.qml
        View/SystemSetup/SystemTypeTraditionPage.qml
        View/SystemSetup/SystemTypeHeatPumpPage.qml
        View/SystemSetup/SystemTypeCoolOnlyPage.qml
        View/SystemSetup/SystemTypeHeatOnlyPage.qml
        View/SystemSetup/SystemAccessoriesPage.qml
        View/SystemSetup/SystemRunDelayPage.qml
        View/SystemSetup/InstallationTypePage.qml
        View/SystemSetup/ResidenceTypePage.qml
        View/SystemSetup/DeviceLocationPage.qml
        View/SystemSetup/ThermostatNamePage.qml
        View/SystemSetup/DualFuelHeatingPage.qml
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
        View/WiringPage.qml
        View/MessagePopupView.qml
        View/VacationModeView.qml
        View/AboutDevicePage.qml
        View/UserGuidePage.qml
        View/ContactContractorPage.qml
        View/NightModePage.qml
        View/BackdoorUpdatePage.qml
        View/PrivacyPolicyPage.qml
        View/MobileAppPage.qml
        View/LockPage.qml
        View/UnlockPage.qml
        View/UnlockEmergencyPage.qml
        View/ContractorInformationPage.qml
        View/ContractorInformationFinishPage.qml

        # Service Titan pages
        View/ServiceTitan/InitialSetupBasePageView.qml
        View/ServiceTitan/JobNumberPage.qml
        View/ServiceTitan/CustomerDetailsPage.qml
        View/ServiceTitan/ServiceTitanReviewPage.qml
        View/ServiceTitan/WarrantyReplacementPage.qml

        # Date and Time
        View/DateTime/DateTimePage.qml
        View/DateTime/SelectTimePage.qml
        View/DateTime/SelectTimezonePage.qml
        View/DateTime/SelectDatePage.qml
        View/DateTime/MonthDayDelegate.qml

        # Test
        View/Test/TestsHostPage.qml
        View/Test/TouchTestPage.qml
        View/Test/ColorTestPage.qml
        View/Test/BrightnessTestPage.qml
        View/Test/BacklightTestPage.qml
        View/Test/AudioTestPage.qml
        View/Test/InternalSensorTestPage.qml
        View/Test/RelayTestPage.qml
        View/Test/VersionInformationPage.qml
        View/Test/SystemUpdateOnTestModePage.qml
        View/Test/StartTestPage.qml
        View/Test/QRCodeTestPage.qml
        View/Test/TestConfigPage.qml

        # Delegates
        View/Delegates/ShadeButtonDelegate.qml
        View/Delegates/ScheduleDelegate.qml
        View/Delegates/WifiDelegate.qml
        View/Delegates/SensorDelegate.qml
        View/Delegates/SensorLocationDelegate.qml

        # Popups
        UiCore/PopUps/AlertNotifPopup.qml
        UiCore/PopUps/BusyPopUp.qml
        UiCore/PopUps/HoldPopup.qml
        UiCore/PopUps/TempratureUnitPopup.qml
        UiCore/PopUps/TimeFormatPopup.qml
        UiCore/PopUps/ExitConfirmPopup.qml
        UiCore/PopUps/ScheduleOverlapPopup.qml
        UiCore/PopUps/DownloadingPopup.qml
        UiCore/PopUps/UpdateInterruptionPopup.qml
        UiCore/PopUps/InstallConfirmationPopup.qml
        UiCore/PopUps/ErrorPopup.qml
        UiCore/PopUps/CountDownPopup.qml

        UiCore/PopUps/UpdateNotificationPopup.qml
        UiCore/PopUps/InfoPopup.qml
        UiCore/PopUps/TestFailedPopup.qml
        UiCore/PopUps/PrivacyPolicyPopup.qml
        UiCore/PopUps/SuccessPopup.qml
        UiCore/PopUps/SwitchHeatingPopup.qml
        UiCore/PopUps/MessagePopup.qml
        UiCore/PopUps/PerfTestPopup.qml
        UiCore/PopUps/ScheduleSystemModeErrorPopup.qml
        UiCore/PopUps/ResetFactoryPopUp.qml
        UiCore/PopUps/ManualDateTimeWarningPopup.qml

        # Dialogs
        UiCore/Dialogs/ResetSettingsDialog.qml
        UiCore/Dialogs/ForgetWifiDialog.qml

    SOURCES
        Backend/Property.h
        Backend/Config.h
        Backend/LogCategoires.h
        Backend/LogCategoires.cpp
        Backend/DeviceInfo.h
        Backend/DeviceInfo.cpp
        Backend/AppSpecCPP.h
        Backend/AppSpecCPP.cpp
        Backend/ScreenSaverManager.h
        Backend/ScreenSaverManager.cpp
        Backend/DateTimeManager.h
        Backend/DateTimeManager.cpp
        Backend/TimezonesDSTMap.h

        Backend/Data/UserData.h

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

        Backend/Core/ProcessExecutor.h
        Backend/Core/ProcessExecutor.cpp
        Backend/Core/Relay.h
        Backend/Core/Relay.cpp
        Backend/Core/Scheme.h
        Backend/Core/Scheme.cpp
        Backend/Core/ScheduleCPP.h
        Backend/Core/ScheduleCPP.cpp
        Backend/Core/NetworkManager.h
        Backend/Core/NetworkManager.cpp
        Backend/Core/HttpExecutor.h
        Backend/Core/HttpExecutor.cpp
        Backend/Core/RestApiExecutor.h
        Backend/Core/RestApiExecutor.cpp
        Backend/Core/DevApiExecutor.h
        Backend/Core/DevApiExecutor.cpp
        Backend/Core/System.h
        Backend/Core/System.cpp
        Backend/Core/Sync.h
        Backend/Core/Sync.cpp        
        Backend/Core/HumidityScheme.cpp
        Backend/Core/HumidityScheme.h
        Backend/Core/BaseScheme.h
        Backend/Core/BaseScheme.cpp
        Backend/Core/SchemeDataProvider.h
        Backend/Core/SchemeDataProvider.cpp
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
        Backend/AppUtilities.h
        Backend/AppUtilities.cpp
        Backend/Device/SystemSetup.h
        Backend/Device/SystemSetup.cpp
        Backend/Device/SystemAccessories.h
        Backend/Device/SystemAccessories.cpp

        Backend/Network/Nmcli/Nmcli.h
        Backend/Network/Nmcli/Nmcli.cpp
        Backend/Network/Nmcli/NmcliObserver.h
        Backend/Network/Nmcli/NmcliObserver.cpp
        Backend/Network/Nmcli/NmcliInterface.h
        Backend/Network/Nmcli/NmcliInterface.cpp
        Backend/Network/NetworkInterface.h
        Backend/Network/NetworkInterface.cpp
        Backend/Network/WifiInfo.h
        Backend/Network/WifiInfo.cpp

        Backend/Services/PerfTestService.h
        Backend/Services/PerfTestService.cpp

        Backend/Proto/ProtoDataManager.h
        Backend/Proto/ProtoDataManager.cpp

    RESOURCES
        Fonts/Font\ Awesome\ 6\ Pro-Thin-100.otf
        Fonts/Font\ Awesome\ 6\ Pro-Solid-900.otf
        Fonts/Font\ Awesome\ 6\ Pro-Regular-400.otf
        Fonts/Font\ Awesome\ 6\ Pro-Light-300.otf
        Fonts/Montserrat-Regular.ttf
        Fonts/RobotoMono-Regular.ttf
        Fonts/Roboto-Regular.ttf
        Fonts/unifont.otf
        Images/smart-phone.png
        Images/mobile-app-link.png
        Images/auto.png
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
        Images/icons8-error-48.png
        Images/icons8-installing-updates-50.png
        Images/icons8-message-50.png
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
        Images/delete-left.svg
        Images/lock-image.svg
        Images/swap.svg

        Resources/privacyPolicy.md
        Resources/termOfUse.md

        # Update file
        update.sh
        systemctlRestart.sh
        update_fw_nrf.sh
        update_fw_nrf_seamless.sh
        getsshpass.sh
        nrf_fw_update.zip
        SOURCES heartbeatsender.h heartbeatsender.cpp
)

if (DEBUG_MODE)
    target_compile_definitions(appStherm PRIVATE DEBUG_MODE=${DEBUG_MODE})
endif()

if(FAKE_UID_MODE)
    target_compile_definitions(appStherm PRIVATE FAKE_UID_MODE_ON)
    target_compile_definitions(appStherm PRIVATE FAKE_UID="${FAKE_UID}")
    target_compile_definitions(appStherm PRIVATE FAKE_SERIALID="${FAKE_SERIALID}")
endif()

if(INITIAL_SETUP_MODE)
    target_compile_definitions(appStherm PRIVATE INITIAL_SETUP_MODE_ON)
endif()

if(TROUBLESHOOTING_MODE)
    target_compile_definitions(appStherm PRIVATE TROUBLESHOOTING_MODE_ON)
endif()

if(SERIAL_TEST_MODE)
    target_compile_definitions(appStherm PRIVATE SERIAL_TEST_MODE_ON)
    target_compile_definitions(appStherm PRIVATE SERIAL_TEST_DELAY_COUNTER=${SERIAL_TEST_DELAY_COUNTER})
endif()

if (NOT DISABLE_PROTOBUF)
    target_compile_definitions(appStherm PRIVATE PROTOBUF_ENABLED)
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

target_compile_definitions(appStherm PRIVATE
# Set a compile definition to be able to pass app version to cpp
    PROJECT_VERSION_STRING="${PROJECT_VERSION}"

# Temperature scheme version and update date.
    TEMPERATURE_SCHEME_VERSION="${TEMPERATURE_SCHEME_VERSION_C}"
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

if(NOT DISABLE_PROTOBUF)
    target_sources(appStherm
        PRIVATE

        ${PROTO_HDRS}
        ${PROTO_SRCS}
    )

    target_link_libraries(appStherm
        PRIVATE

        ${Protobuf_LIBRARIES}

        absl::log_internal_message
        absl::log_internal_check_op
        absl::flags_parse
        absl::status
        absl::statusor
        absl::container_common
        absl::strings
        absl::log_streamer

        utf8_range::utf8_range
        utf8_range::utf8_validity
    )
endif()

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
add_subdirectory(watchdog)
