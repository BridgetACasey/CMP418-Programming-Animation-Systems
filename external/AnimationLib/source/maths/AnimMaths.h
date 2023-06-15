//@BridgetACasey - 1802644

#pragma once

namespace AnimationLib
{
	//Helper maths class with a couple of functions for interpolation

	class AnimMaths
	{
	public:
		static float LinearLerp(float x0, float x1, float t)
		{
			return (1.0f - t) * x0 + t * x1;
		}

		static float RotationLerp(float alpha, float beta, float interpolation)
		{
			float delta = beta - alpha;

			if (delta > 180.0f)
			{
				delta -= 360.0f;
			}
			else if (delta < -180.0f)
			{
				delta += 360.0f;
			}

			return  alpha + interpolation * delta;
		}
	};
}