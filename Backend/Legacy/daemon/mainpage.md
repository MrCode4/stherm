/**
 @mainpage HVAC Daemon Documentation
 @section introduction Introduction
 
 The `hvac_daemon` is the primary application on Linux responsible for communication with TI and NRF boards, as well as the web component of the device.
 The executable should be located in the `/home/root` directory with the necessary execution rights.
 Logs are written to `/var/log/messages`.
 
 @section compilation Compiling the Source Code
 
 To compile the source code, use the following command:
 
 @code
  g++ main.cpp \
      Daemon_helper.cpp \
      DeltaCorrection.cpp \
      InputParameters.cpp \
      Peripheral.cpp \
      WifiManager.cpp \
      crc.c \
      php_interface.cpp \
      serial_drv.c \
      DaemonStatus.cpp \
      -lpthread \
      -o hvac_daemon
  @endcode
 
 @section systemctl Systemctl Setup
 
 The `hvac_daemon` is a daemon and should be managed using the `systemctl` utility. Set up the `hvac_daemon` by creating a file named `hvac_daemon.service`
 at `/etc/systemd/system/hvac_daemon.service` with the following content:
 
 @code
  [Unit]
  Description=HVAC daemon service
 
  [Service]
  Type=forking
  PIDFile=/run/hvac_daemon.pid
  ExecStart=/home/root/hvac_daemon --log-level=all
  User=root
  ExecStop=/bin/kill -TERM $MAINPID
  KillMode=none
  TimeoutStopSec=60
 
  [Install]
  WantedBy=multi-user.target
  @endcode
 
 The arguments for `ExecStart` could be:
 - `--log-level=all` (includes all levels including LOG_DEBUG)
 - `--log-level=errors` (includes all levels including LOG_ERR)
 - `--log-level=log_info` (includes all levels including LOG_INFO)
 If no arguments are provided, the default log level will be (errors) LOG_INFO.
 
 After saving the file, run the following commands:
  @code
  systemctl enable hvac_daemon
  systemctl start hvac_daemon
  @endcode
 For applying any changes in file `/etc/systemd/system/hvac_daemon.service` run the following command:  
 @code
 systemctl daemon-reload
 @endcode
 
 @section description Main Description
 
 The `hvac_daemon` consists of four threads:
 1. main
 2. dynamic
 3. ti
 4. nrf
 
 The main thread communicates with the dynamic, ti, and nrf threads using pipes.
 The dynamic thread fetches data from the web backend every second and sends it to the main thread for parsing and further processing.
 The nrf thread receives commands from the main thread, sends them to the NRF board, and returns the board's response to the main thread.
 The nrf thread also receives sensor data from NRF every 5 seconds.
 Similarly, the ti thread follows the same communication pattern as the nrf thread.
 
 @section communication Communication with Boards
 
 Communication with NRF and TI boards uses a packet-based logic. Each packet includes a command code and data payload.
 Refer to the list of commands in the enumerations of \ref serial_drv.h.
 
 @section logging Logging
 
 The daemon application logs messages to the `/var/log/messages` file based on the log level value. The log levels are categorized as follows:
 
 1. Errors:
    - Invalid command line arguments
    - Invalid PHP calls
    - Invalid data received from PHP call
    - Errors related to software or hardware version files
    - Invalid CRC sum received from the device
    - Errors in packets received from devices
    - Errors during the initialization of UARTs and pins
    - Pipe errors
    - Threads that have stopped working
    - Invalid delta in temperature correction logic
 
 2. Log Information (loginfo):
    - At the start of working with the web application
    - Information about paired devices
    - Hardware and software version details
    - States of input relays
    - Notifications during the pairing of a new sensor
    - Logging when clearing the cache
    - Logging changes in backlight RGB
 
 3. Debug Information (debug):
    - Delta values in temperature correction logic
    - Changes in backlight state within the temperature correction logic
    - Current states of devices
 
 Utilizing these log levels enables effective tracking and troubleshooting of various aspects of the daemon application.
 Proper logging enhances the application's reliability and aids in identifying and resolving issues efficiently.
 
 @section exit_program Exit from the Application
 
 When the program receives kill signals from Linux (SIGHUP or SIGTERM), it handles them by closing all pipes, processes, and threads.
 
 @section additional_functionality Additional Functionality
 
 The application includes three additional classes:
 1. Temperature Correction: This class corrects the temperature received from NRF based on backlight information.
 2. Daemon Status: The daemon continuously monitors the status of TI, NRF, and the backend. Every 10 seconds, it logs the status (LOG_DEBUG level) of devices.
    It checks for received data from NRF, TI, and the web backend.
 3. WifiManager: Used to update wifi signal levels every 20 seconds.


@section intro_sec Introduction
 This documentation provides details about the firmware for [Product Name].
 It includes information about the firmware version, release date, and revision history.
 
  @section version_sec Firmware Version
  - Firmware Version: 1.0
  - Release Date: 25.07.2023
 
  @section revision_sec Revision History
  - [25.07.2023]: Initial release (Version 1.0)
 
  @section author_sec Author
    -       Documentation Author: Aleksandr Menshikh

    - Organization: NS5 LLC
        
    - Contact: aleksandr.menshikh@ns5.tech

