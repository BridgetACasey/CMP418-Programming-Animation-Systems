//@BridgetACasey - 1802644

#include "Character2DSkeletal.h"
#include "anim2d/Animation.h"
#include "utilities/JsonUtils.h"
#include "utilities/load_json.h"
#include "maths/AnimMaths.h"
#include "maths/math_utils.h"

namespace AnimationLib
{
	Character2DSkeletal::Character2DSkeletal()
	{
		playingAnimation = true;
		elapsedAnimTime = 0.0f;
		playRate = 1.0f;
		textureAtlas = nullptr;
		armature = nullptr;
	}

	Character2DSkeletal::~Character2DSkeletal()
	{
	}

	void Character2DSkeletal::LoadData(const char* textureFilePath, const char* skeletonFilePath)
	{
		char* fileData = LoadJSON(skeletonFilePath);

		rapidjson::Document texDocument;

		texDocument.Parse(fileData);

		playRate = 1.0f / UtilsJSON::ValidateFloat(texDocument, "frameRate", 1.0f);

		textureAtlas = new TextureAtlas(textureFilePath);

		armature = new Armature();

		//Load armature data, which loads all subsequent data like bones, skin, etc.
		LoadArmatureData(texDocument);
	}

	void Character2DSkeletal::PlayAnimation(int animIndex, float frameTime, float speedModifier)
	{
		Animation animation = animations[animIndex];

		if (playingAnimation)
		{
			//Elapsed time between zero and animation total length
			elapsedAnimTime = fmod(elapsedAnimTime + frameTime * speedModifier, animation.duration);

			//For every rotation and translation frame, find the corresponding bone and recalculate its transforms accordingly

			for (auto& rot : animation.rotationFrames)
			{
				auto& bone = bones.at(rot.first);
				int rotCount = rot.second.size();

				for (int rotIndex = 0; rotIndex < rotCount; ++rotIndex)
				{
					int nextRot = (rotIndex >= rotCount - 1) ? 0 : rotIndex + 1;

					RotateFrame& startFrame = rot.second.at(rotIndex);
					RotateFrame& endFrame = rot.second.at(nextRot);

					if (elapsedAnimTime >= startFrame.startTime && elapsedAnimTime < endFrame.startTime)
					{
						float angle = bone->rotation;

						startFrame.elapsedTime = (elapsedAnimTime - startFrame.startTime) / (endFrame.startTime - startFrame.startTime);
						angle += AnimMaths::RotationLerp(startFrame.rotate, endFrame.rotate, startFrame.elapsedTime);

						bone->localTransform.Rotate(gef::DegToRad(angle));

						//For every bone we are rotating, check if it has translation data and apply it. We do the same with translation frames.
						if (animation.translationFrames.find(bone->name) == animation.translationFrames.end())
						{
							bone->localTransform.SetTranslation({ bone->x, bone->y });
						}
					}
				}
			}

			for (auto& trans : animation.translationFrames)
			{
				auto& bone = bones.at(trans.first);
				int transCount = trans.second.size();

				for (int transIndex = 0; transIndex < transCount; ++transIndex)
				{
					int nextTrans = (transIndex >= transCount - 1) ? 0 : transIndex + 1;

					TranslateFrame& startFrame = trans.second.at(transIndex);
					TranslateFrame& endFrame = trans.second.at(nextTrans);

					if (elapsedAnimTime >= startFrame.startTime && elapsedAnimTime < endFrame.startTime)
					{
						startFrame.elapsedTime = (elapsedAnimTime - startFrame.startTime) / (endFrame.startTime - startFrame.startTime);

						float x = bone->x;
						float y = bone->y;

						x += AnimMaths::LinearLerp(startFrame.x, endFrame.x, startFrame.elapsedTime);
						y += AnimMaths::LinearLerp(startFrame.y, endFrame.y, startFrame.elapsedTime);

						if (animation.rotationFrames.find(bone->name) == animation.rotationFrames.end())
						{
							bone->localTransform.Rotate(gef::DegToRad(bone->rotation));
						}

						bone->localTransform.SetTranslation(gef::Vector2(x, y));
					}
				}
			}

			//Update world transforms for the whole skeleton
			UpdateBoneWorldTransforms();
		}
	}

