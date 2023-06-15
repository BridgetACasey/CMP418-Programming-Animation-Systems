//@BridgetACasey - 1802644

#include "SubTexture.h"

namespace AnimationLib
{
	SubTexture::SubTexture()
	{
	}

	void SubTexture::BuildTransform()
	{
		float x = width * 0.5f - (frameWidth * 0.5f + frameX);
		float y =  height * 0.5f - (frameHeight * 0.5f + frameY);

		transform.SetIdentity();
		translation.SetIdentity();
		scale.SetIdentity();

		translation.SetTranslation(gef::Vector2(x, y));
		scale.Scale(gef::Vector2(width, height));

		transform = scale * translation;
	}
}