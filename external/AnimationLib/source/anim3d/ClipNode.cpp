//@BridgetACasey - 1802644

#include "ClipNode.h"

#include "BlendTree.h"
#include "animation/animation.h"

namespace AnimationLib
{
	ClipNode::ClipNode() : BlendNodeBase(nullptr)
	{
		clip = nullptr;
		animTime = 0.0f;
		playbackSpeed = 1.0f;
		looping = false;
	}

	ClipNode::ClipNode(BlendTree* tree) : BlendNodeBase(tree)
	{
		clip = nullptr;
		animTime = 0.0f;
		playbackSpeed = 1.0f;
		looping = false;
		outputPose = tree->GetPose();
	}

	bool ClipNode::ProcessNode(float frameTime)
	{
		bool finished = false;

		if (clip)
		{
			// update the animation playback time
			animTime += frameTime * playbackSpeed;

			// check to see if the playback has reached the end of the animation
			if (animTime > clip->duration())
			{
				// if the animation is looping then wrap the playback time round to the beginning of the animation
				if (looping)
					animTime = std::fmodf(animTime, clip->duration());
				else
				{
					// otherwise set the playback time to the end of the animation and flag that we have reached the end
					animTime = clip->duration();
					finished = true;
				}
			}

			// add the clip start time to the playback time to calculate the final time
			// that will be used to sample the animation data
			float time = animTime + clip->start_time();

			// sample the animation data at the calculated time
			// any bones that don't have animation data are set to the bind pose
			outputPose.SetPoseFromAnim(*clip, blendTree->GetPose(), time);
		}
		else
		{
			// no animation associated with this player, just set the pose to the bind pose
			outputPose = blendTree->GetPose();
		}

		if (looping)
			finished = true;

		return finished;
	}
}