	void Character2DSkeletal::UpdateTextureTransforms(gef::Sprite& sprite, TextureAtlas& textureAtlas, const std::string& name)
	{
		float width = textureAtlas.GetSubTextures()[name].width;
		float height = textureAtlas.GetSubTextures()[name].height;

		float x = textureAtlas.GetSubTextures()[name].x;
		float y = textureAtlas.GetSubTextures()[name].y;

		float u = x / textureAtlas.GetWidth();
		float v = y / textureAtlas.GetHeight();

		sprite.set_width(width);
		sprite.set_height(height);
		sprite.set_uv_width(width / textureAtlas.GetWidth());
		sprite.set_uv_height(height / textureAtlas.GetHeight());
		sprite.set_uv_position(gef::Vector2(u, v));
	}

	void Character2DSkeletal::LoadArmatureData(const rapidjson::Value& document)
	{
		const rapidjson::Value& armatureArray = UtilsJSON::ValidateContainer(document, "armature");

		for (int i = 0; i < static_cast<int>(armatureArray.Size()); i++)
		{
			armature->type = UtilsJSON::ValidateString(armatureArray[i], "type");
			armature->frameRate = UtilsJSON::ValidateFloat(armatureArray[i], "frameRate");
			armature->name = UtilsJSON::ValidateString(armatureArray[i], "name");

			const rapidjson::Value& aabbContainer = UtilsJSON::ValidateContainer(armatureArray[i], "aabb");

			armature->aabb.x = UtilsJSON::ValidateFloat(aabbContainer, "x");
			armature->aabb.y = UtilsJSON::ValidateFloat(aabbContainer, "y");
			armature->aabb.width = UtilsJSON::ValidateFloat(aabbContainer, "width");
			armature->aabb.height = UtilsJSON::ValidateFloat(aabbContainer, "height");

			LoadBoneData(armatureArray[i]);
			LoadSkinData(armatureArray[i]);
			LoadSlotData(armatureArray[i]);
			LoadAnimationData(armatureArray[i]);
		}
	}

	void Character2DSkeletal::LoadBoneData(const rapidjson::Value& armatureArray)
	{
		const rapidjson::Value& boneArray = UtilsJSON::ValidateContainer(armatureArray, "bone");

		for (int i = 0; i < static_cast<int>(boneArray.Size()); i++)
		{
			Bone* bone = new Bone();

			bone->name = UtilsJSON::ValidateString(boneArray[i], "name");
			bone->length = UtilsJSON::ValidateFloat(boneArray[i], "length");
			bone->parentName = UtilsJSON::ValidateString(boneArray[i], "parent", "root");

			const rapidjson::Value& transform = UtilsJSON::ValidateContainer(boneArray[i], "transform");

			bone->x = UtilsJSON::ValidateFloat(transform, "x");
			bone->y = UtilsJSON::ValidateFloat(transform, "y");
			bone->rotation = UtilsJSON::ValidateFloat(transform, "skX");

			bone->name = "parts/" + bone->name;
			bone->parentName = "parts/" + bone->parentName;

			bone->BuildLocalTransform();

			bones.insert(std::pair<std::string, Bone*>(bone->name, bone));
		}

		//Assigning parents to each bone
		for (auto& bone : bones)
		{
			if (bone.second->parent == nullptr)
				bone.second->parent = bones[bone.second->parentName];
		}

		BuildBoneWorldTransforms();
	}

	void Character2DSkeletal::LoadSkinData(const rapidjson::Value& armatureArray)
	{
		const rapidjson::Value& skinArray = UtilsJSON::ValidateContainer(armatureArray, "skin");

		for (int j = 0; j < static_cast<int>(skinArray.Size()); j++)
		{
			const rapidjson::Value& slotArray = UtilsJSON::ValidateContainer(skinArray[j], "slot");

			Skin skin;

			for (int i = 0; i < static_cast<int>(slotArray.Size()); i++)
			{
				SkinSlot skinSlot;

				skinSlot.name = UtilsJSON::ValidateString(slotArray[i], "name");

				const rapidjson::Value& display = UtilsJSON::ValidateContainer(slotArray[i], "display", 0);

				skinSlot.partName = UtilsJSON::ValidateString(display, "name");

				const rapidjson::Value& transform = UtilsJSON::ValidateContainer(display, "transform");

				skinSlot.x = UtilsJSON::ValidateFloat(transform, "x");
				skinSlot.y = UtilsJSON::ValidateFloat(transform, "y");
				skinSlot.rotation = UtilsJSON::ValidateFloat(transform, "skX");

				skinSlot.BuildTransform();

				skin.skinSlots.insert(std::pair<std::string, SkinSlot>(skinSlot.partName, skinSlot));
			}

			skins.push_back(skin);
		}
	}

