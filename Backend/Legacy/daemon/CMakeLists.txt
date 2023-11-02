cmake_minimum_required(VERSION 3.19)

project(hvac_daemon)

add_subdirectory(lowlevellib)
add_subdirectory(modules)

add_executable(hvac_daemon 
	main.cpp
	Daemon_helper.cpp
    DeltaCorrection.cpp
    InputParametrs.cpp
    WifiManager.cpp
    php_interface.cpp)
target_link_libraries(hvac_daemon pthread)
target_link_libraries(hvac_daemon lowlevellib)
target_link_libraries(hvac_daemon HVAC_Modules)
