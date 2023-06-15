//@BridgetACasey - 1802644

#pragma once

#include <map>

namespace AnimationLib
{
	struct TranslateFrame
	{
		float startTime = 0.0f;
		float elapsedTime = 0.0f;
		float duration = 0.0f;
		float tweenEasing = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
	};

	struct RotateFrame
	{
		float startTime = 0.0f;
		float elapsedTime = 0.0f;
		float duration = 0.0f;
		float tweenEasing = 0.0f;
		float rotate = 0.0f;
	};

	//Used for 2D skeletal animation
	struct Animation
	{
		Animation() = default;
		~Animation() = default;

		std::string name;

		float elapsedTime = 0.0f;
		float duration = 0.0f;
		float playTimes = 0.0f;

		std::map<std::string, std::vector<TranslateFrame>> translationFrames;
		std::map<std::string, std::vector<RotateFrame>> rotationFrames;
	};
}