	void Character2DSkeletal::LoadSlotData(const rapidjson::Value& armatureArray)
	{
		const rapidjson::Value& slotArray = UtilsJSON::ValidateContainer(armatureArray, "slot");

		for (int i = 0; i < static_cast<int>(slotArray.Size()); i++)
		{
			ArmatureSlot armatureSlot;

			const std::string& name = UtilsJSON::ValidateString(slotArray[i], "name");
			const std::string& parentName = UtilsJSON::ValidateString(slotArray[i], "parent");

			armatureSlot.name = "parts/" + name;
			armatureSlot.parent = "parts/" + parentName;

			slots.push_back(armatureSlot);
		}
	}

	void Character2DSkeletal::LoadAnimationData(const rapidjson::Value& armatureArray)
	{
		const rapidjson::Value& animationArray = UtilsJSON::ValidateContainer(armatureArray, "animation");

		for (int i = 0; i < static_cast<int>(animationArray.Size()); i++)
		{
			Animation animation;

			animation.duration = UtilsJSON::ValidateFloat(animationArray[i], "duration");
			animation.playTimes = UtilsJSON::ValidateFloat(animationArray[i], "playTimes");
			animation.name = UtilsJSON::ValidateString(animationArray[i], "name");

			const rapidjson::Value& boneKeyArray = UtilsJSON::ValidateContainer(animationArray[i], "bone");

			for (int j = 0; j < static_cast<int>(boneKeyArray.Size()); j++)
			{
				std::string boneName = UtilsJSON::ValidateString(boneKeyArray[j], "name");

				boneName = "parts/" + boneName;

				const rapidjson::Value& translateArray = UtilsJSON::ValidateContainer(boneKeyArray[j], "translateFrame");
				const rapidjson::Value& rotateArray = UtilsJSON::ValidateContainer(boneKeyArray[j], "rotateFrame");

				if (translateArray.IsArray())
				{
					std::vector<TranslateFrame> translateFrames;

					const int translateArraySize = static_cast<int>(translateArray.Size());

					if (translateArraySize > 1)
					{
						float startTime = 0.0f;

						for (int t = 0; t < translateArraySize; t++)
						{
							TranslateFrame translation;

							translation.duration = UtilsJSON::ValidateFloat(translateArray[t], "duration");
							translation.tweenEasing = UtilsJSON::ValidateFloat(translateArray[t], "tweenEasing");
							translation.x = UtilsJSON::ValidateFloat(translateArray[t], "x");
							translation.y = UtilsJSON::ValidateFloat(translateArray[t], "y");

							translation.startTime = startTime;
							startTime += translation.duration;

							translateFrames.push_back(translation);
						}

						animation.translationFrames.emplace(boneName, translateFrames);
					}
				}

				if (rotateArray.IsArray())
				{
					std::vector<RotateFrame> rotateFrames;

					const int rotateArraySize = static_cast<int>(rotateArray.Size());

					if (rotateArraySize > 1)
					{
						float startTime = 0.0f;

						for (int r = 0; r < rotateArraySize; r++)
						{
							RotateFrame rotation;

							rotation.duration = UtilsJSON::ValidateFloat(rotateArray[r], "duration");
							rotation.tweenEasing = UtilsJSON::ValidateFloat(rotateArray[r], "tweenEasing");
							rotation.rotate = UtilsJSON::ValidateFloat(rotateArray[r], "rotate");

							rotation.startTime = startTime;
							startTime += rotation.duration;

							rotateFrames.push_back(rotation);
						}

						animation.rotationFrames.emplace(boneName, rotateFrames);
					}
				}
			}

			animations.push_back(animation);
		}
	}

	void Character2DSkeletal::UpdateBoneWorldTransforms()
	{
		for (auto& bone : bones)
		{
			bone.second->worldTransform = bone.second->localTransform * bone.second->parent->worldTransform;
		}
	}

	void Character2DSkeletal::BuildBoneWorldTransforms()	//Re-build the world transforms from scratch
	{
		for (auto& bone : bones)
		{
			bone.second->BuildWorldTransform();
		}
	}
}
