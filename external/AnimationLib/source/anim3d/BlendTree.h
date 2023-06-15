//@BridgetACasey - 1802644

#pragma once

#include "OutputNode.h"
#include "animation/animation.h"
#include "animation/skeleton.h"

namespace AnimationLib
{
	class OutputNode;

	struct BlendNodeVariable { BlendNodeVariable() = default; };

	struct BlendNodeBool : BlendNodeVariable
	{
		BlendNodeBool();
		BlendNodeBool(bool variable) { boolVariable = variable; }

		bool boolVariable;
	};

	struct BlendNodeInt : BlendNodeVariable
	{
		BlendNodeInt();
		BlendNodeInt(int variable) { intVariable = variable; }

		int intVariable;
	};

	struct BlendNodeFloat : BlendNodeVariable
	{
		BlendNodeFloat();
		BlendNodeFloat(float variable) { floatVariable = variable; }

		float floatVariable;
	};

	struct BlendNodeString : BlendNodeVariable
	{
		BlendNodeString();
		BlendNodeString(std::string variable) { stringVariable = std::move(variable); }

		std::string stringVariable;
	};

	struct BlendData3D : BlendNodeVariable
	{
		BlendData3D();

		BlendData3D(gef::Animation& firstAnim, gef::Animation& secondAnim)
		{
			firstSpeed = 1.0f / firstAnim.duration();
			firstMinSpeed = 1.0f;
			firstMaxSpeed = firstAnim.duration() / secondAnim.duration();

			secondSpeed = 1.0f / secondAnim.duration();
			secondMinSpeed = secondAnim.duration() / firstAnim.duration();
			secondMaxSpeed = 1.0f;
		}

		float firstSpeed;
		float firstMinSpeed;
		float firstMaxSpeed;

		float secondSpeed;
		float secondMinSpeed;
		float secondMaxSpeed;

	};

	class BlendTree
	{
	public:
		BlendTree(const gef::SkeletonPose& bindPose);

		void Init(const gef::SkeletonPose& skeletonPose);

		bool UpdateTree(float frameTime);

		void InsertVariable(std::string name, BlendNodeVariable* variable);

		OutputNode& GetOutputNode() { return outputNode; }
		gef::SkeletonPose& GetPose() { return pose; }
		std::map<std::string, BlendNodeVariable*>& GetTreeVariables() { return treeVariables; }

	private:
		gef::SkeletonPose pose;

		OutputNode outputNode;

		std::map<std::string, BlendNodeVariable*> treeVariables;
	};
}