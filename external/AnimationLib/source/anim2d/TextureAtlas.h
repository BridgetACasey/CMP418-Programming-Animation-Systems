//@BridgetACasey - 1802644

#pragma once

#include <map>
#include <string>

#include "SubTexture.h"
#include "rapidjson/document.h"

namespace AnimationLib
{
	class TextureAtlas
	{
	public:
		TextureAtlas(const char* filePath, int* orderOfFrames = nullptr);
		~TextureAtlas() = default;

		float GetWidth() const { return width; }
		float GetHeight() const { return height; }

		std::map<std::string, SubTexture>& GetSubTextures() { return subTextures; }

		int GetTotalSubTextures() const { return static_cast<int>(subTextures.size()); }

	private:
		std::string name, path;
		float width = 0.0f;
		float height = 0.0f;

		std::map<std::string, SubTexture> subTextures;

		void ReOrderFrames(int* frameOrder);

		//The texture atlas is responsible for loading its own textures from the JSON file
		SubTexture LoadSubTextureFromJSON(const rapidjson::Value& subTex);
	};
}