//@BridgetACasey - 1802644

#pragma once

#include "BlendNodeBase.h"

#include "btBulletDynamicsCommon.h"

namespace AnimationLib
{
	class RagdollNode : public BlendNodeBase
	{
	public:
		RagdollNode();
		RagdollNode(BlendTree* tree, btDiscreteDynamicsWorld* dynamicsWorld, const char* modelName);
		~RagdollNode();

		bool ProcessNode(float frameTime) override;

		void UpdatePoseFromRagdoll();
		void UpdateRagdollFromPose();

		gef::Matrix44 btTransform2Matrix(const btTransform& transform);
		btTransform Matrix2btTransform(const gef::Matrix44& matrix);

		void SetScaleFactor(float scale) { scaleFactor = scale; }
		float GetScaleFactor() { return scaleFactor; }

	private:
		std::vector<btRigidBody*> boneRigidBodies;

		std::vector<gef::Matrix44> boneWorldMatrices;
		std::vector<gef::Matrix44> boneOffsetMatrices;

		gef::SkeletonPose pose;

		float scaleFactor;

		std::vector<std::string> boneNames;
	};
}
