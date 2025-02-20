#include <cstdio>
#include<iostream>
#include <string.h>
#include <vector>
#include"WifiManager.h"
#include"InputParametrs.h"

using namespace std;

void my_exit()
{
    //std::cout << "CRITI" << std::endl;
}


/**
 * @brief Main function for the Wi-Fi management program
 *
 * This function initializes an InputParametrs object with the given command-line
 * arguments and then calls the executeCommand() method to perform the specified
 * Wi-Fi management operation.
 *
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return int Program exit code (0 for success, non-zero for failure)
 */
int main(int argc, char** argv)
{
  
    InputParametrs parameters(argv, argc);
    parameters.executeCommand();
}

