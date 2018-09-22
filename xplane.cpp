#include <stdio.h>
#include <string.h>
#include <vector>
#include "CUESDK.h"
#include <fstream>
#include "XPLMDataAccess.h"
#include "XPLMUtilities.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPWidgetDefs.h"
#include "XPWidgetUtils.h"
#include "XPStandardWidgets.h"
#include "json.hpp"
#include "Condition.h"

using json = nlohmann::json;

const std::string IlluminateDir = "Resources/plugins/XPlane-Illuminate/";
json config;
std::vector<Condition> conditions;

int bgRed = 255, bgGreen = 178, bgBlue = 8;

XPLMDataRef GearDeployedDataRef = NULL;
XPLMDataRef GearHandleDown = NULL;

float IlluminateFLCB(float elapsedMe, float elapsedSim, int counter, void * refcon); 

int g_MenuItem;
XPWidgetID IlluminateWidget = NULL;
XPWidgetID IlluminateWindow = NULL;
void IlluminateMenuHandler(void *, void*);
void CreateIlluminateWidget(int x1, int y1, int w, int h);
int IlluminateHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2);
bool evaluateCondition(double value, ConditionType cType, double dataRefValue);

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
	XPLMMenuID PluginMenu;
	int		   PluginSubMenuItem;

	/* Provide our plugin's profile to the plugin system. */
	strcpy(outName, "Illuminate");
	strcpy(outSig, "edwardandrew.xplane.illuminate");
	strcpy(outDesc, "Interactive keyboard illumination.");

	// Load config file
	std::ifstream t(IlluminateDir + "illuminate.conf");
	t >> config;
	t.close();

	// Populate conditions array
	for each (auto condition in config["conditions"])
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

	// Create menu	
	PluginSubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Illuminate", NULL, 1);
	PluginMenu = XPLMCreateMenu("Illuminate", XPLMFindPluginsMenu(), PluginSubMenuItem, IlluminateMenuHandler, NULL);
	XPLMAppendMenuItem(PluginMenu, "Settings", (void *)+1, 1);
	g_MenuItem = 0;

	CorsairPerformProtocolHandshake();

	if (CorsairGetLastError())
	{
		return 0;
	}

	CorsairRequestControl(CAM_ExclusiveLightingControl);
			
	GearDeployedDataRef = XPLMFindDataRef("sim/flightmodel2/gear/deploy_ratio");
	GearHandleDown = XPLMFindDataRef("sim/cockpit/switches/gear_handle_status");

	XPLMRegisterFlightLoopCallback(IlluminateFLCB, 0.0, NULL);
	XPLMSetFlightLoopCallbackInterval(IlluminateFLCB, 0.01, 1, NULL);
	
	/* Only return that we initialized correctly if we found the data ref. */
	if (GearDeployedDataRef == NULL) return 0;
	if (GearHandleDown == NULL) return 0;

	std::vector<CorsairLedColor> backgroundColors;

	CorsairLedPositions *allKeys = CorsairGetLedPositions();
	for (int ledIndex = 0; ledIndex < allKeys->numberOfLed; ledIndex++)
	{
		backgroundColors.push_back(CorsairLedColor{ allKeys->pLedPosition[ledIndex].ledId , bgRed, bgGreen, bgBlue});
	}
	CorsairSetLedsColors(backgroundColors.size(), &backgroundColors[0]);

	return 1;}

PLUGIN_API void	XPluginStop(void)
{
	XPLMUnregisterFlightLoopCallback(IlluminateFLCB, NULL);
	CorsairReleaseControl(CAM_ExclusiveLightingControl);

	if (g_MenuItem == 1)
	{
		XPDestroyWidget(IlluminateWidget, 1);
		g_MenuItem = 0;
	}
}

PLUGIN_API void XPluginDisable(void)
{
	XPLMUnregisterFlightLoopCallback(IlluminateFLCB, NULL);
	CorsairReleaseControl(CAM_ExclusiveLightingControl);

	if (g_MenuItem == 1)
	{
		XPDestroyWidget(IlluminateWidget, 1);
		g_MenuItem = 0;
	}
}

