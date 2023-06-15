//@BridgetACasey - 1802644

#include "OutputNode.h"

#include "BlendTree.h"

namespace AnimationLib
{
	OutputNode::OutputNode() : BlendNodeBase(nullptr)
	{
		inputs.resize(1);
	}

	OutputNode::OutputNode(BlendTree* tree) : BlendNodeBase(tree)
	{
		inputs.resize(1);
		outputPose = tree->GetPose();
	}

	bool OutputNode::ProcessNode(float frameTime)
	{
		bool outputIsValid = true;

		if (inputs.empty())
			outputIsValid = false;
		else
			outputPose = inputs[0]->GetOutputPose();

		return outputIsValid;
	}
}
