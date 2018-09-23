#include "Illuminate.h"
#include "XPLMDataAccess.h"

bool Illuminate::Start() {
	// Request control of Corsair devices
	CorsairPerformProtocolHandshake();
	if (CorsairGetLastError())
	{
		return false;
	}
	CorsairRequestControl(CAM_ExclusiveLightingControl);

	ReloadConfig();

	return true;
}

bool Illuminate::Enable() {
	return Start();
}

void Illuminate::Stop() {
	CorsairReleaseControl(CAM_ExclusiveLightingControl);
}

void Illuminate::Disable() {
	Stop();
}

float Illuminate::FLCB(float inElapsedSinceLastCall, float inElapsedTimeSinceLastFlightLoop, int inCounter, void* inRefcon) {
	// Evaluate all datarefs.
	std::map<std::string, bool> conditionResults;
	for each (Condition c in config.conditions)
	{
		if (c.dataRef == NULL) continue;
		switch (c.dataType) {
		case xplmType_Int:
			conditionResults[c.dataRefName] = c.Evaluate((XPLMGetDatai(c.dataRef) != 0));
			break;
		case xplmType_FloatArray:
			float dataRefValue;
			XPLMGetDatavf(c.dataRef, &dataRefValue, 0, 1);
			conditionResults[c.dataRefName] = c.Evaluate(dataRefValue);
			break;
		}
	}

	// Evaluate all keys.
	vector<CorsairLedColor> corsairLEDColors;
	for each (auto key in config.keys)
	{
		bool result = true;
		for (int i = 0; i < key.conditions.size(); i++)
		{
			string condition = key.conditions[i];
			if (conditionResults.find(condition) != conditionResults.end())
			{
				if (!conditionResults[condition]) {
					result = false;
					break;
				}
			}
		}
		// We only want to set the key color if all conditions are true.
		if (result) {
			corsairLEDColors.push_back(
				CorsairLedColor{
				key.LedId,
				key.Color[0],
				key.Color[1],
				key.Color[2]
				});
		}
	}

	// We only want to update colors that have actually changed.
	vector<CorsairLedColor> changedColors;
	for each (CorsairLedColor color in corsairLEDColors)
	{
		if (previousColors.find(color.ledId) != previousColors.end()) {
			CorsairLedColor previousColor = previousColors[color.ledId];
			int r = previousColor.r, g = previousColor.g, b = previousColor.b;
			r -= color.r;
			g -= previousColor.g;
			b -= previousColor.b;
			if ((r + g + b) != 0) {
				changedColors.push_back(color);
			}
		}
		else {
			changedColors.push_back(color);
		}
	}
	// Update LEDs with new colors.
	CorsairSetLedsColors(changedColors.size(), &changedColors[0]);

	// Set previous colors to new colors.
	previousColors.clear();
	for each(CorsairLedColor key in corsairLEDColors) {
		previousColors[key.ledId] = key;
	}

	return 0.1f;
}

void Illuminate::ReloadConfig() {
	config = Config();
	config.Load();
	setBackgroundColor();
}

void Illuminate::setBackgroundColor() {
	ledPositions = CorsairGetLedPositions();
	for (int ledIndex = 0; ledIndex < ledPositions->numberOfLed; ledIndex++)
	{
		bgKeys.push_back(CorsairLedColor{ ledPositions->pLedPosition[ledIndex].ledId , config.BackgroundColor[0], config.BackgroundColor[1], config.BackgroundColor[2] });
	}
	CorsairSetLedsColors(bgKeys.size(), &bgKeys[0]);
}