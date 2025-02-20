/**
 * @file cpuinfo_serial_extractor.cpp
 * @brief This file contains a program to extract the serial number from the /proc/cpuinfo file.
 *
 * The program reads the /proc/cpuinfo file and extracts the serial number of the CPU.
 * The extracted serial number is then printed to the console.
 */
#include <cstdio>
#include<vector>
#include<string>
#include<iostream>

 /**
  * @brief Retrieve data from the command output and store it in a vector of strings.
  *
  * @param[out] data A reference to a vector of strings to store the retrieved data.
  * @param[in] set_call A C-style string containing the command to execute.
  * @return bool Returns true if the operation is successful, false otherwise.
  */
bool get_data(std::vector<std::string>& data, const char set_call[])
{
    data.clear();
    FILE* fp;
    char line[512];
    fp = popen(set_call, "r");
    if (fp == NULL)
        return false;
    while (fgets(line, 512, fp) != NULL)
    {
        data.push_back(std::string(line));
    }
    pclose(fp);
    return true;
}
/**
 * @brief Main function for extracting and printing the serial number from the /proc/cpuinfo file.
 *
 * @return int Returns true (1) on successful execution, false (0) otherwise.
 */
int main()
{
    const char* set_call = "cat /proc/cpuinfo";
    std::vector < std::string> data;
    // Retrieve data from the command output
    get_data(data, set_call);
    // Iterate through the data vector to find the serial number
    for (int i = 0; i < data.size(); i++)
    {

        if ((data[i].find("Serial")) != std::string::npos)
        {
            size_t pos;
            if((pos=data[i].find(":"))!= std::string::npos)
            {
                // Extract the serial number and print it to the console
                data[i].erase(0, pos + 2);
                std::cout << data[i];
                return true;
            }
            else
            {
                // Serial number not found, print -1
                std::cout << "-1";
                return false;
            }
        }
    }
    // Serial number not found, print -1
    std::cout << "-1";
    return false;

        
}