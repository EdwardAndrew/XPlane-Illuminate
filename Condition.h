#pragma once
#include <string>
using namespace std;

typedef void* XPLMDataRef;
typedef int XPLMDataTypeID;

enum ConditionType{
	exactly,
	less_than,
	greater_than
};

class Condition
{
public:
	bool Evaluate(double);
	string dataRefString;
	string dataRefName;
	XPLMDataRef dataRef = NULL;
	XPLMDataTypeID dataType;
	double value;
	ConditionType conditionType;
	int index;
};

