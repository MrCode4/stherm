// get_configtest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#define _POSIX_SOURCE
#include <sys/ioctl.h>			//Needed for SPI port
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <cstring>
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <fcntl.h>				//Needed for SPI port
#include <sys/stat.h>//mkfifo
#include <sys/types.h>
#include <unistd.h>			//Needed for SPI port
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#define RELAY_OUT_CNT 10
#define WIRING_IN_CNT 10
#define MAX_PAIED_CNT 16
typedef enum RF_Dev_types {
	Main_dev = 0x01,
	AQ_TH_PR,         //Aqs Temp Hum Pressure sensors
	NO_TYPE = 0xFF
} RF_Dev_types_t;
typedef struct AQ_TH_PR_thld {
	uint8_t Tvoc_high; //0.1 to 10+ mg/m^3
	uint8_t etoh_high; // up to 20ppm
	uint8_t iaq_high; //1 to 5
	int8_t temp_high;// up to +127C
	int8_t temp_low;// as low as -128C
	uint8_t humidity_high;// up to 100%
	uint8_t humidity_low;//as low as 0%
	uint16_t pressure_high; //up to 1200 hPa
	uint16_t c02_high;//400 to 5000ppm
} AQ_TH_PR_thld_t;
enum Led_effect_e
{
	LED_STABLE = 0,
	LED_FADE,
	LED_BLINK,
	LED_NO_MODE
};
#define AQS_THRSHLD_SIZE 11
typedef struct AQ_TH_PR_vals {
	int8_t temp;// up to +127C
	uint8_t humidity;// up to 100%
	uint16_t c02;//400 to 5000ppm
	uint8_t etoh; // up to 20ppm
	uint8_t Tvoc; //0.1 to 10+ mg/m^3
	uint8_t iaq; //1 to 5
	uint16_t pressure; //up to 1200 hPa
} AQ_TH_PR_vals_t;
#define AQS_DATA_SIZE 9
typedef struct Response_Time {
	uint8_t TT_if_ack;//in 15s increments
	uint8_t TT_if_nack;//in 100ms increments
	uint16_t TP_internal_sesn_poll;// 10 ms increments
} Response_Time_t;
typedef enum Child_status_t
{
	pending,
	pair
}Child_status;
typedef enum Sns_type {
	SNS_temperature = 1,
	SNS_humidity,
	SNS_co2,
	SNS_etoh,
	SNS_Tvoc,
	SNS_iaq,
	SNS_pressure,
	SNS_NO_TYPE
} Sns_type_t;
struct device_t
{
	uint32_t address;
	uint8_t type;
	bool paired;
};
struct config_time {
	uint8_t ext_sens_type;
	uint8_t TT_if_ack;//in 15s increments
	uint8_t TT_if_nack;//in 100ms increments
	uint16_t TP_internal_sesn_poll;// 10 ms increments
};
#define CONF_TIME_SIZE 5
struct config_thresholds {
	uint8_t sens_type;
	uint16_t min_alert_value;//in 15s increments
	uint16_t max_alert_value;//in 100ms increments
};
struct rgb_vals {
	uint8_t red;
	uint8_t green;
	uint8_t blue;
	uint8_t mode;
	bool operator ==(rgb_vals const& obj) {
		return (this->blue == obj.blue &&
			this->red == obj.red &&
			this->green == obj.green &&
			this->mode == obj.mode);
	}
};
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
				var = (tolower(c) - 87);
			ret |= ((uint32_t)var) << (7 - j) * 4;
			j++;

		}
	}
	return ret;
}
bool get_paired_list(std::vector<device_t>& list)
{
	FILE* fp;
	rapidjson::Document doc;
	list.clear();
	device_t tmp_dev;
	int status;
	char line[256];
	char getPaired_php[] = "php /usr/share/apache2/default-site/htdocs/engine/getPaired.php\0";
	fp = popen(getPaired_php, "r");
	if (fp == NULL)
		return false;
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
	for (int s = 0; s < doc["data"].Size(); s++)
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
		tmp_dev.type = doc["data"][s]["external_sensor_type"].GetInt();
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
	int status;
	config_thresholds tmp_thrld;
	char line[1024];
	char getTreshold_php[] = "php /usr/share/apache2/default-site/htdocs/engine/getTreshold.php\0";
	fp = popen(getTreshold_php, "r");
	if (fp == NULL)
		return false;
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
	for (int s = 0; s < doc["data"].Size(); s++)
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
		tmp_thrld.sens_type = doc["data"][s]["sensor_type"].GetInt();
		if (tmp_thrld.sens_type >= SNS_NO_TYPE)
			return false;
		tmp_thrld.max_alert_value = doc["data"][s]["max_alert_value"].GetInt();
		tmp_thrld.min_alert_value = doc["data"][s]["min_alert_value"].GetInt();
		tholds.push_back(tmp_thrld);
	}
	pclose(fp);
	return true;
}
bool get_Config_list(std::vector<config_time>& time_cnfg)
{
	FILE* fp;
	rapidjson::Document doc;
	int status;
	char line[1024];
	char getConfig_php[] = "php /usr/share/apache2/default-site/htdocs/engine/getConfig.php\0";
	config_time tmp_cfg;
	fp = popen(getConfig_php, "r");
	if (fp == NULL)
		return false;
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
	for (int s = 0; s < doc["data"].Size(); s++)
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
		tmp_cfg.ext_sens_type = doc["data"][s]["external_sensor_type"].GetInt();
		tmp_cfg.TT_if_ack = doc["data"][s]["poll_if_responded"].GetInt();
		tmp_cfg.TT_if_nack = doc["data"][s]["poll_if_not_responded"].GetInt();
		tmp_cfg.TP_internal_sesn_poll = doc["data"][s]["internal_poll_time"].GetInt();
		time_cnfg.push_back(tmp_cfg);
	}
	pclose(fp);
	return true;
}
bool get_dynamic(std::vector<uint8_t>& relays, bool& pairing, bool& wiring_check, rgb_vals& rgbm, uint8_t& brighness_mode, uint32_t& last_upd)
{
	FILE* fp;
	rapidjson::Document doc;
	int status;
	char line[256];
	char getDynamic_php[] = "php /usr/share/apache2/default-site/htdocs/engine/getDynamic.php\0";
	fp = popen(getDynamic_php, "r");
	if (fp == NULL)
		return false;
	if (fgets(line, 256, fp) == NULL)
		return false;
	if (doc.ParseInsitu(line).HasParseError())
	{
		return false;
	}
	if (!doc.HasMember("relay_state") || !doc.HasMember("pairing_mode") || !doc.HasMember("wiring_check")
		|| !doc.HasMember("backlight") || !doc.HasMember("brightness_mode") || !doc.HasMember("last_update"))
	{
		return false;
	}
	if (!doc["relay_state"].IsArray() || !doc["pairing_mode"].IsBool() || !doc["wiring_check"].IsBool()
		|| !doc["backlight"].IsArray() || !doc["brightness_mode"].IsInt() || !doc["last_update"].IsInt()
		|| doc["relay_state"].Size() != RELAY_OUT_CNT || doc["backlight"].Size() != 4//RGBN
		)
	{
		return false;
	}
	for (int s = 0; s < doc["relay_state"].Size(); s++)
	{
		if ((uint8_t)doc["relay_state"][s].GetInt() <= 1)
			relays.push_back((uint8_t)doc["relay_state"][s].GetInt());
		else
		{
			return false;
		}
	}
	pairing = (uint8_t)doc["pairing_mode"].GetBool();
	wiring_check = (uint8_t)doc["wiring_check"].GetBool();
	rgbm.red = (uint8_t)doc["backlight"][0].GetInt();
	rgbm.green = (uint8_t)doc["backlight"][1].GetInt();
	rgbm.blue = (uint8_t)doc["backlight"][2].GetInt();
	rgbm.mode = (uint8_t)doc["backlight"][3].GetInt();
	if (rgbm.mode >= LED_NO_MODE)
		return false;
	brighness_mode = (uint8_t)doc["brightness_mode"].GetInt();
	last_upd = (uint32_t)doc["last_update"].GetInt();
	pclose(fp);
	return true;
}
bool setWiring(std::vector<uint8_t>& wiring)
{
	if (wiring.size() != WIRING_IN_CNT)
		return false;
	char set_call[128];
	FILE* fp;
	int status;
	char line[256];
	char setWiring_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setWiring.php '{\"wiring_state\":[%d,%d,%d,%d,%d,%d,%d,%d,%d,%d]}'\0";//WIRING_IN_CNT
	sprintf(set_call, setWiring_php, wiring[0], wiring[1], wiring[2], wiring[3], wiring[4], wiring[5], wiring[6], wiring[7], wiring[8], wiring[9]);
	fp = popen(set_call, "r");
	if (fp == NULL)
		return false;
	fgets(line, 256, fp);
	printf("%s\n", line);
	pclose(fp);
	return line[0] == '0';
}
bool setAlert(uint32_t dev_id, uint8_t err_code, uint8_t level)
{
	char set_call[128];
	FILE* fp;
	int status;
	char line[256];
	char setAlert_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setAlert.php '{\"device_id\":\"%4x\",\"error_code\":%d,\"level\":%d}'\0";
	sprintf(set_call, setAlert_php, dev_id, err_code, level);
	fp = popen(set_call, "r");
	if (fp == NULL)
		return false;
	fgets(line, 256, fp);
	printf("%s\n", line);
	pclose(fp);
	return line[0] == '0';
}
bool addPendingSensor(device_t& dev_one)
{
	char set_call[128];
	FILE* fp;
	int status;
	char line[256];
	char addPendingSensor_php[] = "php /usr/share/apache2/default-site/htdocs/engine/addPendingSensor.php '{\"external_sensor_id\":\"%4x\",\"external_sensor_type\":%d}'";
	sprintf(set_call, addPendingSensor_php, dev_one.address, dev_one.type);
	fp = popen(set_call, "r");
	if (fp == NULL)
		return false;
	fgets(line, 128, fp);
	printf("%s\n", line);
	pclose(fp);
	return line[0] == '0';
}
bool setSensorData(device_t& dev, uint8_t* data, uint16_t size)
{
	char set_call[512];
	FILE* fp;
	int status;
	char line[256];
	char setSensorData_Aqs_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setSensorData.php '{\"external_sensor_type\":%d, \"external_sensor_id\":\"%4x\",\"data\":[\"temperature\":%d, \"humidity\":%d, \"co2\":%d, \"etoh\":%d, \"tvoc\":%d, \"aiq\":%d, \"pressure\":%d]'\0";
	char setSensorData_main_php[] = "php /usr/share/apache2/default-site/htdocs/engine/setSensorData.php '{\"external_sensor_type\":%d, \"external_sensor_id\":\"%4x\",\"data\":[\"temperature\":%d, \"humidity\":%d, \"co2\":%d, \"etoh\":%d, \"tvoc\":%d, \"aiq\":%d, \"pressure\":%d, \"tof\":%d, \"ambient\":%d]}'\0";
	switch (dev.type)
	{
	case Main_dev:
		sprintf(set_call, setSensorData_main_php,dev.type, dev.address, ((int16_t*)data)[0], ((uint8_t*)(data + 2))[0], ((uint16_t*)(data + 3))[0], ((uint16_t*)(data + 5))[0], ((uint16_t*)(data + 7))[0], ((uint8_t*)(data + 9))[0], ((uint16_t*)(data + 10))[0], ((uint16_t*)(data + 12))[0], ((uint32_t*)(data + 14))[0]);
		break;
	case AQ_TH_PR:
		sprintf(set_call, setSensorData_Aqs_php, dev.type, dev.address, ((int8_t*)data)[0], ((uint8_t*)(data + 1))[0], ((uint16_t*)(data + 2))[0], ((uint8_t*)(data + 4))[0], ((uint8_t*)(data + 5))[0], ((uint8_t*)(data + 6))[0], ((uint16_t*)(data + 7))[0]);
		break;
	default:
		break;
	}
	fp = popen(set_call, "r");
	if (fp == NULL)
		return false;
	fgets(line, 128, fp);
	printf("%s\n", line);
	pclose(fp);
	return line[0] == '0';
}
bool setBrightness(uint32_t Luminosity)
{
	char set_call[32];
	uint8_t brightness_lvl = 100;
	char execute[] = "brightness %d\0";
	sprintf(set_call, execute, brightness_lvl);
	system(set_call);
	return true;
}
int main()
{
	
	device_t dev;
	dev.address = 0xAABBCCEE;
	dev.type = 2;
	dev.paired = true;
	//setAlert(dev.address, 2, 1);
	//addPendingSensor(dev);
	std::vector<uint8_t> wiring;
	for (int i = 0; i < 10; i++)
		wiring.push_back(1);
	//setWiring(wiring);
	uint8_t data[] = {26,30,1,170,10,13,3,3,10};

	setSensorData(dev, data, 9);
	return 0;
}

