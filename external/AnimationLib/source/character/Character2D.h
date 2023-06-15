//@BridgetACasey - 1802644

#pragma once

#include "CharacterBase.h"

#include "anim2d/TextureAtlas.h"

namespace AnimationLib
{
	class Character2D : public CharacterBase
	{
	public:
		Character2D(){};
		~Character2D(){};

		//Removed for now as having LoadData here was causing linker errors
		//virtual void LoadData() = 0;

		TextureAtlas* GetTextureAtlas() { return textureAtlas; }

		float GetElapsedAnimTime() const { return elapsedAnimTime; }

	protected:
		TextureAtlas* textureAtlas;
		float elapsedAnimTime;
		float playRate;
	};
}