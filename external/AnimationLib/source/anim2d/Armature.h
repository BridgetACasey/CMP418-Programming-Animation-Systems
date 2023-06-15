//@BridgetACasey - 1802644

#pragma once

#include <string>

namespace AnimationLib
{
	struct AABB
	{
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
	};

	struct ArmatureSlot
	{
		int displayIndex = 0;
		std::string name, parent;
	};

	struct Armature
	{
		Armature() = default;
		~Armature() = default;

		std::string name, type;
		float frameRate = 0.0f;
		AABB aabb;
	};
}