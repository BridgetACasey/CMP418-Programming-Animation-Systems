//@BridgetACasey - 1802644

#include "Character2DFramed.h"

#include "utilities/JsonUtils.h"
#include "utilities/load_json.h"

namespace AnimationLib
{
	Character2DFramed::Character2DFramed()
	{
		frameIndex = 0;
		elapsedAnimTime = 0.0f;
		playingAnimation = true;
		customSpeed = 1.0f;
	}

	Character2DFramed::~Character2DFramed()
	{
	}

	void Character2DFramed::LoadData(const char* textureFilePath, const char* skeletonFilePath)
	{
		char* fileData = LoadJSON(skeletonFilePath);

		rapidjson::Document texDocument;

		texDocument.Parse(fileData);

		playRate = 1.0f / UtilsJSON::ValidateFloat(texDocument, "frameRate", 1.0f);

		textureAtlas = new TextureAtlas(textureFilePath);
	}

	void Character2DFramed::SetFrameOrder(std::string& frame)
	{
		frameOrder.push_back(frame);
	}

	void Character2DFramed::PlayAnimation(gef::Sprite& sprite, TextureAtlas& textureAtlas, float frameTime)
	{
		if (playingAnimation)
		{
			if(!frameOrder.empty())
			frameName = frameOrder[frameIndex];

			float customPlayRate = playRate / customSpeed;

			if (elapsedAnimTime > ((float)frameIndex * customPlayRate) && elapsedAnimTime < (customPlayRate * (float)textureAtlas.GetTotalSubTextures()))
			{
				frameIndex++;
			}

			if(frameIndex >= textureAtlas.GetTotalSubTextures() - 1)
			{
				elapsedAnimTime = 0.0f;
				frameIndex = 0;
			}

			elapsedAnimTime += frameTime;
		}
	}

	void Character2DFramed::UpdateTextureTransforms(gef::Sprite& sprite, float screenX, float screenY, AnimationLib::TextureAtlas& textureAtlas, const std::string& name)
	{
		float width = textureAtlas.GetSubTextures().at(name).width;
		float height = textureAtlas.GetSubTextures().at(name).height;
		float frameWidth = textureAtlas.GetSubTextures().at(name).frameWidth;
		float frameHeight = textureAtlas.GetSubTextures().at(name).frameHeight;
		float frameX = textureAtlas.GetSubTextures().at(name).frameX;
		float frameY = textureAtlas.GetSubTextures().at(name).frameY;

		float spritePosX = (textureAtlas.GetWidth() * 0.5f) - ((frameWidth * 0.5f) + frameX);
		float spritePosY = (textureAtlas.GetHeight() * 0.5f) - ((frameHeight * 0.5f) + frameY);

		float x = textureAtlas.GetSubTextures().at(name).x;
		float y = textureAtlas.GetSubTextures().at(name).y;

		float u = x / textureAtlas.GetWidth();
		float v = y / textureAtlas.GetHeight();

		sprite.set_width(width);
		sprite.set_height(height);
		sprite.set_uv_width(width / textureAtlas.GetWidth());
		sprite.set_uv_height(height / textureAtlas.GetHeight());
		sprite.set_uv_position(gef::Vector2(u, v));
		//sprite.set_position(gef::Vector4(spritePosX, spritePosY, sprite.position().z()));
	}
}