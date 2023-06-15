//@BridgetACasey - 1802644

#pragma once

#include <string>

#include "maths/matrix33.h"

namespace AnimationLib
{
	class Bone
	{
	public:
		Bone();
		~Bone() = default;

		//Can specify position and rotation of new transform or just leave blank to reset
		gef::Matrix33 BuildLocalTransform(float rot = 0.0f, float posX = 0.0f, float posY = 0.0f);
		gef::Matrix33 BuildWorldTransform();

		Bone* parent = nullptr;

		std::string name, parentName;

		float length = 0.0f;
		float rotation = 0.0f;
		float x = 0.0f;
		float y = 0.0f;

		gef::Matrix33 localTransform = gef::Matrix33::kIdentity;
		gef::Matrix33 worldTransform = gef::Matrix33::kIdentity;
	};
}
