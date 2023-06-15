//@BridgetACasey - 1802644

#include "LinearBlendNode.h"

#include "BlendTree.h"

namespace AnimationLib
{
	LinearBlendNode::LinearBlendNode() : BlendNodeBase(nullptr)
	{
	}

	LinearBlendNode::LinearBlendNode(BlendTree* tree) : BlendNodeBase(tree)
	{
		outputPose = tree->GetPose();
		inputs.resize(2);
		variables.resize(1);
	}

	bool LinearBlendNode::ProcessNode(float frameTime)
	{
		bool outputIsValid = true;

		BlendNodeFloat* alpha = static_cast<BlendNodeFloat*>(blendTree->GetTreeVariables().at(variables[0]));

		if (inputs.empty())
			outputIsValid = false;
		else
			//Perform simple linear blend between two poses
			outputPose.Linear2PoseBlend(inputs.at(0)->GetOutputPose(), inputs.at(1)->GetOutputPose(), alpha->floatVariable);

		return outputIsValid;
	}
}
