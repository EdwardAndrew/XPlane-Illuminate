#include <stdio.h>
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPWidgets.h"
#include "XPWidgetDefs.h"
#include "XPWidgetUtils.h"
#include "XPStandardWidgets.h"
#include "Illuminate.h"


int g_MenuItem;
XPWidgetID IlluminateWidget = NULL;
XPWidgetID IlluminateWindow = NULL;
float IlluminateFLCB(float elapsedMe, float elapsedSim, int counter, void * refcon); 
void IlluminateMenuHandler(void *, void*);
void CreateIlluminateWidget(int x1, int y1, int w, int h);
int IlluminateWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2);
Illuminate illuminate;

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

	
	// Create menu	
	PluginSubMenuItem = XPLMAppendMenuItem(XPLMFindPluginsMenu(), "Illuminate", NULL, 1);
	PluginMenu = XPLMCreateMenu("Illuminate", XPLMFindPluginsMenu(), PluginSubMenuItem, IlluminateMenuHandler, NULL);
	XPLMAppendMenuItem(PluginMenu, "Reload Config", (void *)+1, 1);
	XPLMAppendMenuItem(PluginMenu, "Settings", (void *)+2, 2);
	g_MenuItem = 0;

	illuminate = Illuminate();
	if (!illuminate.Start()) {
		return 0;
	}

	XPLMRegisterFlightLoopCallback(IlluminateFLCB, 0.0, NULL);
	XPLMSetFlightLoopCallbackInterval(IlluminateFLCB, 0.01, 1, NULL);
	return 1;}

PLUGIN_API void	XPluginStop(void)
{
	XPLMUnregisterFlightLoopCallback(IlluminateFLCB, NULL);
	illuminate.Stop();

	if (g_MenuItem == 1)
	{
		XPDestroyWidget(IlluminateWidget, 1);
		g_MenuItem = 0;
	}
}

PLUGIN_API void XPluginDisable(void)
{
	XPLMUnregisterFlightLoopCallback(IlluminateFLCB, NULL);
	illuminate.Disable();
	if (g_MenuItem == 1)
	{
		XPDestroyWidget(IlluminateWidget, 1);
		g_MenuItem = 0;
	}
}

PLUGIN_API int XPluginEnable(void)
{
	illuminate.Enable();
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
	return illuminate.FLCB(
		inElapsedSinceLastCall,
		inElapsedTimeSinceLastFlightLoop,
		inCounter,
		inRefcon
	);
}

void IlluminateMenuHandler(void * inMenuRef, void * inItemRef)
{
	switch ((int)inItemRef)
	{
		case 1:
			illuminate.ReloadConfig();
		break;

		default: 
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
		break;
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

	//for (int i = 0; i < config.conditions.size(); i++)
	//{
	//	Condition c = config.conditions[i];
	//	XPWidgetID btn = XPCreateWidget(x+10, y-(40*(i+1)), x + 400, y - (60 *(i +1)), 1, std::to_string(c.dataType).c_str(), 0, IlluminateWidget, xpWidgetClass_TextField);
	//	XPSetWidgetProperty(btn, xpTextEntryField, 0);
	//}

	XPAddWidgetCallback(IlluminateWidget, IlluminateWidgetHandler);
}

int IlluminateWidgetHandler(XPWidgetMessage inMessage, XPWidgetID inWidget, intptr_t inParam1, intptr_t inParam2)
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