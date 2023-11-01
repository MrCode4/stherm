// HvAc_Wifi_Manual.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <string>
#include <stdlib.h>
#include <fstream>

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

enum IP
{
    IP0 = 0,
    IP128 = 128,
    IP192 = 192,
    IP224 = 224,
    IP240 = 240,
    IP248 = 248,
    IP252 = 252,
    IP254 = 254,
    IP255 = 255

};
int main(int argc, char** argv)
{
    string hvac = "HVAC";
    string essid;
    string pass;
    string ip;
    string subnet;
    string gtw;
    string dns;
    string dns2;
    string subnet_part;
    int subnet_array[4] = { 0 };

    if (argc < 2 || argc >8)
    {
        cout << USAGE_ERROR << endl;
        return 0;
    }


    else if (argc == 2)
    {
        for (int i = 0; argv[1][i] != '\0'; i++)
        {
            essid.push_back(argv[1][i]);
        }
        string cmmd = "nmcli d wifi connect '" + essid + "'";
        string otp = " > /tmp/wifi_connection.txt  2>&1";
        cmmd = cmmd + otp;
        system(cmmd.c_str());
        cmmd = "echo " + essid + " > /tmp/wifi_essid.txt";
        system(cmmd.c_str());
        string file_name = "/tmp/wifi_connection.txt";
        ifstream myfile;
        string line;
        string error = "Error: ";
        string conected = "Device 'wlan0' successfully activated with";
        myfile.open(file_name.c_str());
        if (myfile.is_open())
        {
            while (getline(myfile, line))
            {
                bool equal = true;
                if (line.find(error, 0) == line.npos)
                {
                    if (line.find(conected, 0) == line.npos)
                    {
                        cout << INTERNAL_ERROR << endl;
                    }
                    else
                    {
                        cout << ERROR_NO << endl;
                    }
                }
                else
                {
                    cout << line << endl;
                }

            }
            myfile.close();
        }
        else
        {
            cout << INTERNAL_ERROR << endl;
        }
        return 0;
    }
    else if (argc == 3)
    {

        for (int i = 0; argv[1][i] != '\0'; i++)
        {
            essid.push_back(argv[1][i]);
        }
        for (int i = 0; argv[2][i] != '\0'; i++)
        {
            pass.push_back(argv[2][i]);
        }
        string cmmd = "nmcli d wifi connect '" + essid + "'" + " password " + pass;
        string otp = " > /tmp/wifi_connection.txt  2>&1";
        cmmd = cmmd + otp;
        system(cmmd.c_str());
        cmmd = "echo " + essid + " > /tmp/wifi_essid.txt";
        system(cmmd.c_str());
        string file_name = "/tmp/wifi_connection.txt";
        ifstream myfile;
        string line;
        string error = "Error: ";
        string conected = "Device 'wlan0' successfully activated with";
        myfile.open(file_name.c_str());
        if (myfile.is_open())
        {
            while (getline(myfile, line))
            {
                bool equal = true;
                if (line.find(error, 0) == line.npos)
                {
                    if (line.find(conected, 0) == line.npos)
                    {
                        cout << INTERNAL_ERROR << endl;
                    }
                    else
                    {
                        cout << ERROR_NO << endl;
                    }
                }
                else
                {
                    cout << line << endl;
                }

            }
            myfile.close();
        }
        else
        {
            cout << INTERNAL_ERROR << endl;
        }
        return 0;
    }
    else if (argc == 8)
    {

        for (int i = 0; argv[1][i] != '\0'; i++)
        {
            essid.push_back(argv[1][i]);

        }
        for (int i = 0; argv[2][i] != '\0'; i++)
        {
            pass.push_back(argv[2][i]);
        }
        for (int i = 0; argv[3][i] != '\0'; i++)
        {
            ip.push_back(argv[3][i]);
        }
        for (int i = 0; argv[4][i] != '\0'; i++)
        {
            subnet.push_back(argv[4][i]);
        }
        subnet.push_back('.');
        int a = 0;
        for (int i = 0; i < subnet.length(); i++)
        {
            if (subnet[i] != '.')
            {
                subnet_part.push_back(subnet[i]);
            }
            else
            {
                subnet_array[a] = atoi(subnet_part.c_str());
                a++;
                subnet_part = "";
            }
        }
        // cout << subnet << endl;
        int subnet_int = 0;
        for (int i = 0; i < 4; i++)
        {
            switch (subnet_array[i])
            {
            case IP0:
                subnet_int = subnet_int + 0;
                break;
            case IP128:
                subnet_int += 1;
                break;
            case IP192:
                subnet_int = subnet_int + 2;
                break;
            case IP224:
                subnet_int = subnet_int + 3;
                break;
            case IP240:
                subnet_int = subnet_int + 4;
                break;
            case IP248:
                subnet_int = subnet_int + 5;
                break;
            case IP252:
                subnet_int = subnet_int + 6;
                break;
            case IP254:
                subnet_int = subnet_int + 7;
                break;
            case IP255:
                subnet_int = subnet_int + 8;
                break;
            default:
                cout << USAGE_ERROR << endl;
                break;
            }
        }
        subnet = to_string(subnet_int);
        //cout << subnet << endl;

        for (int i = 0; argv[5][i] != '\0'; i++)
        {
            gtw.push_back(argv[5][i]);
        }
        for (int i = 0; argv[6][i] != '\0'; i++)
        {
            dns.push_back(argv[6][i]);
        }
        for (int i = 0; argv[7][i] != '\0'; i++)
        {
            dns2.push_back(argv[7][i]);
        }
        string cmmd = "nmcli con add con-name " + hvac + " ifname wlan0 type wifi ssid '" + essid + "' ip4 " + ip + "/"+subnet + " gw4 "+gtw;
        string otp = " > /tmp/wifi_connection.txt  2>&1";
        //cout << cmmd << endl;
        system(cmmd.c_str());
        cmmd = "nmcli con modify " + hvac + " wifi-sec.key-mgmt wpa-psk";
        //cout << cmmd << endl;
        system(cmmd.c_str());
        cmmd = "nmcli con modify " + hvac + " wifi-sec.psk " + pass;
        //cout << cmmd << endl;
        system(cmmd.c_str());
        cmmd = "nmcli con modify " + hvac + " ipv4.dns \"" + dns + " " + dns2 + "\"";
        //cout << cmmd << endl;
        system(cmmd.c_str());
        cmmd = "nmcli con up " + hvac ;
        cmmd = cmmd + otp;
        //cout << cmmd << endl;
        system(cmmd.c_str());       
        cmmd = "echo " + essid + " > /tmp/wifi_essid.txt";
        system(cmmd.c_str());
        string file_name = "/tmp/wifi_connection.txt";
        ifstream myfile;
        string line;
        string hvac = "HVAC";
        string error = "Error: ";
        string conected = "Connection successfully activated ";
 
        myfile.open(file_name.c_str());
        if (myfile.is_open())
        {
            while (getline(myfile, line))
            {
                bool equal = true;
                if (line.find(error, 0) == line.npos)
                {
                    if (line.find(conected, 0) == line.npos)
                    {
                        cout << INTERNAL_ERROR << endl;
                    }
                    else
                    {
                        cout << ERROR_NO << endl;
                    }
                }
                else
                {
                    cout << line << endl;
                }

            }
            myfile.close();
        }
        else
        {
            cout << INTERNAL_ERROR << endl;
        }
        return 0;


    }
    return 0;


}