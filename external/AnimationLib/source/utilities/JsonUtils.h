//@BridgetACasey - 1802644

#pragma once

#include <string>

#include "rapidjson/document.h"

namespace AnimationLib
{
	class UtilsJSON
	{
	public:
		//Reads the current level of the JSON document for the specified label, and returns a default value if not found.
		static std::string ValidateString(const rapidjson::Value& document, const char* label, const std::string& defaultValue = std::string("STRING NOT FOUND"))
		{
			if (document.HasMember(label))
				return document[label].GetString();
			else
				return defaultValue;
		}

		//Reads the current level of the JSON document for the specified label, and returns a default value if not found.
		static float ValidateFloat(const rapidjson::Value& document, const char* label, const float defaultValue = 0.0f)
		{
			if (document.HasMember(label))
				return document[label].GetFloat();
			else
				return defaultValue;
		}

		//Reads the current level of the JSON document for the specified label, and returns a default value if not found.
		static double ValidateDouble(const rapidjson::Value& document, const char* label, const double defaultValue = 0.0)
		{
			if (document.HasMember(label))
				return document[label].GetDouble();
			else
				return defaultValue;
		}

		//Reads the current level of the JSON document for the specified label, and returns a default value if not found.
		static int ValidateInt(const rapidjson::Value& document, const char* label, const int defaultValue = 0)
		{
			if (document.HasMember(label))
				return document[label].GetInt();
			else
				return defaultValue;
		}

		//Reads the current level of the JSON document for the specified label, and returns a default value if not found.
		static bool ValidateBool(const rapidjson::Value& document, const char* label, const bool defaultValue = false)
		{
			if (document.HasMember(label))
				return document[label].GetBool();
			else
				return defaultValue;
		}

		//Reads the current level of the JSON document for a container with the specified label and optional index, and returns a default value if not found.
		static const rapidjson::Value& ValidateContainer(const rapidjson::Value& document, const char* label, int index = -1)
		{
			if (document.HasMember(label))
			{
				if (document[label].IsArray() && index >= 0)
					return document[label][index];
				else
					return document[label];
			}
			else
				return document;
		}
	};
}