PLUGIN_API int XPluginEnable(void)
{
	CorsairRequestControl(CAM_ExclusiveLightingControl);

	XPLMRegisterFlightLoopCallback(IlluminateFLCB, 0.0, NULL);
	XPLMSetFlightLoopCallbackInterval(IlluminateFLCB, 0.01, 1, NULL);

	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					int				inMessage,
					void *			inParam)
{
}

float IlluminateFLCB(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon)
{

   //Evaluate all datarefs

	std::map<std::string, bool> conditionResults;

	for each (Condition c in conditions)
	{
		if (c.dataRef == NULL) continue;
		switch (c.dataType) {
			case xplmType_Int:
				conditionResults[c.dataRefName] = evaluateCondition(c.value, c.conditionType, (XPLMGetDatai(c.dataRef) != 0));
				break;
			case xplmType_FloatArray:
				float dataRefValue;
				XPLMGetDatavf(c.dataRef, &dataRefValue, 0, 1);
				conditionResults[c.dataRefName] = evaluateCondition(c.value, c.conditionType, dataRefValue);
				break;
		}
	}

	for each (auto key in config["keys"])
	{
		bool result = true;

		for(int i = 0; i < key["conditions"].size(); i++)
		{
			auto condition = key["conditions"][i];
			string conditionString = condition.get<std::string>();
			if (conditionResults.find(conditionString) != conditionResults.end())
			{
				if (!conditionResults[conditionString]) {
					result = false;
					break;
				}
			}
		}

		if (result) {
			char k = key["key"].get<std::string>().c_str()[0];

			CorsairLedColor keyColor{
				CorsairGetLedIdForKeyName(k),
				key["color"]["r"].get<int>(),
				key["color"]["g"].get<int>(),
				key["color"]["b"].get<int>()
			};
			CorsairSetLedsColors(1, &keyColor);
		}
	}
	return 0.1f;
}


void IlluminateMenuHandler(void * inMenuRef, void * inItemRef)
{
	switch ((int)inItemRef)
	{
		case 1: 
			if (g_MenuItem == 0)
			{
				CreateIlluminateWidget(500, 500, 500, 500);
				XPShowWidget(IlluminateWidget);
				g_MenuItem = 1;
			}
			else 
			{
				if (!XPIsWidgetVisible(IlluminateWidget)) {
					XPShowWidget(IlluminateWidget);
				}
			}
	}
}

void CreateIlluminateWidget(int x, int y, int w, int h)
{
	int Index;
	int x2 = x + w;
	int y2 = y - h;

	IlluminateWidget = XPCreateWidget(x, y, x2, y2,
		1,
		"Illuminate",
		1,
		NULL,
		xpWidgetClass_MainWindow);

	XPSetWidgetProperty(IlluminateWidget, xpProperty_MainWindowHasCloseBoxes, 1);

	for (int i = 0; i < conditions.size(); i++)
	{
		Condition c = conditions[i];
		XPWidgetID btn = XPCreateWidget(x+10, y-(40*(i+1)), x + 400, y - (60 *(i +1)), 1, std::to_string(c.dataType).c_str(), 0, IlluminateWidget, xpWidgetClass_TextField);
		XPSetWidgetProperty(btn, xpTextEntryField, 0);
	}

	XPAddWidgetCallback(IlluminateWidget, IlluminateHandler);
}

int IlluminateHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2)
{
	if (inMessage == xpMessage_CloseButtonPushed)
	{
		if (g_MenuItem == 1)
		{
			XPHideWidget(IlluminateWidget);
		}
		return 1;
	}
	return 0;
}

bool evaluateCondition(double value, ConditionType cType, double dataRefValue)
{
	switch (cType) {
	case ConditionType::exactly:
			return value == dataRefValue;
	case ConditionType::less_than:
		return dataRefValue < value;

	case ConditionType::greater_than:
		return dataRefValue > value;
	}

	return false;
}