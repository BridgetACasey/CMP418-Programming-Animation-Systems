//@BridgetACasey - 1802644

#include "TextureAtlas.h"

#include "utilities/load_json.h"
#include "utilities/JsonUtils.h"

namespace AnimationLib
{
	TextureAtlas::TextureAtlas(const char* filePath, int* orderOfFrames)
	{
		char* fileData = LoadJSON(filePath);
		
		rapidjson::Document texDocument;
		
		texDocument.Parse(fileData);
		
		name = UtilsJSON::ValidateString(texDocument, "name");
		path = UtilsJSON::ValidateString(texDocument, "imagePath");
		width = UtilsJSON::ValidateFloat(texDocument, "width");
		height = UtilsJSON::ValidateFloat(texDocument, "height");
		
		const rapidjson::Value& subTextureArray = UtilsJSON::ValidateContainer(texDocument, "SubTexture");
		
		for (int i = 0; i < (int)subTextureArray.Size(); i++)
		{
			SubTexture subTexture = LoadSubTextureFromJSON(subTextureArray[i]);
			subTextures.emplace(subTexture.name, subTexture);
			//subTextures.insert(std::pair<std::string, SubTexture>(name, subTexture));
		}
		
		if (orderOfFrames != nullptr)
			ReOrderFrames(orderOfFrames);
	}

	//Used to re-structure a vector of frames based on a specified order. Currently not in use due to switching to using a map.
	void TextureAtlas::ReOrderFrames(int* frameOrder)
	{
		//std::vector<SubTexture> tempSubTextures = subTextures;
		//
		//for (int i = 0; i < tempSubTextures.size(); i++)
		//{
		//	tempSubTextures[i] = subTextures[frameOrder[i]];
		//}
		//
		//subTextures = tempSubTextures;
	}

	SubTexture TextureAtlas::LoadSubTextureFromJSON(const rapidjson::Value& subTex)
	{
		SubTexture subTexture;	//Load and verify subtexture data, then build initial texture transforms

		subTexture.name = UtilsJSON::ValidateString(subTex, "name");
		subTexture.width = UtilsJSON::ValidateFloat(subTex, "width");
		subTexture.height = UtilsJSON::ValidateFloat(subTex, "height");
		subTexture.frameWidth = UtilsJSON::ValidateFloat(subTex, "frameWidth", subTexture.width);
		subTexture.frameHeight = UtilsJSON::ValidateFloat(subTex, "frameHeight", subTexture.height);
		subTexture.x = UtilsJSON::ValidateFloat(subTex, "x");
		subTexture.y = UtilsJSON::ValidateFloat(subTex, "y");
		subTexture.frameX = UtilsJSON::ValidateFloat(subTex, "frameX");
		subTexture.frameY = UtilsJSON::ValidateFloat(subTex, "frameY");

		subTexture.BuildTransform();

		return subTexture;
	}
}