cmake_minimum_required(VERSION 3.19)

project(hvac_daemon)

add_library(lowlevellib Peripheral.cpp)

add_executable(hvac_daemon 
	main.cpp
	Daemon_helper.cpp
    DeltaCorrection.cpp
    InputParametrs.cpp
    WifiManager.cpp
    crc.c
    php_interface.cpp
    serial_drv.c
    DaemonStatus.cpp)
target_link_libraries(hvac_daemon pthread)
target_link_libraries(hvac_daemon lowlevellib)
