//@BridgetACasey - 1802644

#include "Skin.h"

#include "maths/math_utils.h"

namespace AnimationLib
{
	SkinSlot::SkinSlot()
	{
	}

	void SkinSlot::BuildTransform()
	{
		gef::Matrix33 rotationMatrix;
		gef::Matrix33 translationMatrix;

		rotationMatrix.SetIdentity();
		translationMatrix.SetIdentity();
		transform.SetIdentity();

		rotationMatrix.Rotate(gef::DegToRad(rotation));
		translationMatrix.SetTranslation(gef::Vector2(x, y));

		transform = rotationMatrix * translationMatrix;
	}
}
