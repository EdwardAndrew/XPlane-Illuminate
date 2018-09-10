#include <stdio.h>
#include <string.h>
#include <vector>
#include "CUESDK.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"

int bgRed = 255, bgGreen = 178, bgBlue = 8;

XPLMDataRef GearDeployedDataRef = NULL;
XPLMDataRef GearHandleDown = NULL;

float GearLightingFLCB(float elapsedMe, float elapsedSim, int counter, void * refcon); 

PLUGIN_API int XPluginStart(
						char *		outName,
						char *		outSig,
						char *		outDesc)
{
		int			mySubMenuItem;

	/* Provide our plugin's profile to the plugin system. */
	strcpy(outName, "Illuminate");
	strcpy(outSig, "edwardandrew.xplane.illuminate");
	strcpy(outDesc, "Interactive keyboard illumination.");
	
	CorsairPerformProtocolHandshake();

	if (CorsairGetLastError())
	{
		return 0;
	}

	CorsairRequestControl(CAM_ExclusiveLightingControl);
			
	GearDeployedDataRef = XPLMFindDataRef("sim/flightmodel2/gear/deploy_ratio");
	GearHandleDown = XPLMFindDataRef("sim/cockpit/switches/gear_handle_status");

	XPLMRegisterFlightLoopCallback(GearLightingFLCB, 0.0, NULL);
	XPLMSetFlightLoopCallbackInterval(GearLightingFLCB, 0.01, 1, NULL);
	
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
	XPLMUnregisterFlightLoopCallback(GearLightingFLCB, NULL);

	CorsairReleaseControl(CAM_ExclusiveLightingControl);
}

PLUGIN_API void XPluginDisable(void)
{
	XPLMUnregisterFlightLoopCallback(GearLightingFLCB, NULL);

	CorsairReleaseControl(CAM_ExclusiveLightingControl);
}

PLUGIN_API int XPluginEnable(void)
{
	CorsairRequestControl(CAM_ExclusiveLightingControl);

	XPLMRegisterFlightLoopCallback(GearLightingFLCB, 0.0, NULL);
	XPLMSetFlightLoopCallbackInterval(GearLightingFLCB, 0.01, 1, NULL);

	return 1;
}

PLUGIN_API void XPluginReceiveMessage(
					XPLMPluginID	inFromWho,
					int				inMessage,
					void *			inParam)
{
}

float GearLightingFLCB(
	float                inElapsedSinceLastCall,
	float                inElapsedTimeSinceLastFlightLoop,
	int                  inCounter,
	void *               inRefcon)
{

	float gearDeployRatio[3];
	XPLMGetDatavf(GearDeployedDataRef,&gearDeployRatio[0],0,3);

	CorsairLedColor GDeployed{ CLK_G,0,255,0 };
	CorsairLedColor GNotDeployed{ CLK_G, 255, 0, 0 };
	CorsairLedColor GOff{ CLK_G, bgRed,bgGreen, bgBlue };

	CorsairLedColor VDeployed{ CLK_V,0,255,0 };
	CorsairLedColor VNotDeployed{ CLK_V, 255, 0, 0 };
	CorsairLedColor VOff{ CLK_V, bgRed,bgGreen, bgBlue };

	CorsairLedColor BDeployed{ CLK_B,0,255,0 };
	CorsairLedColor BNotDeployed{ CLK_B, 255, 0, 0 };
	CorsairLedColor BOff{ CLK_B, bgRed,bgGreen, bgBlue };

	bool gearHandleDown = XPLMGetDatai(GearHandleDown);

	if (gearHandleDown)
	{
		if (gearDeployRatio[0] < 1.0f)
		{
			CorsairSetLedsColors(1, &GNotDeployed);
		}
		else
		{
			CorsairSetLedsColors(1, &GDeployed);
		}

		if (gearDeployRatio[1] < 1.0f)
		{
			CorsairSetLedsColors(1, &VNotDeployed);
		}
		else
		{
			CorsairSetLedsColors(1, &VDeployed);
		}

		if (gearDeployRatio[2] < 1.0f)
		{
			CorsairSetLedsColors(1, &BNotDeployed);
		}
		else
		{
			CorsairSetLedsColors(1, &BDeployed);
		}

	}
	else
	{
		if (gearDeployRatio[0] > 0.0f)
		{
			CorsairSetLedsColors(1, &GNotDeployed);
		}
		else
		{
			CorsairSetLedsColors(1, &GOff);
		}

		if (gearDeployRatio[1] > 0.0f)
		{
			CorsairSetLedsColors(1, &VNotDeployed);
		}
		else
		{
			CorsairSetLedsColors(1, &VOff);
		}

		if (gearDeployRatio[2] > 0.0f)
		{
			CorsairSetLedsColors(1, &BNotDeployed);
		}
		else
		{
			CorsairSetLedsColors(1, &BOff);
		}
	}
	return 0.1f;
}

