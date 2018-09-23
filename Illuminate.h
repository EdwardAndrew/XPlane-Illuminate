#pragma once
#include "Config.h"
#include "CUESDK.h"
#include <map>

class Illuminate
{
public:
	bool Start();
	void Stop();
	void Disable();
	bool Enable();

	void ReloadConfig();

	float FLCB(float, float, int, void*);
private:
	Config config;
	std::map<CorsairLedId, CorsairLedColor> previousColors;
	void setBackgroundColor();
	CorsairLedPositions* ledPositions;
	std::vector<CorsairLedColor> bgKeys;
};

