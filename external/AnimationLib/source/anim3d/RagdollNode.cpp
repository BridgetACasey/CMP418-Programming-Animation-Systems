//@BridgetACasey - 1802644

#include "RagdollNode.h"

#include "BlendTree.h"
#include "system/debug_log.h"
#include "btBulletWorldImporter.h"

namespace AnimationLib
{
	RagdollNode::RagdollNode() : BlendNodeBase(nullptr)
	{
	}

	RagdollNode::RagdollNode(BlendTree* tree, btDiscreteDynamicsWorld* dynamicsWorld, const char* modelName) : BlendNodeBase(tree)
	{
		inputs.resize(1);

		outputPose = tree->GetPose();
		pose = outputPose;

		gef::Matrix44 identity;
		identity.SetIdentity();

		const Int32 jointCount = outputPose.skeleton()->joint_count();

		//Match the container sizes to the number of joints in the skeleton
		boneWorldMatrices.resize(jointCount);
		boneOffsetMatrices.resize(jointCount, identity);
		boneRigidBodies.resize(jointCount, nullptr);
		boneNames.resize(jointCount);

		boneOffsetMatrices.resize(outputPose.skeleton()->joint_count(), identity);
		boneRigidBodies.resize(outputPose.skeleton()->joint_count(), NULL);
		boneWorldMatrices.resize(outputPose.skeleton()->joint_count());

		btBulletWorldImporter* fileLoader = new btBulletWorldImporter(dynamicsWorld);

		fileLoader->loadFile(modelName);

		int numRigidBodies = fileLoader->getNumRigidBodies();

		for (Int32 rigidBodyIndex = 0; rigidBodyIndex < numRigidBodies; rigidBodyIndex++)
		{
			//Cast collision object to current rigidbody object and retrieve modified name
			btCollisionObject* obj = fileLoader->getRigidBodyByIndex(rigidBodyIndex);
			btRigidBody* body = btRigidBody::upcast(obj);
			std::string rigidBodyName(fileLoader->getNameForPointer(body));
			rigidBodyName = std::string(&rigidBodyName.c_str()[sizeof("OBArmature_") - 1]);
			char* newName = const_cast<char*>(rigidBodyName.c_str());

			newName[rigidBodyName.length() - sizeof("_hitbox") + 1] = 0;
			rigidBodyName = std::string(newName);

			gef::DebugOut("  object name = %s\n", rigidBodyName.c_str());
			// Blender CoM
			gef::DebugOut("  get position = (%4.3f,%4.3f,%4.3f)\n", body->getCenterOfMassPosition().getX(), body->getCenterOfMassPosition().getY(), body->getCenterOfMassPosition().getZ());

			//If the mass is not 0, the object is treated as dynamic
			if (static_cast<Int32>(body->getInvMass()) == 0)
			{
				gef::DebugOut("  static object\n");
			}
			else
			{
				gef::DebugOut("  mass = %4.3f\n", 1.0f / body->getInvMass());		// Blender Mass
			}
			gef::DebugOut("\n");

			//If the skeleton exists
			if (outputPose.skeleton())
			{
				//Retrieve the name of the joint
				const gef::StringId jointNameID = gef::GetStringId(rigidBodyName);


				if (jointNameID != 0)
				{
					//Find the bone in skeleton that matches the name of the rigid body
					const Int32 jointNumber = outputPose.skeleton()->FindJointIndex(jointNameID);


					if (jointNumber != -1)
					{
						boneNames[jointNumber] = rigidBodyName;
						boneRigidBodies[jointNumber] = body;

						// CALCULATE THE BONE TO RIGID BODY OFFSET MATRIX HERE

						gef::Matrix44 boneWorldPoseMatrix = outputPose.global_pose()[jointNumber];
						gef::Vector4 boneWorldPoseTranslation = boneWorldPoseMatrix.GetTranslation();
						const float scaleFactor = 0.01f;
						boneWorldPoseMatrix.SetTranslation(boneWorldPoseTranslation * scaleFactor);

						//Get the inverse matrix
						gef::Matrix44 boneWorldPoseInverseMatrix;
						boneWorldPoseInverseMatrix.AffineInverse(boneWorldPoseMatrix);

						//Get centre of mass for rigid body transform
						const btTransform rbWbtTransform = body->getCenterOfMassTransform();

						//Convert to gef format and calculate offset
						const gef::Matrix44 rbWorldTransform = btTransform2Matrix(rbWbtTransform);
						const gef::Matrix44 rbOffset = rbWorldTransform * boneWorldPoseInverseMatrix;

						//Store the offset matrix for the bone
						boneOffsetMatrices[jointNumber] = rbOffset;
					}
				}
			}
		}

		delete fileLoader;
		fileLoader = NULL;
	}

	RagdollNode::~RagdollNode()
	{
	}

	bool RagdollNode::ProcessNode(float frameTime)
	{
		BlendNodeBool* simulatePhysics = static_cast<BlendNodeBool*>(blendTree->GetTreeVariables().at("enablePhysicsSimulation"));

		if(simulatePhysics != nullptr)
		{
			if(simulatePhysics->boolVariable)
				UpdatePoseFromRagdoll();
			else
				UpdateRagdollFromPose();
		}
		else
		{
			UpdateRagdollFromPose();
		}

		return true;
	}

