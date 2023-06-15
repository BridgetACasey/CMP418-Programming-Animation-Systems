//@BridgetACasey - 1802644

#pragma once

namespace AnimationLib
{
	//Base class that all animated characters inherit from, both 2D and 3D
	class CharacterBase
	{
	public:
		CharacterBase(){};
		~CharacterBase(){};

		//virtual void LoadData();

		void Play() { playingAnimation = true; }
		void Stop() { playingAnimation = false; }

	protected:
		bool playingAnimation = false;
	};
}