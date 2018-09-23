#include "Condition.h"

bool Condition::Evaluate(double dataRefValue)
{
	switch (conditionType) {
	case ConditionType::exactly:
		return value == dataRefValue;
	case ConditionType::less_than:
		return dataRefValue < value;

	case ConditionType::greater_than:
		return dataRefValue > value;
	}

	return false;
}