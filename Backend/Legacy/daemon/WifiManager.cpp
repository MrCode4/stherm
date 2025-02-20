#include "WifiManager.h"
#include <string>
#include <syslog.h>
#include <errno.h>
#include "string.h"
int WifiManager::parser(const char set_call[],const std::vector<std::string> findable_words, std::map <std::string, std::vector<std::string>>& base_map,int key)
{
    std::vector < std::string> data_v;
    if (get_data(data_v,set_call))
    {
        std::vector<std::string> line;
        std::vector<size_t> start_pos_vector;
        std::vector<size_t> end_pos_vector;
        find_words(data_v, findable_words, start_pos_vector, end_pos_vector);
        for (int i = 1; i < data_v.size(); i++)
        {
            for (int j = 0; j < start_pos_vector.size(); j++)
            {
                if (j <= start_pos_vector.size() - 1)
                {
                    line.push_back(data_v[i].substr(start_pos_vector[j], start_pos_vector[j + 1] - start_pos_vector[j]));
                    removeSpacesFromBack(line[j]);

                }
                else
                {
                    line.push_back(data_v[i].substr(start_pos_vector[j],std::string::npos));
                    removeSpacesFromBack(line[j]);
                }
            }
            base_map.insert(pair(line[key],line));
            line.clear();
        }

        return true;
    }
    return false;
}
int WifiManager::get_wifi_list()
{
        std::map <std::string, std::vector<std::string>> base_map;
        const std::vector < std::string> wifi_search = { "NAME","SSID", "SSID-HEX","BSSID","MODE","CHAN","FREQ","RATE","SIGNAL","BARS","SECURITY","WPA-FLAGS","RSN-FLAGS","DEVICE","ACTIVE","IN-USE","DBUS-PATH" };
        if (!parser("nmcli -f ALL dev wifi", wifi_search, base_map, 3))
        {
            return ERROR_INTERNAL_ERROR;
        }
        std::cout << "\'{\"data\":[";
        std::vector<std::string> keys;
        for (auto it = base_map.begin(); it != base_map.end(); it++) 
        {
            keys.push_back(it->first);
        }
        for (int i = 0; i < keys.size(); i++)
        {
            std::cout << "{";
            std::cout << "\"essid\":\"" << base_map.find(keys[i])->second[ssid] << "\",";
            std::cout << "\"unique_name\":\"" << keys[i] << "\",";
            if (base_map.find(keys[i])->second[active] == "yes")
            {
                std::cout << "\"connected\":true,";
            }
            else
            {
                std::cout << "\"connected\":false,";

            }
            if (base_map.find(keys[i])->second[security] != "--")
            {
                std::cout << "\"secret\":true,";
            }
            else
            {
                std::cout << "\"secret\":false,";
            }
            std::cout << "\"signal\":" << base_map.find(keys[i])->second[signal];
            if (i < keys.size() - 1)
            {
                std::cout << "},";
            }
            else
            {
                std::cout << "}";
            }
        }
        std::cout << "]}\'";
        std::cout << std::endl;
        return ERROR_NO;
    
}
int WifiManager::get_wifi_state(std::string name)
{
    if (name.size() != 0)
    {
        std::map <std::string, std::vector<std::string>> base_map;
        const std::vector < std::string> wifi_search = { "NAME","SSID", "SSID-HEX","BSSID","MODE","CHAN","FREQ","RATE","SIGNAL","BARS","SECURITY","WPA-FLAGS","RSN-FLAGS","DEVICE","ACTIVE","IN-USE","DBUS-PATH" };
        parser("nmcli -f ALL dev wifi", wifi_search, base_map,3);
        for (auto it = base_map.begin(); it != base_map.end(); it++)
        {
            if (it->second[ssid] == name && it->second[active]=="yes")
            {
                std::cout << it->second[signal];
                return ERROR_NO;
            }
        }
        //return ERROR_WRONG_NAME;
    }
    return ERROR_WRONG_NAME;
}
int WifiManager::wifi_connect(std::vector <std::string>& wifi_params)
{
    if (wifi_params.size() > 0)
    {
        std::string name = wifi_params[0];
        int err_code = 0;
        std::map <std::string, std::vector<std::string>> base_map;
        const std::vector < std::string> wifi_search = { "NAME","SSID", "SSID-HEX","BSSID","MODE","CHAN","FREQ","RATE","SIGNAL","BARS","SECURITY","WPA-FLAGS","RSN-FLAGS","DEVICE","ACTIVE","IN-USE","DBUS-PATH" };
        parser("nmcli -f ALL dev wifi", wifi_search, base_map, 3);
        for (auto it = base_map.begin(); it != base_map.end(); it++)
        {
            if (it->second[ssid] == name && it->second[active] == "yes")
            {

                return ERROR_CONNECTION_ALREADY_EXISTS;
            }
        }
        std::map <std::string, std::vector<std::string>> conection_map;
        const std::vector < std::string> wifi_con{ "NAME","UUID","TYPE","DEVICE" };
        parser("nmcli connection show", wifi_con, conection_map, 0);
        for (auto it = base_map.begin(); it != base_map.end(); it++)
        {
            if (it->second[ssid] == name)
            {

                if (conection_map.find(name) != conection_map.end())//we have this con
                {
                    err_code = wifi_con_up(name);
                    return err_code;
                }
                if (it->second[security] != "--")//need pass
                {
                    if (wifi_params.size() > 1)
                    {
                        if (wifi_params.size() == 2)
                        {
                            std::string pass = wifi_params[1];
                            std::vector<std::string> data;
                            std::string cmd = " nmcli d wifi connect '" + name + "'" + " password '" + pass + "'";
                            get_data(data, cmd.c_str());
                            std::string find_error = "Error:";
                            for (int i = 0; i < data.size(); i++)
                            {
                                if (data[i].find(find_error) != std::string::npos)
                                {
                                    return ERROR_WRONG_PASS;
                                }
                            }
                            return 0;
                        }
                        return ERROR_WRONG_ARGUMENTS;//TODO
                    }
                    else
                    {
                        return ERROR_WRONG_ARGUMENTS;
                    }
                }
                else
                {
                    if (wifi_params.size() == 1)//without pass
                    {
                        std::vector<std::string> data;
                        std::string cmd = " nmcli d wifi connect '" + name + "'";
                        get_data(data, cmd.c_str());
                        std::string find_error = "Error:";
                        for (int i = 0; i < data.size(); i++)
                        {
                            if (data[i].find(find_error) != std::string::npos)
                            {
                                return ERROR_WRONG_PASS;
                            }

                        }
                        return 0;
                    }
                    return ERROR_WRONG_ARGUMENTS;

                }
            }
        }






    }
    return ERROR_WRONG_ARGUMENTS;

}
int WifiManager::wifi_disconnect(std::string name)
{
    if (name.size()!=0)
    {
        std::map <std::string, std::vector<std::string>> base_map;
        const std::vector < std::string> wifi_search = { "NAME","SSID", "SSID-HEX","BSSID","MODE","CHAN","FREQ","RATE","SIGNAL","BARS","SECURITY","WPA-FLAGS","RSN-FLAGS","DEVICE","ACTIVE","IN-USE","DBUS-PATH" };
        if (!parser("nmcli -f ALL dev wifi", wifi_search, base_map, 3))
        {
            return ERROR_INTERNAL_ERROR;
        }
        for (auto it = base_map.begin(); it != base_map.end(); it++)
        {
            if (it->second[ssid] == name && it->second[active] == "yes")
            {

                std::string cmd = " nmcli c down '" + it->second[ssid] + "'" + m_dev_nul;
                int err_code = system(cmd.c_str());
                return err_code;
            }
        }
        //return ERROR_WRONG_BSSID;

    }
    return ERROR_WRONG_NAME;
    
}
int WifiManager::wifi_forget(std::string name)
{
    if (name.size() != 0)
    {
        std::map <std::string, std::vector<std::string>> base_map;
        const std::vector < std::string> wifi_search{ "NAME","UUID","TYPE","DEVICE" };
        parser("nmcli connection show", wifi_search, base_map,0);
        if (base_map.find(name) != base_map.end())
        {
            std::string cmd = " nmcli connection delete '" + name + "'" + m_dev_nul;
            int err_code = system(cmd.c_str());
            return err_code;
        }
        else
        {
            return ERROR_WRONG_NAME;
        }

    }
    return ERROR_WRONG_NAME;
}
int WifiManager::wifi_con_up(std::string name)
{
    std::map <std::string, std::vector<std::string>> conection_map;
    const std::vector < std::string> wifi_con{ "NAME","UUID","TYPE","DEVICE" };
    parser("nmcli connection show", wifi_con, conection_map, 0);
    if (conection_map.find(name) != conection_map.end())//ssid exsist
    {
        std::string cmd = " nmcli con up '" + conection_map.find(name)->second[0] + "'" + m_dev_nul;
        int err_code = system(cmd.c_str());
        return err_code;
    }
    return ERROR_WRONG_NAME;
}
int WifiManager::get_active_wifi()
{
    std::map <std::string, std::vector<std::string>> base_map;
    const std::vector < std::string> wifi_search = { "NAME","SSID", "SSID-HEX","BSSID","MODE","CHAN","FREQ","RATE","SIGNAL","BARS","SECURITY","WPA-FLAGS","RSN-FLAGS","DEVICE","ACTIVE","IN-USE","DBUS-PATH" };
    parser("nmcli -f ALL dev wifi", wifi_search, base_map, 3);
    std::vector<std::string> keys;
    for (auto it = base_map.begin(); it != base_map.end(); it++)
    {
        keys.push_back(it->first);
    }
    for (int i = 0; i < keys.size(); i++)
    {
        if (base_map.find(keys[i])->second[active] == "yes")
        {
            std::cout << keys[i];
            std::cout << std::endl;
            return ERROR_NO;
        }
    }
    return ERROR_NO_ACTIVE_WIFI;
   
}
int WifiManager::get_wifi_connections()
{
    std::map <std::string, std::vector<std::string>> conection_map;
    const std::vector < std::string> wifi_con{ "NAME","UUID","TYPE","DEVICE" };
    if (!parser("nmcli connection show", wifi_con, conection_map, 0))
    {
        return ERROR_INTERNAL_ERROR;
    }
    std::vector<std::string> keys;
    for (auto it = conection_map.cbegin(); it != conection_map.cend(); ++it)
    {
        keys.push_back(it->first);
    }
    for (int i = 0; i < keys.size(); i++)
    {
        std::cout << keys[i] << " ";
    }
    return ERROR_NO;

}
int WifiManager::delete_all_con()
{

    std::map <std::string, std::vector<std::string>> conection_map;
    const std::vector < std::string> wifi_con{ "NAME","UUID","TYPE","DEVICE" };
    parser("nmcli connection show", wifi_con, conection_map, 0);
    std::vector<std::string> keys;
    for (auto it = conection_map.cbegin(); it != conection_map.cend(); ++it)
    {
        keys.push_back(it->first);
    }
    for (int i = 0; i <keys.size(); i++)
    {
        if (!wifi_forget(keys[i]))
        {
            return ERROR_CRITICAL_DELETE;
        }

    }
    return ERROR_NO;
    

}
int WifiManager::bssid_to_ssid(std::string& bssid)
{
    std::map <std::string, std::vector<std::string>> base_map;
    const std::vector < std::string> wifi_search = { "NAME","SSID", "SSID-HEX","BSSID","MODE","CHAN","FREQ","RATE","SIGNAL","BARS","SECURITY","WPA-FLAGS","RSN-FLAGS","DEVICE","ACTIVE","IN-USE","DBUS-PATH" };
    sleep(3);//review later
    if (parser("nmcli -f ALL dev wifi", wifi_search, base_map, 3))
    {
        if (base_map.find(bssid) != base_map.end())
        {
            bssid = base_map.find(bssid)->second[ssid];
            return ERROR_NO;
        }
        return ERROR_WRONG_BSSID;

    }
    return ERROR_INTERNAL_ERROR_PARSING;
   
}
int WifiManager::write_wifi_signal()
{
    //const char set_call = "iwconfig wlan0 |grep Link";
    std::vector<std::string> data;
    get_data(data, "iwconfig wlan0");
    std::string findable_word = " Link Quality=";
    std::string findable_word2 = "Signal level";
    size_t first_pos = 0;
    size_t second_pos = 0;
    std::string word;
    for (int i = 0; i < data.size(); i++)
    {
        if ((first_pos = data[i].find(findable_word)) != std::string::npos)
        {
            if ((second_pos = data[i].find(findable_word2)) != std::string::npos)
            {
                word = data[i].substr(first_pos + findable_word.size(), second_pos - (first_pos+findable_word.size()));
                if ((first_pos = word.find("/")) != std::string::npos)
                {
                    int singal_level = std::stoi(word.substr(0, first_pos));
                    int singal_level_delimeter = stoi(word.erase(0, first_pos + 1));
                    std::string cmd = "php /usr/share/apache2/default-site/htdocs/engine/setWifiSignal.php " + std::to_string(singal_level*100 / singal_level_delimeter);
                    return system(cmd.c_str());
                }
            }
        }  
    }
    std::string cmd = "php /usr/share/apache2/default-site/htdocs/engine/setWifiSignal.php -1";
    return system(cmd.c_str());
}
int WifiManager::wifi_manual_connect(std::vector<std::string>& args)
{
    std::map <std::string, std::vector<std::string>> base_map;
    const std::vector < std::string> wifi_search = { "NAME","SSID", "SSID-HEX","BSSID","MODE","CHAN","FREQ","RATE","SIGNAL","BARS","SECURITY","WPA-FLAGS","RSN-FLAGS","DEVICE","ACTIVE","IN-USE","DBUS-PATH" };
    if (!parser("nmcli -f ALL dev wifi", wifi_search, base_map, 3))
    {
        return ERROR_INTERNAL_ERROR;
    }
    std::vector<std::string> keys;
    for (auto it = base_map.begin(); it != base_map.end(); ++it)
    {
        if (it->second[1] == args[0])
        {
            std::map <std::string, std::vector<std::string>> conection_map;
            const std::vector < std::string> wifi_con{ "NAME","UUID","TYPE","DEVICE" };
            if (!parser("nmcli connection show", wifi_con, conection_map, 0))
            {
                return ERROR_INTERNAL_ERROR;
            }
            std::string s_essid = it->second[1];
            if (conection_map.find(s_essid) == base_map.end())
            {
                return ERROR_CONNECTION_ALREADY_EXISTS;
            }
            usleep(500000);
            std::string cmd = "nmcli con add con-name " + s_essid + " ifname wlan0 type wifi ssid '" + s_essid + "' ip4 " + args[ip] + "/" + args[subnet] + " gw4 " + args[gtw] + m_dev_nul;
            int err_code = system(cmd.c_str());
            
            if (err_code != 0)
            {
                return err_code;
            }
            cmd = "nmcli con modify " + s_essid + " wifi-sec.key-mgmt wpa-psk"+ m_dev_nul;
            err_code = system(cmd.c_str());
            usleep(500000);
            if (err_code != 0)
            {
                return err_code;
            }
            cmd = "nmcli con modify " + s_essid + " wifi-sec.psk " + args[pass]+ m_dev_nul;
            err_code = system(cmd.c_str());
            usleep(500000);
            if (err_code != 0)
            {
                return err_code;
            }
            cmd = "nmcli con modify " + s_essid + " ipv4.dns \"" + args[dns] + " " + args[dns2] + "\""+ m_dev_nul;
            err_code = system(cmd.c_str());
            usleep(500000);
            if (err_code != 0)
            {
                return err_code;
            }
            cmd = "nmcli con up " + s_essid+ m_dev_nul;
            err_code = system(cmd.c_str());
            usleep(500000);
            if (err_code != 0)
            {
                return err_code;
            }
            return err_code;
        }
    }
    return ERROR_WRONG_ARGUMENTS;

}
bool WifiManager::get_data(std::vector<std::string>& data, const char set_call[])
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
void WifiManager::removeSpacesFromBack(std::string& data)
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
void WifiManager::find_words(std::vector<std::string>& data, const std::vector<std::string> findable_words, std::vector<size_t>& start_pos_vector, std::vector<size_t>& end_pos_vector)
{
    for (int i = 0; i < findable_words.size(); i++)
    {
        start_pos_vector.push_back(data[0].find(findable_words[i]));
        end_pos_vector.push_back(start_pos_vector[i] + findable_words[i].size());
    }
}