	void RagdollNode::UpdatePoseFromRagdoll()
	{
		const gef::Skeleton* skeleton = inputs[0]->GetOutputPose().skeleton();

		const auto& skeletonPoses = inputs[0]->GetOutputPose().local_pose();

		for (Int32 boneNum = 0; boneNum < skeleton->joint_count(); ++boneNum)
		{
			const gef::Joint& joint = skeleton->joint(boneNum);
			gef::Matrix44 boneLocalTransform;

			btRigidBody* rigidBody = boneRigidBodies[boneNum];

			if (rigidBody != nullptr)
			{
				//Get the bone local matrix
				boneLocalTransform = skeletonPoses[boneNum].GetMatrix();

				//Get rigid body world transform and offset inverse
				btTransform rbWbtTransform = rigidBody->getCenterOfMassTransform();
				gef::Matrix44 rbWorldTransform = btTransform2Matrix(rbWbtTransform);

				gef::Matrix44 rbOffsetInv;
				rbOffsetInv.AffineInverse(boneOffsetMatrices[boneNum]);

				gef::Matrix44 boneWorldTransform = rbOffsetInv * rbWorldTransform;

				//Scale the translation to the skeleton transform
				gef::Vector4 boneTranslation = boneWorldTransform.GetTranslation();

				const float inverseScaleFactor = 1.0f / 0.01f;

				boneWorldTransform.SetTranslation(boneTranslation * inverseScaleFactor);

				if (joint.parent == -1)
				{
					boneLocalTransform = boneWorldTransform;
				}
				else
				{
					//Bone local transform equals the bone's world transform, achieved by inverting the parent's world transform
					gef::Matrix44 parentBoneWorldTransformInverse;
					parentBoneWorldTransformInverse.AffineInverse(boneWorldMatrices[joint.parent]);

					boneLocalTransform = boneWorldTransform * parentBoneWorldTransformInverse;
				}
			}
			else
			{
				//Otherwise set the bone's local transform to local pose
				boneLocalTransform = skeletonPoses[boneNum].GetMatrix();
			}

			//Calculate the bone world transforms for the animated skeleton
			if (joint.parent == -1)
				boneWorldMatrices[boneNum] = boneLocalTransform;
			else
				boneWorldMatrices[boneNum] = boneLocalTransform * boneWorldMatrices[joint.parent];

		}

		outputPose.CalculateLocalPose(boneWorldMatrices);
		outputPose.CalculateGlobalPose();

	}

	void RagdollNode::UpdateRagdollFromPose()
	{
		outputPose = inputs[0]->GetOutputPose();

		for (int bone_num = 0; bone_num < outputPose.skeleton()->joint_count(); ++bone_num)
		{
			const gef::Joint& joint = outputPose.skeleton()->joint(bone_num);

			btRigidBody* bone_rb = boneRigidBodies[bone_num];
			if (bone_rb)
			{
				// CALCULATE THE RIGID BODY WORLD TRANSFORM BASED ON THE CURRENT SKELETON POSE

				gef::Matrix44 boneWorldTransform = outputPose.global_pose()[bone_num];
				gef::Vector4 boneWorldTranslation = boneWorldTransform.GetTranslation();

				boneWorldTransform.SetTranslation(boneWorldTranslation * 0.01f);

				gef::Matrix44 rbOffsetTransform = boneOffsetMatrices[bone_num];
				gef::Matrix44 rbWorldTransform = rbOffsetTransform * boneWorldTransform;
				btTransform rbWbtTransform = Matrix2btTransform(rbWorldTransform);

				bone_rb->setCenterOfMassTransform(rbWbtTransform);
				bone_rb->setLinearVelocity(btVector3(0.0f, 0.0f, 0.0f));
				bone_rb->setAngularVelocity(btVector3(0.0f, 0.0f, 0.0f));
			}
		}
	}

	gef::Matrix44 RagdollNode::btTransform2Matrix(const btTransform& transform)
	{
		gef::Matrix44 result;

		btQuaternion rotation = transform.getRotation();
		btVector3 position = transform.getOrigin();

		result.Rotation(gef::Quaternion(rotation.x(), rotation.y(), rotation.z(), rotation.w()));
		result.SetTranslation(gef::Vector4(position.x(), position.y(), position.z()));

		return result;
	}

	btTransform RagdollNode::Matrix2btTransform(const gef::Matrix44& matrix)
	{
		gef::Vector4 mtx_position = matrix.GetTranslation();

		gef::Quaternion mtx_rot;
		mtx_rot.SetFromMatrix(matrix);

		btTransform result;
		result.setOrigin(btVector3(mtx_position.x(), mtx_position.y(), mtx_position.z()));
		result.setRotation(btQuaternion(mtx_rot.x, mtx_rot.y, mtx_rot.z, mtx_rot.w));

		return result;
	}
}