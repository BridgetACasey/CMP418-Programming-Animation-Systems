//@BridgetACasey - 1802644

#pragma once

#include "BlendNodeBase.h"

namespace AnimationLib
{
	class ClipNode : public BlendNodeBase
	{
	public:
		ClipNode();
		ClipNode(BlendTree* tree);

		bool ProcessNode(float frameTime) override;

		void SetPlaybackSpeed(const float speed) { playbackSpeed = speed; }
		const float GetPlaybackSpeed() const { return playbackSpeed; }

		void SetLooping(const bool loop) { looping = loop; }
		const bool GetLooping() const { return looping; }

		void SetClip(const gef::Animation* c) { clip = c; }
		const gef::Animation* GetClip() const { return clip; }

	protected:
		/// A pointer to the animation GetClip to be sampled
		const gef::Animation* clip;

		/// The current playback time the animation GetClip is being sampled at
		float animTime;

		/// The playback speed scaling factor used to adjust the speed the animation is played at
		float playbackSpeed;

		/// The flag indicating whether the playback is to be looped or not
		bool looping;
	};
}