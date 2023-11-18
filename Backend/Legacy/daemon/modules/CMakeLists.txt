find_package(Qt6 REQUIRED COMPONENTS Core)
set(CMAKE_AUTOMOC ON)

add_library(HVAC_Modules
        DaemonStatus.cpp
        Daemon_helper.cpp
        threads/NRFThread.cpp
        threads/TIThread.cpp
        threads/NRFThread.h
        threads/TIThread.h)
target_link_libraries(HVAC_Modules PUBLIC lowlevellib)
target_link_libraries(HVAC_Modules PUBLIC Qt::Core)
target_include_directories(HVAC_Modules PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}
        ${CMAKE_CURRENT_LIST_DIR}/threads)
