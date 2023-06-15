//@BridgetACasey - 1802644

#pragma once

#include <utility>

#include "animation/skeleton.h"

namespace AnimationLib
{
	class BlendTree;

	class BlendNodeBase
	{
	public:
		BlendNodeBase(BlendTree* tree);
		~BlendNodeBase();

		virtual bool UpdateNode(float frameTime);
		virtual bool ProcessNode(float frameTime) = 0;

		void SetInput(BlendNodeBase* inputNode, int inputIndex = 0);
		void SetVariable(std::string variable, int inputIndex = 0);

		gef::SkeletonPose& GetOutputPose() { return outputPose; }

	protected:
		BlendTree* blendTree;

		std::vector<BlendNodeBase*> inputs;
		std::vector<std::string> variables;

		gef::SkeletonPose outputPose;
	};
}
