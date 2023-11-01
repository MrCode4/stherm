#include "php_interface.h"
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include <errno.h>
#include <fstream>
uint32_t hextoint(std::string a)
{
	if (a.length() != 8)
	{
		return 0xffffffff;
	}

	uint32_t ret = 0;
	int j = 0;
	uint8_t var = 0;
	for (char c : a)
	{
		if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))
		{
			if ((c >= '0') && (c <= '9'))
			{
				var = (c - 48);
			}
			else
				var = (static_cast<uint8_t>(tolower(c)) - 87);
			ret |= ((uint32_t)var) << (7 - j) * 4;
			j++;

		}
	}
	return ret;
}
char dev_id[16]{0};
bool get_paired_list(std::vector<device_t>& list)
{
	FILE* fp;
	rapidjson::Document doc;
	list.clear();
	device_t tmp_dev;
	char line[256];
	char getPaired_php[] = "php /usr/share/apache2/default-site/htdocs/engine/getPaired.php\0";
	fp = popen(getPaired_php, "r");
	if (fp == nullptr)
	{
		syslog(LOG_INFO, "%s", strerror(errno));
		return false;
	}
	fgets(line, 1024, fp);
	if (doc.ParseInsitu(line).HasParseError())
	{
		return false;
	}
	if (!doc.HasMember("data"))
	{
		return false;
	}
	if (!doc["data"].IsArray())
	{
		return false;
	}
	if (!doc["data"].Size())
	{
		return true;
	}
	for (uint  s = 0; s < doc["data"].Size(); s++)
	{
		if (!doc["data"][s].HasMember("external_sensor_id")
			|| !doc["data"][s].HasMember("external_sensor_type"))
		{
			return false;
		}
		if (!doc["data"][s]["external_sensor_id"].IsString() || !doc["data"][s]["external_sensor_type"].IsInt())
		{
			return false;
		}
		tmp_dev.address = hextoint(doc["data"][s]["external_sensor_id"].GetString());
		tmp_dev.type = static_cast<uint8_t>(doc["data"][s]["external_sensor_type"].GetInt());
		tmp_dev.paired = true;
		list.push_back(tmp_dev);
	}
	pclose(fp);
	return true;
}
bool get_thresholds_list(std::vector<config_thresholds>& tholds)
{
	FILE* fp;
	rapidjson::Document doc;
	config_thresholds tmp_thrld;
	char line[1024];
	char getTreshold_php[] = "php /usr/share/apache2/default-site/htdocs/engine/getTreshold.php\0";
	fp = popen(getTreshold_php, "r");
	if (fp == nullptr)
	{
		syslog(LOG_INFO, "%s", strerror(errno));
		return false;
	}
	fgets(line, 1024, fp);
	if (doc.ParseInsitu(line).HasParseError())
	{
		return false;
	}
	if (!doc.HasMember("data"))
	{
		return false;
	}
	if (!doc["data"].IsArray())
	{
		return false;
	}
	if (!doc["data"].Size())
	{
		return false;
	}
	for (uint s = 0; s < doc["data"].Size(); s++)
	{
		if (!doc["data"][s].HasMember("sensor_type")
			|| !doc["data"][s].HasMember("min_alert_value") || !doc["data"][s].HasMember("max_alert_value"))
		{
			return false;
		}
		if (!doc["data"][s]["sensor_type"].IsInt() || !doc["data"][s]["min_alert_value"].IsInt()
			|| !doc["data"][s]["max_alert_value"].IsInt())
		{
			return false;
		}
		tmp_thrld.sens_type= static_cast<uint8_t>(doc["data"][s]["sensor_type"].GetInt());
		if (tmp_thrld.sens_type >= SNS_NO_TYPE)
			return false;
		tmp_thrld.max_alert_value = static_cast<uint16_t>(doc["data"][s]["max_alert_value"].GetInt());
		tmp_thrld.min_alert_value = static_cast<uint16_t>(doc["data"][s]["min_alert_value"].GetInt());
		tholds.push_back(tmp_thrld);
	}
	pclose(fp);
	return true;
}
bool get_Config_list(std::vector<config_time>& time_cnfg)
{
	FILE* fp;
	rapidjson::Document doc;
	char line[1024];
	char getConfig_php[] = "php /usr/share/apache2/default-site/htdocs/engine/getConfig.php\0";
	config_time tmp_cfg;
	fp = popen(getConfig_php, "r");
	if (fp == nullptr)
	{
		syslog(LOG_INFO, "%s", strerror(errno));
		return false;
	}
	fgets(line, 1024, fp);
	if (doc.ParseInsitu(line).HasParseError())
	{
		return false;
	}
	if (!doc.HasMember("data"))
	{
		return false;
	}
	if (!doc["data"].IsArray())
	{
		return false;
	}
	if (!doc["data"].Size())
	{
		return false;
	}
	for (uint s = 0; s < doc["data"].Size(); s++)
	{
		if (!doc["data"][s].HasMember("external_sensor_type")
			|| !doc["data"][s].HasMember("poll_if_responded") || !doc["data"][s].HasMember("poll_if_not_responded")
			|| !doc["data"][s].HasMember("internal_poll_time"))
		{
			return false;
		}
		if (!doc["data"][s]["external_sensor_type"].IsInt() || !doc["data"][s]["poll_if_responded"].IsInt()
			|| !doc["data"][s]["poll_if_not_responded"].IsInt() || !doc["data"][s]["internal_poll_time"].IsInt())
		{
			return false;
		}
		tmp_cfg.ext_sens_type= static_cast<uint8_t>(doc["data"][s]["external_sensor_type"].GetInt());
		tmp_cfg.TT_if_ack= static_cast<uint8_t>(doc["data"][s]["poll_if_responded"].GetInt());
		tmp_cfg.TT_if_nack= static_cast<uint8_t>(doc["data"][s]["poll_if_not_responded"].GetInt());
		tmp_cfg.TP_internal_sesn_poll= static_cast<uint16_t>(doc["data"][s]["internal_poll_time"].GetInt());
		time_cnfg.push_back(tmp_cfg);
	}
	pclose(fp);
	return true;
}
//bool setWiring(std::vector<uint8_t>&wiring)
//{
//	if (wiring.size() != WIRING_IN_CNT)
//		return false;
//	char set_call[256];
//	FILE* fp;
//	char line[256];
//	char setWiring_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setWiring.php '{\"wiring_state\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]}'\0";//WIRING_IN_CNT
//	sprintf(set_call, setWiring_php, wiring[0], wiring[1], wiring[2], wiring[3], wiring[4], wiring[5], wiring[6], wiring[7], wiring[8], wiring[9]);
//	fp = popen(set_call, "r");
//	if (fp == nullptr)
//	{
//		syslog(LOG_INFO, "%s", strerror(errno));
//		return false;
//	}
//	while (fgets(line, 256, fp) != NULL)
//	{
//	}
//	pclose(fp);
//	return line[0] == '1';
//}
bool setAlert(uint32_t dev_id, uint8_t err_code, uint8_t level)
{
	char set_call[256];
	FILE* fp;
	char line[256];
	char setAlert_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setAlert.php '{\"device_id\":\"%4x\",\"error_code\":%d,\"level\":%d}'\0";
	sprintf(set_call, setAlert_php, dev_id,err_code,level);
	fp = popen(set_call, "r");
	if (fp == nullptr)
	{
		syslog(LOG_INFO, "%s", strerror(errno));
		return false;
	}
		
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
	syslog(LOG_INFO, "ALERT:: device_id:%4x, error_code:%d, level: %d}\n", dev_id, err_code, level);
	return line[0] == '1';
}
bool addPendingSensor(device_t& dev_one)
{
	char set_call[256];
	FILE* fp;
	char line[256];
	char addPendingSensor_php[] = "php /usr/share/apache2/default-site/htdocs/engine/addPendingSensor.php '{\"data\":[{\"external_sensor_id\":\"%4x\",\"external_sensor_type\":%d}]}'";
	sprintf(set_call, addPendingSensor_php, dev_one.address, dev_one.type);
	fp = popen(set_call, "r");
	if (fp == nullptr)
	{
		syslog(LOG_INFO, "%s", strerror(errno));
		return false;
	}
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
	return line[0] == '1';
}
bool setSensorData(device_t& dev, uint8_t* data, uint16_t size)
{
	char set_call[512];
	FILE* fp;
	char line[256];
	char setSensorData_Aqs_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setSensorData.php '{\"external_sensor_type\":%d, \"external_sensor_id\":\"%4x\",\"data\":{\"temperature\":%d, \"humidity\":%d, \"co2\":%d, \"etoh\":%d, \"tvoc\":%d, \"aiq\":%d, \"pressure\":%d}}'\0";
	char setSensorData_main_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setSensorData.php '{\"external_sensor_type\":%d, \"external_sensor_id\":\"%s\",\"data\":{\"temperature\":%d, \"humidity\":%d, \"co2\":%d, \"etoh\":%d, \"tvoc\":%d, \"aiq\":%d, \"pressure\":%d, \"tof\":%d, \"ambiend\":%d}}'\0";
	switch (dev.type)
	{
	case Main_dev:
		sprintf(set_call, setSensorData_main_php,dev.type, dev_id, ((int16_t*)data)[0], ((uint8_t*)(data+2))[0], ((uint16_t*)(data + 3))[0], ((uint16_t*)(data + 5))[0]/100, ((uint16_t*)(data + 7))[0]/1000, ((uint8_t*)(data + 9))[0]/10, ((uint16_t*)(data + 10))[0], ((uint16_t*)(data + 12))[0], ((uint32_t*)(data + 14))[0]);
		syslog(LOG_INFO, "'{\"external_sensor_type\":%d, \"external_sensor_id\":\"%s\",\"data\":{\"temperature\":%d, \"humidity\":%d, \"co2\":%d, \"etoh\":%d, \"tvoc\":%d, \"aiq\":%d, \"pressure\":%d, \"tof\":%d, \"ambiend\":%d}}'\n"
			, dev.type, dev_id, ((int16_t*)data)[0], ((uint8_t*)(data + 2))[0], ((uint16_t*)(data + 3))[0], ((uint16_t*)(data + 5))[0] / 100, ((uint16_t*)(data + 7))[0] / 1000, ((uint8_t*)(data + 9))[0] / 10, ((uint16_t*)(data + 10))[0], ((uint16_t*)(data + 12))[0], ((uint32_t*)(data + 14))[0]);
		break;
	case AQ_TH_PR:
		sprintf(set_call, setSensorData_Aqs_php, dev.type, dev.address, ((int16_t*)(data+3))[0], ((uint8_t*)(data + 5))[0], ((uint16_t*)(data + 6))[0], ((uint8_t*)(data + 1))[0], ((uint8_t*)(data))[0], ((uint8_t*)(data + 2))[0]/10, ((uint16_t*)(data + 8))[0]);
		syslog(LOG_INFO, "'{\"external_sensor_type\":%d, \"external_sensor_id\":\"%4x\",\"data\":{\"temperature\":%d, \"humidity\":%d, \"co2\":%d, \"etoh\":%d, \"tvoc\":%d, \"aiq\":%d, \"pressure\":%d}}'\n"
			, dev.type, dev.address, ((int16_t*)(data + 3))[0], ((uint8_t*)(data + 5))[0], ((uint16_t*)(data + 6))[0], ((uint8_t*)(data + 1))[0], ((uint8_t*)(data))[0], ((uint8_t*)(data + 2))[0] / 10, ((uint16_t*)(data + 8))[0]);
		break;
	default:
		break;
	}
	fp = popen(set_call, "r");
	if (fp == nullptr)
	{
		syslog(LOG_ERR, "%s", strerror(errno));
		return false;
	}
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
	return line[0] == '1';
}
bool set_fan_speed_INFO( uint16_t fan_speed)
{
	char set_call[512];
	FILE* fp;
	char line[256];
	char setfan_speed_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setVentilator.php %d\0";
	sprintf(set_call, setfan_speed_php, fan_speed);
	syslog(LOG_INFO, "fan_speed: %d\n",fan_speed);
	fp = popen(set_call, "r");
	if (fp == nullptr)
	{
		syslog(LOG_ERR, "%s", strerror(errno));
		return false;
	}
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
	return line[0] == '1';
}
bool set_brightnessData(int lvl)
{
	FILE* fp;
	char line[256];
	char setBrightness_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setBrightnessData.php %d\0";
	char set_call[256];
	sprintf(set_call, setBrightness_php, lvl);
	fp = popen(set_call, "r");
	if (fp == nullptr)
	{
		syslog(LOG_ERR, "%s", strerror(errno));
		return false;
	}
		
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
 	return line[0] == '1';
}
bool setBrightness( uint32_t Luminosity)
{
	Luminosity += 20;
	if (Luminosity > 100)
		Luminosity = 100;
	char set_call[256];
	FILE* fp;
	char line[256];
	char execute[] = "/usr/share/apache2/default-site/htdocs/engine/setBrightness %d\0";
	sprintf(set_call, execute, Luminosity);
	syslog(LOG_INFO, "SET_BRIGHTNESS %d", Luminosity);
	fp = popen(set_call, "r");
	if (fp == nullptr)
	{
		syslog(LOG_ERR, "%s", strerror(errno));
		return false;
	}
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
	return set_brightnessData(Luminosity) && line[0] == '0';
}
bool get_device_id()
{
	FILE* fp;
	char line[256];
	char execute[] = "/usr/share/apache2/default-site/htdocs/engine/getUid.o\0";
	fp = popen(execute, "r");
	if (fp == nullptr)
	{
		syslog(LOG_ERR, "%s", strerror(errno));
		return false;
	}
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
	memset(dev_id, 0, 16);
	memcpy(dev_id, line, 16);
	syslog(LOG_INFO, "%s\n", dev_id);
	return true;
}
bool get_dynamic(std::vector<uint8_t>& relays, bool& pairing, bool& update_paired_list, bool& wiring_check, rgb_vals& rgbm, uint8_t& brighness_mode, bool& browser_ok, uint8_t* line1,int size, bool& shutdownFromDynamic)
{
	rapidjson::Document doc;
	relays.clear();
	char line[256];
	if (size < 1)
	{
		return false;
	}
	memcpy(line, line1, size);

	line[size] = '\0';
	char tmp[256];
        sprintf(tmp, "%s\n", line);

	if (doc.ParseInsitu(line).HasParseError())
	{
		return false;
	}
	if (!doc.HasMember("relay_state") || !doc.HasMember("pairing_mode")
		|| !doc.HasMember("backlight") || !doc.HasMember("browser_ok") || !doc.HasMember("forget_sensor"))
	{
		return false;
	}
	if (!doc["relay_state"].IsArray() || !doc["pairing_mode"].IsBool() || !doc["forget_sensor"].IsBool() 
		|| !doc["backlight"].IsArray() || !doc["browser_ok"].IsBool()
		|| doc["relay_state"].Size() != RELAY_OUT_CNT || doc["backlight"].Size() != 4 /*RGBN */
		)
	{
		return false;
	}
	for (uint s = 0; s < doc["relay_state"].Size(); s++)
	{
		if ((uint8_t)doc["relay_state"][s].GetInt() <= 1)
			relays.push_back((uint8_t)doc["relay_state"][s].GetInt());
		else
		{
			return false;
		}
	}
	pairing = doc["pairing_mode"].GetBool();
	//wiring_check = doc["wiring_check"].GetBool();
	update_paired_list = doc["forget_sensor"].GetBool();
	rgbm.red = (uint8_t)doc["backlight"][0].GetInt();
	rgbm.green = (uint8_t)doc["backlight"][1].GetInt();
	rgbm.blue = (uint8_t)doc["backlight"][2].GetInt();
	rgbm.mode = (uint8_t)doc["backlight"][3].GetInt();
	//shutdownFromDynamic = doc["shut_down"].GetBool();
	if (rgbm.mode >= LED_NO_MODE)
		return false;
	//brighness_mode = (uint8_t)doc["brightness_mode"].GetInt();
	browser_ok = doc["browser_ok"].GetBool();
	return true;
}
bool setHW(char* HW, int len)
{
	char set_call[256];
	FILE* fp;
	char line[256];
	char execute[] = "/usr/share/apache2/default-site/htdocs/engine/setHW %s\0";
	sprintf(set_call, execute, HW);
	fp = popen(set_call, "r");
	if (fp == nullptr)
	{
		syslog(LOG_ERR, "%s", strerror(errno));
		return false;
	}
	while (fgets(line, 256, fp) != NULL)
	{
	}
	pclose(fp);
	return line[0] == '0';
}

// writes data of tof and lum to file for web. 
void saveBrTofTofile(uint16_t RangeMilliMeter, uint32_t Luminosity)
{
	std::ofstream	tof_file;
	char			echo_call[100]; 
	char			data_string[] = "{\"tof\": %d,\"brightness\": %d}";

	Luminosity += 20;
	if (Luminosity > 100)
		Luminosity = 100;

	if (RangeMilliMeter > TOF_IRQ_MIN_RANGE && RangeMilliMeter <= TOF_IRQ_MAX_RANGE)
		RangeMilliMeter = 1; // someone_is_close 
	else
		RangeMilliMeter = 0;


	sprintf(echo_call, data_string, RangeMilliMeter, Luminosity);
	tof_file.open(TOF_BR_FILE);
	if (tof_file.good())
	{
		tof_file << echo_call;
		tof_file.close();
	}
	else
	{
		syslog(LOG_INFO, "Error: saveBrTofTofile\n");
	}
}