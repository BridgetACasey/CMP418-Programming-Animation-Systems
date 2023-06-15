//@BridgetACasey - 1802644

#pragma once

#include <vector>

#include "Character2D.h"
#include "graphics/sprite.h"
#include "rapidjson/document.h"
#include "anim2d/Skin.h"
#include "anim2d/Armature.h"
#include "anim2d/Bone.h"

namespace AnimationLib
{
	class Animation;

	//2D character that is animated based on a skeletal armature
	class Character2DSkeletal : public Character2D
	{
	public:
		Character2DSkeletal();
		~Character2DSkeletal();

		virtual void LoadData(const char* textureFilePath, const char* skeletonFilePath);

		virtual void PlayAnimation(int animIndex, float frameTime, float speedModifier);

		virtual void UpdateTextureTransforms(gef::Sprite& sprite, TextureAtlas& textureAtlas, const std::string& name);

		SkinSlot GetSkinSlotAt(const int skinIndex, const std::string& slotIndex) const { return skins[skinIndex].skinSlots.at(slotIndex); }
		std::map<std::string, Bone*>& GetBones() { return bones; }
		std::vector<ArmatureSlot>& GetSlots() { return slots; }

	protected:
		void LoadArmatureData(const rapidjson::Value& document);
		void LoadBoneData(const rapidjson::Value& armatureArray);
		void LoadSkinData(const rapidjson::Value& armatureArray);
		void LoadSlotData(const rapidjson::Value& armatureArray);
		void LoadAnimationData(const rapidjson::Value& armatureArray);

		void UpdateBoneWorldTransforms();
		void BuildBoneWorldTransforms();

		std::map<std::string, Bone*> bones;
		std::vector<Skin> skins;
		std::vector<ArmatureSlot> slots;
		std::vector<Animation> animations;
		Armature* armature;
	};
}