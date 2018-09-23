#include "Config.h"
#include <fstream>
#include "XPLMDataAccess.h"
#include "CUESDK.h"

bool Config::Load() {
	//Clear current data
	conditions.clear();
	keys.clear();

	// Load js file
	std::ifstream t(DirPath + "illuminate.conf");
	t >> js;
	t.close();

	// Set background color
	BackgroundColor[0] = js["background"]["color"]["r"].get<int>();
	BackgroundColor[1] = js["background"]["color"]["g"].get<int>();
	BackgroundColor[2] = js["background"]["color"]["b"].get<int>();

	// Populate conditions 
	for each (auto condition in js["conditions"])
	{
		Condition c = Condition();
		c.dataRefString = condition["dataRef"].get<std::string>();
		c.dataRefName = condition["name"].get<std::string>();
		c.value = condition["value"].get<double>();

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

		c.dataRef = XPLMFindDataRef(c.dataRefString.c_str());
		if (c.dataRef == NULL) continue;
		c.dataType = XPLMGetDataRefTypes(c.dataRef);

		if (c.dataType == xplmType_FloatArray || c.dataType == xplmType_IntArray) {
			c.index = condition["index"].get<int>();
		}

		conditions.push_back(c);
	}

	 //Populate keys 
	for each (auto key in js["keys"])
	{
		Key k = Key();
		k.name = key["name"].get<std::string>();

		for each (auto condition in key["conditions"])
		{
			k.conditions.push_back(condition.get<std::string>());
		}

		char tmp = key["key"].get<std::string>().c_str()[0];
		k.LedId = CorsairGetLedIdForKeyName(tmp);

		k.Color[0] = key["color"]["r"].get<int>();
		k.Color[1] = key["color"]["g"].get<int>();
		k.Color[2] = key["color"]["b"].get<int>();

		keys.push_back(k);
	}
	return true;
}