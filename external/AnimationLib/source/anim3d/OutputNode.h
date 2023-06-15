//@BridgetACasey - 1802644

#pragma once

#include "BlendNodeBase.h"

namespace AnimationLib
{
	class OutputNode : public BlendNodeBase
	{
	public:
		OutputNode();
		OutputNode(BlendTree* tree);

		bool ProcessNode(float frameTime) override;
	};
}
