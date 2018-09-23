#pragma once
#include <string>
#include <vector>
#include "Condition.h"
#include "Key.h"
#include "json.hpp"

struct CorsairLedColor;
struct CorsairLedPositions;

class Config {

public:
	bool Load();
	string DirPath = "Resources/plugins/XPlane-Illuminate/";
	string ConfigFileName = "illuminate.conf";
	std::vector<Condition> conditions;
	std::vector<Key> keys;
	int BackgroundColor[3];
};