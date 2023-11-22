// HvAc_Brigtness_Set.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <stdlib.h>
#include <string>
#include <iostream>
#include<fstream>
using namespace std;
typedef enum SIO_Errors {
    ERROR_NO = 0x00,
    ERROR_01,         // Error: I2C_BUS
    ERROR_02,          // Error: Temperature/Humidity is not updated
    ERROR_GPIO_INIT,
    ERROR_UNKNOWN_COMMAND,
    ERROR_CRC,
    ERROR_RELAY_NOT_FOUND,
    USAGE_ERROR,//programm call argument count error
    ARGUMENT_ERROR,//programm call argument expression error
    RESOURCE_BUSY_ERROR,//can not open recource associated with the programm
    INTERNAL_ERROR,//error on the side of microcontrollers 
    WRONG_ESSID, // essid wrong
    WRONG_PASSWORD // password wrong


} SIO_Errors_t;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cout << USAGE_ERROR << endl;
        return 1;
    }

    string brigtness_level;
    for (int i = 0; argv[1][i] != '\0'; i++)
    {
        brigtness_level.push_back(argv[1][i]);
    }
    int i = std::stoi(brigtness_level);
    if (i < 0 || i>100)
    {
        cout << ARGUMENT_ERROR << endl;
    }
    
    else
    {
        string cmmd = "echo " + to_string(i/4) + " > /sys/class/backlight/backlight_display/brightness";
        system(cmmd.c_str());
        cout << cmmd << endl;
        cout << ERROR_NO << std::endl;
    }
    return 0;
}
