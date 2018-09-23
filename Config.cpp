#include "Config.h"
#include <fstream>
#include "XPLMDataAccess.h"
#include "CUESDK.h"

bool Config::Load() {
	//Clear current data
	conditions.clear();
	keys.clear();

	// Load js file
	nlohmann::json js;

	std::ifstream t(DirPath + "illuminate.conf");
	t >> js;
	t.close();

	BackgroundColor[0] = 0;
	BackgroundColor[1] = 0;
	BackgroundColor[2] = 0;

	// Set background color
	if (js.find("background") != js.end())
	{
		if (!js["background"]["color"].is_null())
		{
			auto bgColor = js["background"]["color"];
			if (bgColor["r"].is_number()) BackgroundColor[0] = bgColor["r"].get<int>();
			if (bgColor["g"].is_number()) BackgroundColor[1] = bgColor["g"].get<int>();
			if (bgColor["b"].is_number()) BackgroundColor[2] = bgColor["b"].get<int>();
		}
	}

	if (js["conditions"].is_array())
	{
		// Populate conditions 
		for each (auto condition in js["conditions"])
		{
			Condition c = Condition();
			if(condition["dataRef"].is_string()) c.dataRefString = condition["dataRef"].get<std::string>();
			if(condition["name"].is_string()) c.dataRefName = condition["name"].get<std::string>();
			if(condition["value"].is_number()) c.value = condition["value"].get<double>();

			if (condition["match"].is_string())
			{
				string match = condition["match"].get<std::string>();
				if (match == std::string("less_than"))
				{
					c.conditionType = ConditionType::less_than;
				}
				else if (match == std::string("greater_than"))
				{
					c.conditionType = ConditionType::greater_than;
				}
				else {
					c.conditionType = ConditionType::exactly;
				}
			}

			c.dataRef = XPLMFindDataRef(c.dataRefString.c_str());
			if (c.dataRef == NULL) continue;
			c.dataType = XPLMGetDataRefTypes(c.dataRef);

			if (c.dataType == xplmType_FloatArray || c.dataType == xplmType_IntArray) {
				if (condition["index"].is_number()) {
					c.index = condition["index"].get<int>();
				}
				else {
					c.index = 0;
				}
			}
			conditions.push_back(c);
		}
	}

	if (js["keys"].is_array())
	{
		//Populate keys 
		for each (auto key in js["keys"])
		{
			Key k = Key();
			if(key["name"].is_string()) k.name = key["name"].get<std::string>();

			if (key["conditions"].is_array())
			{
				for each (auto condition in key["conditions"])
				{
					if(condition.is_string()) k.conditions.push_back(condition.get<std::string>());
				}
			}

			if (key["key"].is_string())
			{
				char tmp = key["key"].get<std::string>().c_str()[0];
				k.LedId = CorsairGetLedIdForKeyName(tmp);
			}

			if (key["color"].is_string())
			{
				k.Color[0] = BackgroundColor[0];
				k.Color[1] = BackgroundColor[1];
				k.Color[2] = BackgroundColor[2];
			}
			else
			{
				if(key["color"]["r"].is_number()) k.Color[0] = key["color"]["r"].get<int>();
				if(key["color"]["g"].is_number()) k.Color[1] = key["color"]["g"].get<int>();
				if(key["color"]["b"].is_number()) k.Color[2] = key["color"]["b"].get<int>();
			}
			keys.push_back(k);
		}

	}
	return true;
}