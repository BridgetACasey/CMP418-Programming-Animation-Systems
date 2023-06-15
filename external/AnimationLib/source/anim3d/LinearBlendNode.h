//@BridgetACasey - 1802644

#pragma once

#include "BlendNodeBase.h"

namespace AnimationLib
{
	class LinearBlendNode : public BlendNodeBase
	{
	public:
		LinearBlendNode();
		LinearBlendNode(BlendTree* tree);

		bool ProcessNode(float frameTime) override;
	};
}