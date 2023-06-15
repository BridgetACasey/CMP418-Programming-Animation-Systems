//@BridgetACasey - 1802644

#pragma once

#include <vector>

#include "Character2D.h"
#include "graphics/sprite.h"
#include "rapidjson/document.h"

namespace AnimationLib
{
	//2D character that's animated by swapping subtexture frames
	class Character2DFramed : public Character2D
	{
	public:
		Character2DFramed();
		~Character2DFramed();

		virtual void LoadData(const char* textureFilePath, const char* skeletonFilePath);
		virtual void SetFrameOrder(std::string& frame);

		virtual void PlayAnimation(gef::Sprite& sprite, TextureAtlas& textureAtlas, float frameTime);

		virtual void UpdateTextureTransforms(gef::Sprite& sprite, float screenX, float screenY, AnimationLib::TextureAtlas& textureAtlas, const std::string& name);

		std::string GetFrameName() const { return frameName; }

		void SetCustomSpeed(float speed) { customSpeed = speed; }

	protected:
		float customSpeed;
		int frameIndex;
		std::string frameName;
		std::vector<std::string> frameOrder;
	};
}