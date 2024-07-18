# STHERM

## Introduction

## Pre-requirments
Ensure you have the following software installed and configured before proceeding:
- CMake v3.16 or later
- Qt 6.4.3 with the following modules and libraries:
    - Qt 5 Compatiblity Module
    - Qt Shader Tools
    - Additional libraries
        - Qt Image Formats
        - Qt Multimedia
        - Qt Serial Port
        - Qt Virtual Keyboard
        
if you are using Windows:
        - MSVC 2019 64-bit with the following workloads:
            - Desktop development with C++ workload

## How to build?
1. Set CMake build type to Release
     - Qt Creator: </br>
        Navigate to `Projects` > `Edit build configuration` > `Release`
2. Generate makefiles:
     - Qt Creator: </br> 
        Navigate to `Build` > `Run CMake`
     - CMake: 
        ```properties
        cmake -S . -B ./build/release
        ```
3. Build project
     - Qt Creator: </br>
        Navigate to `Build` > `Build Project`
     - CMake: 
        ```properties
        cmake --build ./build/release --target ALL_BUILD --config Release
        ```

## How to Fake a Serial Number?
If you are running the app on a device, you can set a fake serial number in the `DeviceAPI::DeviceAPI()` function:
```C++
//some codes
#ifdef __unix__
    // _uid = UtilityHelper::getCPUInfo().toStdString();
    _uid = "123456a1a0123456"; 
#else
 //some codes
```

If you are running the app on your PC, you can set a fake serial number in the `DeviceConfig::load()` function:
```C++
    // uid = config.value("uid").toString().toStdString();
    uid = "110879d4d9642249";
```
> **_WARNING:_** Ensure your fake serial number is not a valid serial number connected to a real device. Any changes you make in the app will appear on the device.

## Devloper Mode
If you want to test the UI without needing a valid serial number or connecting to a device, you can set `userLevel` to `DEVELOPER` in `UiSession.qml`:
```QML
    // property int userLevel: UiSession.UserLevel.USER
    property int userLevel: UiSession.UserLevel.DEVELOPER
```
## Initial setup
> **_NOTE:_** This section is only for developers.

### What is initial setup exactly?
The initial setup flow involves configuring several settings for your device. These include:
- Selecting the system type:
    - Traditional 
    - Heat Pump
    - Cool only
    - Heat Only
- Configuring the settings for the chosen system type.
- Specifying the accessories your device has.
You can select between a humidifier, dehumidifier, or none. Each accessory may have additional options.
- Setting the system run delay.
- Enabling technician access, which displays a QR code. By scanning the QR code, you can view information about the device after logging into your [nuvehvac.com](https://https://www.nuvehvac.com/#EN/USA/user/login/) account.

### How to Run Initial Setup Automatically
If your app has a valid serial number and the variable `hasClient` is set to `false`, the app will automatically run in initial setup mode.

### How to Run Initial Setup Manually

> **_NOTE_**: Make sure your app has a serial number. If you are testing the app on Windows, refer to the **How to Fake a Serial Number** section.

1. Comment the line in `NUVE::DeviceConfig::load()`:
    ```C++
    // serial_number = config.value("serial_number").toString().toStdString();
    ```

1. Comment the line in the Sync Class Constructor:
    ```C++
    // mHasClient = setting.value(m_HasClientSetting).toBool();
    ```

2. Set `mHasClient` to `false` in the Sync Class:
    ```C++
    // some codes
    auto sn = dataObj.value("serial_number").toString();
    // mHasClient = dataObj.value("has_client").toBool();
    mHasClient = false;
    // some codes
    ```
3. Set `startMode` in `DeviceControllerCPP::startDevice()` function to whatever except 0.
    ```C++
    void DeviceControllerCPP::startDevice()
    {
        // some codes
        // int startMode = getStartMode();
        int startMode = 1;
        emit startModeChanged(startMode);
        // some codes
    }
    ```

> **_TODO:_** Let's make it easier.

## Test mode
### What it is Test mode exactly?

### How to move the app to the test mode?