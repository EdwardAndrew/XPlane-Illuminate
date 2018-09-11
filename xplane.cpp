#include <stdio.h>
#include <string.h>
#include <vector>
#include "CUESDK.h"
#include "XPLMDataAccess.h"
#include "XPLMProcessing.h"
#include "XPLMMenus.h"
#include "XPLMDisplay.h"
#include "XPLMGraphics.h"
#include <string.h>
#include <gl/GL.h>

int bgRed = 255, bgGreen = 178, bgBlue = 8;

XPLMDataRef GearDeployedDataRef = NULL;
XPLMDataRef GearHandleDown = NULL;

float GearLightingFLCB(float elapsedMe, float elapsedSim, int counter, void * refcon); 

int g_menu_container_idx;
XPLMMenuID g_menu_id;
void menu_handler(void *, void*);

XPLMWindowID g_window = NULL;
void				draw_hello_world(XPLMWindowID in_window_id, void * in_refcon);
int					dummy_mouse_handler(XPLMWindowID in_window_id, int x, int y, int is_down, void * in_refcon) { return 0; }
XPLMCursorStatus	dummy_cursor_status_handler(XPLMWindowID in_window_id, int x, int y, void * in_refcon) { return xplm_CursorDefault; }
int					dummy_wheel_handler(XPLMWindowID in_window_id, int x, int y, int wheel, int clicks, void * in_refcon) { return 0; }
void				dummy_key_handler(XPLMWindowID in_window_id, char key, XPLMKeyFlags flags, char virtual_key, void * in_refcon, int losing_focus) { }
void draw_button(const char * text, float in_out_lbrt[4] /* you set the left and bottom, we'll set the right and top */);

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

	// Temporary Menu
	g_menu_container_idx = XPLMAppendMenuItem(XPLMFindPluginsMenu(),"Illuminate", 0, 0);
	g_menu_id = XPLMCreateMenu("Illuminate", XPLMFindPluginsMenu(), g_menu_container_idx, menu_handler, nullptr);
	XPLMAppendMenuItem(g_menu_id, "Settings", (void *)"Menu Item 1", 1);

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

	// Window Stuff
	XPLMCreateWindow_t params;
	params.structSize = sizeof(params);
	params.visible = 1;
	params.drawWindowFunc = draw_hello_world;
	// Note on "dummy" handlers:
	// Even if we don't want to handle these events, we have to register a "do-nothing" callback for them
	params.handleMouseClickFunc = dummy_mouse_handler;
	params.handleRightClickFunc = dummy_mouse_handler;
	params.handleMouseWheelFunc = dummy_wheel_handler;
	params.handleKeyFunc = dummy_key_handler;
	params.handleCursorFunc = dummy_cursor_status_handler;
	params.refcon = NULL;
	params.layer = xplm_WindowLayerFloatingWindows;
	// Opt-in to styling our window like an X-Plane 11 native window
	// If you're on XPLM300, not XPLM301, swap this enum for the literal value 1.
	params.decorateAsFloatingWindow = xplm_WindowDecorationRoundRectangle;

	// Set the window's initial bounds
	// Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
	// We'll need to query for the global desktop bounds!
	int left, bottom, right, top;
	XPLMGetScreenBoundsGlobal(&left, &top, &right, &bottom);
	params.left = left + 50;
	params.bottom = bottom + 150;
	params.right = params.left + 200;
	params.top = params.bottom + 200;

	g_window = XPLMCreateWindowEx(&params);

	// Position the window as a "free" floating window, which the user can drag around
	XPLMSetWindowPositioningMode(g_window, xplm_WindowPositionFree, -1);
	// Limit resizing our window: maintain a minimum width/height of 100 boxels and a max width/height of 300 boxels
	XPLMSetWindowResizingLimits(g_window, 200, 200, 300, 300);
	XPLMSetWindowTitle(g_window, "Sample Window");

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

void menu_handler(void * in_menu_ref, void * in_item_ref)
{
}

void	draw_hello_world(XPLMWindowID in_window_id, void * in_refcon)
{
	// Mandatory: We *must* set the OpenGL state before drawing
	// (we can't make any assumptions about it)
	XPLMSetGraphicsState(
		0 /* no fog */,
		0 /* 0 texture units */,
		0 /* no lighting */,
		0 /* no alpha testing */,
		1 /* do alpha blend */,
		1 /* do depth testing */,
		0 /* no depth writing */
	);

	int l, t, r, b;
	XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);

	float col_white[] = { 1.0, 1.0, 1.0 }; // red, green, blue

	XPLMDrawString(col_white, l + 10, t - 20, "Hello world!", NULL, xplmFont_Proportional);
	float lbrt[] = { l, b, r, t };
	draw_button("Toggle", lbrt);
}

void draw_button(const char * text, float in_out_lbrt[4] /* you set the left and bottom, we'll set the right and top */)
{
	// We draw our rudimentary button boxes based on the height of the button text
	int char_height;
	XPLMGetFontDimensions(xplmFont_Proportional, NULL, &char_height, NULL);

	in_out_lbrt[2] = in_out_lbrt[0] + XPLMMeasureString(xplmFont_Proportional, text, strlen(text)); // *just* wide enough to fit the button text
	in_out_lbrt[3] = in_out_lbrt[1] + (1.25f * char_height); // a bit taller than the button text

	// Draw the box around our rudimentary button
	float green[] = { 0.0, 1.0, 0.0, 1.0 };
	glColor4fv(green);
	glBegin(GL_LINE_LOOP);
	{
		glVertex2i(in_out_lbrt[0], in_out_lbrt[3]);
		glVertex2i(in_out_lbrt[2], in_out_lbrt[3]);
		glVertex2i(in_out_lbrt[2], in_out_lbrt[1]);
		glVertex2i(in_out_lbrt[0], in_out_lbrt[1]);
	}
	glEnd();

	// Draw the button text (pop in/pop out)
	float col_white[] = { 1.0, 1.0, 1.0 };
	XPLMDrawString(col_white, in_out_lbrt[0], in_out_lbrt[1] + 4, (char *)text, NULL, xplmFont_Proportional);
}