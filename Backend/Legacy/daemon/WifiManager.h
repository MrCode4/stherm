#pragma once
#include <string>
#include <map>
#include <vector>
#include <unistd.h>
#define DEBUG

#include <iostream>


//struct parse_struct
//{
//    std::string name;
//    std::string ssid;
//    std::string ssid_hex;
//    std::string bssid;
//    std::string mode;
//    std::string chan;
//    std::string freq;
//    std::string rate;
//    std::string rate;
//    std::string signal;
//    std::string bars;
//    std::string security;
//    std::string wpa_flags;
//    std::string rsn_flags;
//    std::string device;
//    std::string active;
//    std::string in_use;
//    std::string dbus_path;
//};
class WifiManager
{
public:
    int get_wifi_list();
    int get_wifi_state(std::string name);
    int wifi_connect(std::vector <std::string>& wifi_params);
    int wifi_disconnect(std::string name);
    int wifi_forget(std::string name);
    int wifi_con_up(std::string name);
    int get_active_wifi();
    int get_wifi_connections();
    int delete_all_con();
    int bssid_to_ssid(std::string& bssid);
    int write_wifi_signal();
    int wifi_manual_connect(std::vector<std::string>& args);
    enum ERROR
    {
        ERROR_NO,// Success – indicates the operation succeeded.
        ERROR_UNKNOWN,//Unknown or unspecified error.
        ERROR_INPUT,//Invalid user input, wrong nmcli invocation.
        ERROR_TIMEOUT,//Timeout expired.
        ERROR_ConActFail,//Connection activation failed.
        ERROR_ConDactFail,// Connection deactivation failed.
        ERROR_DiscDevFail,//Disconnecting device failed.
        ERROR_ConDelFail,// Connection deletion failed.
        ERROR_NetMng,//NetworkManager is not running.
        ERROR_NotExsist=10,//Connection, device, or access point does not exist.
        ERROR_WRONG_BSSID,
        ERROR_WRONG_NAME,
        ERROR_WRONG_PASS,
        ERROR_NO_ACTIVE_WIFI,
        ERROR_WRONG_ARGUMENTS,
        ERROR_CRITICAL_DELETE,
        ERROR_INTERNAL_ERROR,
        ERROR_CONNECTION_ALREADY_EXISTS,
        ERROR_INTERNAL_ERROR_PARSING,
        ERROR_WRONG_SUBNET,
        ERROR_WRONG_IP,
        ERROR_WRONG_ESSID,
        
    };  
    enum Manual_connection
    {
        essid,
        ip,
        subnet,
        gtw,
        dns,
        dns2,
        pass
    };

private:
 
    enum WifiSearchWord_e
    {
        name,
        ssid,
        ssid_hex,
        bssid,
        mode,
        chan,
        freq,
        rate,
        signal,
        bars,
        security,
        wpa_flags,
        rsn_flags,
        device,
        active,
        in_use,
        dbus_path
    };
 
    typedef std::pair<std::string, std::vector<std::string>> pair;
    bool get_data(std::vector<std::string>& data, const char set_call[]);
    void removeSpacesFromBack(std::string& data);
    void find_words(std::vector<std::string>& data, const std::vector<std::string> findable_words, std::vector<size_t>& start_pos_vector, std::vector<size_t>& end_pos_vector);
    int parser(const char set_call[], const std::vector<std::string> findable_words, std::map <std::string, std::vector<std::string>>& base_map,int key);
  
    const std::string m_dev_nul = " > /dev/null 2>&1";
};

