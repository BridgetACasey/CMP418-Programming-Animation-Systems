#ifndef _SCENE_APP_H
#define _SCENE_APP_H

#include <system/application.h>
#include <graphics/sprite.h>
#include <input/input_manager.h>
#include <graphics/skinned_mesh_instance.h>

#include "btBulletDynamicsCommon.h"
#include "btBulletWorldImporter.h"
#include "character/Character2DFramed.h"
#include "character/Character2DSkeletal.h"
#include "anim3d/BlendTree.h"
#include "anim3d/ClipNode.h"
#include "anim3d/LinearBlendNode.h"
#include "anim3d/RagdollNode.h"

#include <string>
#include <memory>

#include "anim3d/BilinearBlendNode.h"
#include "GuiEditor/imgui.h"
#include "GuiEditor/imgui_node_editor.h"
#include "GuiEditor/imgui_impl_win32.h"
#include "GuiEditor/imgui_impl_dx11.h"
#include "maths/matrix33.h"
#include "utilities/gef_debug_drawer.h"
#include "utilities/primitive_builder.h"
#include "utilities/primitive_renderer.h"
#include "utilities/vertex_colour_unlit_shader.h"

// FRAMEWORK FORWARD DECLARATIONS
namespace gef
{
	class Platform;
	class SpriteRenderer;
	class Renderer3D;
	class Mesh;
	class Scene;
	class Skeleton;
	class Font;
	class InputManager;
	class PlatformD3D11;
}

class SceneApp : public gef::Application
{
public:
	SceneApp(gef::Platform& platform);
	void Init();

	gef::Skeleton* GetFirstSkeleton(gef::Scene* scene);

	gef::Mesh* GetFirstMesh(gef::Scene* scene);

	void CleanUp();
	bool Update(float frame_time);
	void Render();
private:
	void InitFont();
	void CleanUpFont();
	void DrawHUD();
	void SetupLights();
	void SetupCamera();

	void InitPhysicsWorld();
	void CleanUpPhysicsWorld();
	void UpdatePhysicsWorld(float delta_time);

	void CreateRigidBodies();
	void CleanUpRigidBodies();
	void UpdateRigidBodies();

	gef::Animation* LoadAnimation(const char* anim_scene_filename, const char* anim_name);

	void ImGuiRender();

	gef::SpriteRenderer* sprite_renderer_;
	gef::Renderer3D* renderer_3d_;
	gef::InputManager* input_manager_;
	gef::Font* font_;

	gef::Texture* boyAttackTexture;
	gef::Texture* boyWinTexture;
	gef::Texture* boyFallTexture;
	gef::Texture* skeletalSpriteTexture;
	gef::Sprite skeletalSprite;
	gef::Sprite framedSprite;

	float fps_;
	bool render2D;
	bool render3D;

	class gef::Mesh* mesh_;
	gef::SkinnedMeshInstance* player_;

	gef::Scene* model_scene_;

	gef::Vector4 camera_eye_;
	gef::Vector4 camera_lookat_;
	gef::Vector4 camera_up_;
	float camera_fov_;
	float near_plane_;
	float far_plane_;

	PrimitiveBuilder* primitive_builder_;
	PrimitiveRenderer* primitive_renderer_;

	gef::Animation* idleAnim;
	gef::Animation* walkAnim;
	gef::Animation* runAnim;
	gef::Animation* jumpAnim;
	gef::SkeletonPose blendedPose;
	float framedSpeed, skeletalSpeed;

	void FrontendInit();
	void FrontendRelease();
	void FrontendUpdate(float frame_time);
	void FrontendRender();

	gef::PlatformD3D11& d3dPlatform;

	ImGuiContext* guiContext = nullptr;
	ax::NodeEditor::EditorContext* editorContext = nullptr;

	gef::Vector2 skeletalSpritePos;
	gef::Vector2 framedSpritePos;
	gef::Matrix33 rigTransform;
	float scale;
	int framedAnimIndex;
	int skeletalAnimIndex;

	AnimationLib::Character2DFramed* boyAttack;
	AnimationLib::Character2DFramed* boyFall;
	AnimationLib::Character2DFramed* boyWin;
	AnimationLib::Character2DSkeletal* skeletalCharacter;

	btDiscreteDynamicsWorld* dynamics_world_;
	btSequentialImpulseConstraintSolver* solver_;
	btBroadphaseInterface* overlapping_pair_cache_;
	btCollisionDispatcher* dispatcher_;
	btAlignedObjectArray<btCollisionShape*> collision_shapes_;
	GEFDebugDrawer* debug_drawer_;

	gef::Mesh* floor_mesh_;
	gef::MeshInstance floor_gfx_;

	btRigidBody* sphere_rb_;
	gef::Mesh* sphere_mesh_;
	gef::MeshInstance sphere_gfx_;

	AnimationLib::BlendTree* blendTree;
	AnimationLib::ClipNode* idleClipNode;
	AnimationLib::ClipNode* walkClipNode;
	AnimationLib::ClipNode* runClipNode;
	AnimationLib::ClipNode* jumpClipNode;
	AnimationLib::LinearBlendNode* linearBlendNode;
	AnimationLib::BilinearBlendNode* bilinearBlendNode;
	AnimationLib::RagdollNode* ragdollNode;

	float blendAlpha;	//This value is for linear blending only
	float blendUAxis, blendVAxis;	//These are for bilinear blending
	bool simulatingPhysics;
	AnimationLib::BlendNodeBool* enablePhysicsSimulation;
	AnimationLib::BlendNodeFloat* blendAlphaTreeVariable;
	AnimationLib::BlendNodeFloat* blendUAxisTreeVariable;
	AnimationLib::BlendNodeFloat* blendVAxisTreeVariable;
};

#endif // _SCENE_APP_H