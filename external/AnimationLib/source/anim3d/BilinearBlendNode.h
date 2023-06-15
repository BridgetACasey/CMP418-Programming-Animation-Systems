//@BridgetACasey - 1802644

#pragma once

#include "BlendNodeBase.h"

namespace AnimationLib
{
	class BilinearBlendNode : public BlendNodeBase
	{
	public:
		BilinearBlendNode();
		BilinearBlendNode(BlendTree* tree);

		bool ProcessNode(float frameTime) override;
	};
}