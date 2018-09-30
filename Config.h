#pragma once
#include <string>
#include <vector>
#include "Condition.h"
#include "Key.h"
#include "Mapping.h"
#include"Color.h"
#include "json.hpp"

struct CorsairLedColor;
struct CorsairLedPositions;

class Config {

public:
	bool Load();
	string DirPath = "Resources/plugins/Illuminate/";
	string ConfigFileName = "illuminate.conf";
	std::vector<Condition> conditions;
	std::vector<Key> keys;
	std::map<std::string, Color> colors;
	int BackgroundColor[3];
};