//@BridgetACasey - 1802644

#include "BilinearBlendNode.h"

#include "BlendTree.h"
#include "maths/AnimMaths.h"

namespace AnimationLib
{
	BilinearBlendNode::BilinearBlendNode() : BlendNodeBase(nullptr)
	{
	}

	BilinearBlendNode::BilinearBlendNode(BlendTree* tree) : BlendNodeBase(tree)
	{
		outputPose = tree->GetPose();
		inputs.resize(4);
		variables.resize(2);
	}

	bool BilinearBlendNode::ProcessNode(float frameTime)
	{
		bool outputIsValid = true;

		if (inputs.empty())
			outputIsValid = false;
		else
		{
			//Retrieving user input from the blend tree and using it to blend 4 animations along U and V axes

			BlendNodeFloat* u = static_cast<BlendNodeFloat*>(blendTree->GetTreeVariables().at(variables[0]));
			BlendNodeFloat* v = static_cast<BlendNodeFloat*>(blendTree->GetTreeVariables().at(variables[1]));

			if (u != nullptr && v != nullptr)
			{
				gef::SkeletonPose firstPose = blendTree->GetPose();
				firstPose.Linear2PoseBlend(inputs.at(0)->GetOutputPose(), inputs.at(1)->GetOutputPose(), u->floatVariable);

				gef::SkeletonPose secondPose = blendTree->GetPose();
				secondPose.Linear2PoseBlend(inputs.at(2)->GetOutputPose(), inputs.at(3)->GetOutputPose(), u->floatVariable);

				outputPose.Linear2PoseBlend(firstPose, secondPose, v->floatVariable);
			}
			else
				outputIsValid = false;
		}

		return outputIsValid;
	}
}
