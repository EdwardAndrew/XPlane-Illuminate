#pragma once
#include <string>
#include <vector>

typedef enum CorsairLedId;

struct Key
{
public:
	std::string name;
	std::vector<std::string> conditions;
	int Color[3];
	CorsairLedId LedId;
};

