//@BridgetACasey - 1802644

#pragma once

#include <map>
#include <string>
#include <vector>

#include "maths/math_utils.h"
#include "maths/matrix33.h"

namespace AnimationLib
{
	class SkinSlot
	{
	public:
		SkinSlot();
		~SkinSlot() = default;

		void BuildTransform();

		std::string name, partName;

		float x = 0.0f;
		float y = 0.0f;
		float rotation = 0.0f;

		gef::Matrix33 transform;
	};

	struct Skin
	{
		//Map of skin slots, since a character may have more than one skin file, even if only one is currently used in the coursework
		std::map<std::string, SkinSlot> skinSlots;
	};
}
