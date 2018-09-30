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

	std::ifstream t(DirPath + "illuminate.json");
	if (t.is_open())
	{
		t >> js;
	}
	t.close();

	BackgroundColor[0] = 0;
	BackgroundColor[1] = 0;
	BackgroundColor[2] = 0;

	// Load Colors
	if (js["colors"].is_array())
	{
		for each (auto color in js["colors"])
		{
			if (color["name"].is_string()) {
				Color c = Color();
				c.name = color["name"].get<std::string>();
				if (color["r"].is_number()) c.r = color["r"].get<int>();
				if (color["g"].is_number()) c.g = color["g"].get<int>();
				if (color["b"].is_number()) c.b = color["b"].get<int>();
				colors[c.name] = c;

				if (c.name.compare("background") == 0) {
					BackgroundColor[0] = c.r;
					BackgroundColor[1] = c.g;
					BackgroundColor[2] = c.b;
				}
			}
		}
	}

	if (js["conditions"].is_array())
	{
		// Load conditions 
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
		// Load keys 
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
			else if (key["key"].is_number())
			{
				k.LedId = (CorsairLedId)key["key"].get<int>();
			}

			if (key["color"].is_string())
			{
				if (colors.find(key["color"].get<std::string>()) != colors.end())
				{
					Color c = colors[key["color"].get<std::string>()];
					k.Color[0] = c.r;
					k.Color[1] = c.g;
					k.Color[2] = c.b;
				}
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