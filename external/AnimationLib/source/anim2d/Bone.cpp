//@BridgetACasey - 1802644

#include "Bone.h"

#include "maths/math_utils.h"

namespace AnimationLib
{
	Bone::Bone()
	= default;

	gef::Matrix33 Bone::BuildLocalTransform(float rot, float posX, float posY)
	{
		gef::Matrix33 rotationMatrix;
		gef::Matrix33 translationMatrix;

		rotationMatrix.SetIdentity();
		translationMatrix.SetIdentity();
		localTransform.SetIdentity();

		rotationMatrix.Rotate(gef::DegToRad(rotation + rot));
		translationMatrix.SetTranslation(gef::Vector2(x + posX, y + posY));

		localTransform = rotationMatrix * translationMatrix;

		return localTransform;
	}

	gef::Matrix33 Bone::BuildWorldTransform()	//World transforms calculated by recursively calling parent's build transform functions
	{
		if (parent != nullptr)
		{
			if (name != "parts/root")
			{
				worldTransform = localTransform * parent->BuildWorldTransform();
			}
		}

		return worldTransform;
	}
}