//@BridgetACasey - 1802644

#include "BlendTree.h"

namespace AnimationLib
{
	BlendTree::BlendTree(const gef::SkeletonPose& bindPose)
	{
		Init(bindPose);
	}

	void BlendTree::Init(const gef::SkeletonPose& skeletonPose)
	{
		pose = skeletonPose;
		outputNode = OutputNode(this);
	}

	bool BlendTree::UpdateTree(float frameTime)
	{
		outputNode.UpdateNode(frameTime);

		pose = outputNode.GetOutputPose();

		return true;
	}

	void BlendTree::InsertVariable(std::string name, BlendNodeVariable* variable)
	{
		treeVariables.insert(std::pair<std::string, BlendNodeVariable*>(name, variable));
	}
}
