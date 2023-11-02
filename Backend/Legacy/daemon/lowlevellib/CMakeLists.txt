add_library(lowlevellib Peripheral.cpp serial_drv.c crc.c)
target_include_directories(lowlevellib INTERFACE ${CMAKE_CURRENT_LIST_DIR})
