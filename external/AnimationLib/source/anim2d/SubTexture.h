//@BridgetACasey - 1802644

#pragma once

#include <string>

#include "maths/matrix33.h"

namespace AnimationLib
{
	class SubTexture
	{
	public:
		SubTexture();
		~SubTexture() = default;

		void BuildTransform();

		std::string name;

		float width = 0.0f;
		float height = 0.0f;
		float frameWidth = 0.0f;
		float frameHeight = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float frameX = 0.0f;
		float frameY = 0.0f;

		gef::Matrix33 transform;
		gef::Matrix33 rotation;
		gef::Matrix33 translation;
		gef::Matrix33 scale;
	};
}
