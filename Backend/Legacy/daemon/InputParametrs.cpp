#include "InputParametrs.h"
#include"def_values.h"
#include <unistd.h>

int InputParametrs::executeCommand()
{
    int err_code;
    int tmp_err_code;
    std::vector<std::string> params;
    // Check if the arguments are valid.
    if (!get_arguments(ip_argv, ip_argc, ip_arguments))
    {
        std::cout << WifiManager::ERROR::ERROR_WRONG_ARGUMENTS << std::endl;
        return WifiManager::ERROR::ERROR_WRONG_ARGUMENTS;

    }
    // Execute the appropriate command.
    switch (get_command(ip_argv,ip_argc))
    {
    case COMMAND_SEARCH:
        print_errors(wifi_function.get_wifi_list());
        break;
    case COMMAND_MANUAL:
        err_code=manual_connect_arguments(params, ip_arguments[0]);
        if (err_code != 0)
        {
            std::cout << err_code << std::endl;
            return err_code;
        }
        err_code=wifi_function.wifi_manual_connect(params);
        // wifi_function.write_wifi_signal();
        if (err_code != WifiManager::ERROR::ERROR_NO && err_code != WifiManager::ERROR::ERROR_CONNECTION_ALREADY_EXISTS
            && err_code != WifiManager::ERROR::ERROR_WRONG_ARGUMENTS)
        {
            //need to delete connection because it created
            if (wifi_function.wifi_forget(params[essid]) != 0)
            {
                std::cout << WifiManager::ERROR::ERROR_INTERNAL_ERROR << std::endl;
                //if (!wifi_function.delete_all_con())
                //{
                //    WifiManager::ERROR::ERROR_INTERNAL_ERROR;
                //    assert(0 && "CRITICAL ERROR");
                //    //return WifiManager::ERROR::ERROR_CRITICAL_DELETE;
                //}
                //return WifiManager::ERROR::ERROR_CRITICAL_DELETE;
                exit(0);
                //assert(0 && "CRITICAL ERROR");
            }
            std::cout << err_code << std::endl;
            break;
        }
        if (err_code == WifiManager::ERROR::ERROR_NO)
        {
            wifi_function.write_wifi_signal();
        }
        std::cout << err_code << std::endl;
        break;
    case COMMAND_CONNECT:
        err_code = wifi_function.wifi_connect(ip_arguments);
        // wifi_function.write_wifi_signal();
        //sleep(3);
        if (err_code != WifiManager::ERROR::ERROR_NO && err_code != WifiManager::ERROR::ERROR_CONNECTION_ALREADY_EXISTS 
            && err_code != WifiManager::ERROR::ERROR_WRONG_ARGUMENTS)
        {
            //need to delete connection because it created
            int forget_result = wifi_function.wifi_forget(ip_arguments[0]);
            // if 0 or ERROR_WRONG_NAME:
            //  return err_code (means connection wasnt create and error was on wifi_connect)
            // if not go return ERROR_INTERNAL_ERROR (something was wrong on forget process
            if (forget_result != 0 && forget_result != WifiManager::ERROR::ERROR_WRONG_NAME)
            {
                std::cout << WifiManager::ERROR::ERROR_INTERNAL_ERROR << std::endl;
                //if (!wifi_function.delete_all_con())
                //{
                //    WifiManager::ERROR::ERROR_INTERNAL_ERROR;
                //    assert(0 && "CRITICAL ERROR");
                //    //return WifiManager::ERROR::ERROR_CRITICAL_DELETE;
                //}
                //return WifiManager::ERROR::ERROR_CRITICAL_DELETE;
                exit(0);
                //assert(0 && "CRITICAL ERROR");
            }
            std::cout << err_code << std::endl;
            break;
        }
        if (err_code == WifiManager::ERROR::ERROR_NO)
        {
            wifi_function.write_wifi_signal();
        }
        std::cout << err_code << std::endl;
        break;
    case COMMAND_DISCONNECT:
        if(!print_errors(wifi_function.wifi_disconnect(ip_arguments[0])))
        {
            std::cout << WifiManager::ERROR::ERROR_NO << std::endl;
            wifi_function.write_wifi_signal();
        }
        break;
    case COMMAND_FORGET:
        if(!print_errors(wifi_function.wifi_forget(ip_arguments[0])))
        {
            std::cout << WifiManager::ERROR::ERROR_NO << std::endl;
            wifi_function.write_wifi_signal();
        }
        break;
    case COMMAND_SHOW_ACTIVE_CONNECTIONS:
        print_errors(wifi_function.get_active_wifi());
        break;
    case COMMAND_SHOW_CONNECTIONS:
        print_errors(wifi_function.get_wifi_connections());
        break;
    case COMMAND_DELETE_ALL:
        if (!print_errors(wifi_function.delete_all_con()))
        {
            std::cout << WifiManager::ERROR::ERROR_NO << std::endl;
            wifi_function.write_wifi_signal();
        }
        break;
    default:
        std::cout << WifiManager::ERROR::ERROR_UNKNOWN << std::endl;
        return WifiManager::ERROR::ERROR_UNKNOWN;
    }
    return 0;

}
bool InputParametrs::get_arguments(char** argv, int argc, std::vector<std::string>& all_argunments)
{
    std::string argument;
    if (check_arg_count(argv,argc))
    {
        // Loop through the command line arguments, skipping the first two (program name and command)
        for (int arguments = 2; arguments < argc; arguments++)
        {
            // Construct the argument string
            for (int i = 0; argv[arguments][i] != '\0'; i++)
            {
                argument.push_back(argv[arguments][i]);
            }
            all_argunments.push_back(argument);
            argument.clear();
        }
        return true;
    }
    return false;
}
int InputParametrs::get_command(char** argv, int argc)
{
    if (argc < ARGUMENT_MIN_COUNT)
    {
        return WifiManager::ERROR::ERROR_WRONG_ARGUMENTS;
    }
    // Construct the command string from the input arguments
    std::string cmd_str;
    for (int i = 0; argv[1][i] != '\0'; i++)
    {
        cmd_str.push_back(argv[1][i]);
    }
    // Compare the command string with the predefined command array
    for (int i = 0; i < my_arr_size; i++)
    {
        if (cmd_str == my_cmd_array[i])
        {
            //return COMMANDS(i);
            return i + 100;
        }
    }

    return WifiManager::ERROR::ERROR_WRONG_ARGUMENTS;
}
bool InputParametrs::check_arg_count(char** argv, int argc)
{
    int cmd=get_command(argv, argc);
    switch (cmd)
    {
    case COMMAND_SEARCH:
        if (argc==2)
        {
            return true;
        }
        return false;
    case COMMAND_MANUAL:
        if (argc==3)
        {
            return true;
        }
        return false;
    case COMMAND_CONNECT:
        if (argc>2 && argc<8)
        {
            return true;
        }
        return false;
    case COMMAND_DISCONNECT:
        if (argc == 3)
        {
            return true;
        }
        return false;
    case COMMAND_FORGET:
        if (argc == 3)
        {
            return true;;
        }
        return false;
    case COMMAND_SHOW_ACTIVE_CONNECTIONS:
        if (argc == 2)
        {
            return true;
        }
        return false;
    case COMMAND_SHOW_CONNECTIONS:
        if (argc == 2)
        {
            return true;
        }
        return false;
    case COMMAND_DELETE_ALL:
        if (argc == 2)
        {
            return true;
        }
        return false;
    default:
        return false;
        break;
    }

}
int InputParametrs::print_errors(int err_code)
{
    if (err_code != WifiManager::ERROR::ERROR_NO)
    {
        std::cout << err_code << std::endl;
        return err_code;
    }
    return err_code;
  /*  switch (err_code)
    {
    case WifiManager::ERROR_NO: std::cout << std::endl << "Success – indicates the operation succeeded" << std::endl; break;
    case WifiManager::ERROR_UNKNOWN: std::cout << std::endl << "Unknown or unspecified error." << std::endl; break;
    case WifiManager::ERROR_INPUT: std::cout << std::endl << "Invalid user input, wrong nmcli invocation" << std::endl; break;
    case WifiManager::ERROR_TIMEOUT: std::cout << std::endl << "Timeout expired" << std::endl; break;
    case WifiManager::ERROR_ConActFail: std::cout << std::endl << "Connection activation failed" << std::endl; break;
    case WifiManager::ERROR_ConDactFail: std::cout << std::endl << "Connection deactivation failed" << std::endl; break;
    case WifiManager::ERROR_DiscDevFail: std::cout << std::endl << "Disconnecting device failed" << std::endl; break;
    case WifiManager::ERROR_ConDelFail: std::cout << std::endl << "Connection deletion failed" << std::endl; break;
    case WifiManager::ERROR_NetMng: std::cout << std::endl << "NetworkManager is not running" << std::endl; break;
    case WifiManager::ERROR_NotExsist: std::cout << std::endl << "Connection, device, or access point does not exist" << std::endl; break;
    case WifiManager::ERROR_WRONG_BSSID: std::cout << std::endl << "Wrong BSSID/uniq_name" << std::endl; break;
    case WifiManager::ERROR_WRONG_NAME: std::cout << std::endl << "Wrong essid" << std::endl; break;
    case WifiManager::ERROR_WRONG_PASS: std::cout << std::endl << "Wrong Password" << std::endl; break;
    case WifiManager::ERROR_NO_ACTIVE_WIFI: std::cout << std::endl << "Wrong _NO_ACTIVE_WIF" << std::endl; break;
    case WifiManager::ERROR_WRONG_ARGUMENTS: std::cout << std::endl << "Wrong _NO_ACTIVE_WIF" << std::endl; break;
    case WifiManager::ERROR_CRITICAL_DELETE: std::cout << std::endl << "Wrong _NO_ACTIVE_WIF" << std::endl; break;

    default:
        assert(0 && "It can't be, but it happened contact the developer https://t.me/Gorynych_G")
    }*/
}
int InputParametrs::manual_connect_arguments(std::vector<std::string>& args, std::string& data)
{
    //. / wifi.out manual "essid:LanTekIV ip:172.16.0.227 subnet:255.255.255.0 gtw:172.16.0.1 dns:8.8.8.8 dns2:4.4.4.4 pass:IdealNetworks123*"

    // Default values for optional parameters
    std::string params_default_values[7] = { "","","/24","","8.8.8.8","4.4.4.4","" };
    const std::vector<std::string> params_name_vector = { "essid:","ip:", "subnet:", "gtw:", "dns:", "dns2:","pass:" };
    std::vector<size_t> start_pos_vector;
    std::vector<size_t> end_pos_vector;
    // Validate the input arguments
    if (!find_words(args, data, params_name_vector))
    {
        return WifiManager::ERROR::ERROR_WRONG_ARGUMENTS;
    }
    // Check ESSID
    if (args[essid].size() == 0)
    {
        return WifiManager::ERROR::ERROR_WRONG_ESSID;
    }
    // Check IP
    if (args[ip].size() == 0 || (!is_ip(args[ip])))
    {
        return WifiManager::ERROR::ERROR_WRONG_IP;
    }
    // Check subnet
    if (args[subnet].size() == 0)
    {
        args[subnet] = params_default_values[subnet];
    }
    else if (!is_ip(args[subnet]))
    {
        return WifiManager::ERROR::ERROR_WRONG_SUBNET;
    }
    else if(!subet_reformating(args[subnet]))
    {
        return WifiManager::ERROR::ERROR_WRONG_SUBNET;
    }
    // Check gateway
    if (args[gtw].size() == 0 || (!is_ip(args[gtw])))
    {
        return WifiManager::ERROR::ERROR_WRONG_ARGUMENTS;
    }
    // Check DNS
    if (args[dns].size() == 0)
    {
        args[dns] = params_default_values[dns];
    }
    else if (!is_ip(args[dns]))
    {
        return WifiManager::ERROR::ERROR_WRONG_ARGUMENTS;
    }
    // Check DNS2
    if (args[dns2].size() == 0)
    {
        args[dns2] = params_default_values[dns2];
    }
    else if (!is_ip(args[dns2]))
    {
        return WifiManager::ERROR::ERROR_WRONG_ARGUMENTS;
    }
    // Check password
    if (args[pass].size() == 0)
    {
        return WifiManager::ERROR::ERROR_WRONG_PASS;
    } 
    return 0;
}
bool InputParametrs::is_ip(std::string ip)
{
    if (ip.size() != 0)
    {

        size_t first_pos = 0;
        std::string findable = ".";
        int counter = 0;
        std::string token;
        // Iterate through the IP address string and validate each octet
        while ((first_pos = ip.find(findable)) != std::string::npos)
        {
            token = ip.substr(0, first_pos);
            ip.erase(0, first_pos + findable.size());
            counter++;
            // Check if the octet is valid
            if (counter > 3 || std::stoi(token) < 0 || std::stoi(token) > 255)
            {
                return false;
            }
        }
        if (counter == 3)
        {
            return true;
        }
    }
    return false;
  

}
bool InputParametrs::subet_reformating(std::string& subnet)
{

    constexpr int MAX_IP_PARTS = 4;
    size_t first_pos = 0;
    std::string findable = ".";
    int n_subnet = 0;
    int array[4]{};
    // Extract and validate the subnet mask components
    for (int i = 0; i < MAX_IP_PARTS; i++)
    {
        if (i < 3)
        {
            if (first_pos = subnet.find(findable))
            {
                array[i] = std::stoi(subnet.substr(0, first_pos));
                subnet.erase(0, first_pos + findable.size());
            }
        }
        else
        {
            array[i] = std::stoi(subnet.substr(0, std::string::npos));
        }
        // Check if the subnet mask component is a valid value
        switch (array[i])
        {
        case 0:      n_subnet += SUBNET_0;       break;
        case 128:    n_subnet += SUBNET_128;     break;
        case 192:    n_subnet += SUBNET_192;     break;
        case 224:    n_subnet += SUBNET_224;     break;
        case 240:    n_subnet += SUBNET_240;     break;
        case 248:    n_subnet += SUBNET_248;     break;
        case 252:    n_subnet += SUBNET_252;     break;
        case 254:    n_subnet += SUBNET_254;     break;
        case 255:    n_subnet += SUBNET_255;     break;
        default:
            subnet.clear();
            return false;
            break;
        }

    }
    // Verify that the subnet mask components are in descending order
    for (int i = 0; i < 4; i++)
    {
        for (int j = i; j < 4; j++)
        {
            if (array[i] < array[j])
            {
                subnet.clear();
                return false;


            }

        }
    }
    // Validate the CIDR notation range
    if (n_subnet < 0 || n_subnet>32)
    {
        subnet.clear();
        return false;
    }
    // Convert the subnet mask to CIDR notation
    subnet = std::to_string(n_subnet);
    return true;

    
}
void InputParametrs::removeSpacesFromBack(std::string& data)
{
    for (int i = data.size() - 1; i >= 0; i--)
    {
        if (data[i] != ' ')
        {
            data = data.substr(0, i + 1);
            break;
        }
    }
}
bool InputParametrs::find_words(std::vector<std::string>& args, std::string& data, const std::vector<std::string> findable_words)
{
    size_t start_pos;
    size_t end_pos;
    std::string token;
    // Iterate through findable_words and search for each word in the data string
    for (int i = 0; i < findable_words.size(); i++)
    {
        if ((start_pos = data.find(findable_words[i])) != std::string::npos)
        {
            if (i == 0)
            {
                data.erase(0, start_pos + findable_words[i].size());
            }
            else if (i < findable_words.size() - 1)
            {
                token = data.substr(0, start_pos);
                removeSpacesFromBack(token);
                args.push_back(token);
                data.erase(0, start_pos + findable_words[i].size());
            }
            else
            {
                token = data.substr(0, start_pos);
                removeSpacesFromBack(token);
                args.push_back(token);
                data.erase(0, start_pos + findable_words[i].size());
                token = data.substr(0, std::string::npos);
                removeSpacesFromBack(token);
                args.push_back(token);
            }
        }
        else
        {
            args.clear();
            return false;
        }
    }
    return true;
}