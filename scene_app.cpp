#include "scene_app.h"
#include <system/platform.h>
#include <graphics/sprite_renderer.h>
#include <graphics/font.h>
#include <system/debug_log.h>
#include <graphics/renderer_3d.h>
#include <graphics/mesh.h>
#include <graphics/primitive.h>
#include <graphics/image_data.h>
#include <maths/math_utils.h>
#include <input/sony_controller_input_manager.h>
#include <input/keyboard.h>
#include <maths/math_utils.h>
#include <graphics/renderer_3d.h>
#include <graphics/scene.h>
#include <animation/skeleton.h>
#include <animation/animation.h>
#include <graphics/sprite.h>
#include "utilities/load_texture.h"
#include <platform/d3d11/system/platform_d3d11.h>

#include "anim3d/BlendTree.h"

SceneApp::SceneApp(gef::Platform& platform) :
	Application(platform),
	sprite_renderer_(NULL),
	input_manager_(NULL),
	font_(NULL),
	mesh_(NULL),
	player_(NULL),
	renderer_3d_(NULL),
	model_scene_(NULL),
	walkAnim(NULL),
	skeletalSpriteTexture(NULL),
	d3dPlatform(dynamic_cast<gef::PlatformD3D11&>(platform_)),
	primitive_builder_(NULL),
	primitive_renderer_(NULL),
	dynamics_world_(NULL),
	solver_(NULL),
	overlapping_pair_cache_(NULL),
	dispatcher_(NULL),
	debug_drawer_(NULL),
	floor_mesh_(NULL),
	sphere_mesh_(NULL),
	sphere_rb_(NULL)
{
}

void SceneApp::Init()
{
	sprite_renderer_ = gef::SpriteRenderer::Create(platform_);
	renderer_3d_ = gef::Renderer3D::Create(platform_);
	input_manager_ = gef::InputManager::Create(platform_);

	InitFont();
	SetupCamera();
	SetupLights();

	// create a new scene object and read in the data from the file
	// no meshes or materials are created yet
	// we're not making any assumptions about what the data may be loaded in for
	model_scene_ = new gef::Scene();
	model_scene_->ReadSceneFromFile(platform_, "xbot/xbot.scn");
	
	// we do want to render the data stored in the scene file so lets create the materials from the material data present in the scene file
	model_scene_->CreateMaterials(platform_);
	
	// if there is mesh data in the scene, create a mesh to draw from the first mesh
	mesh_ = GetFirstMesh(model_scene_);
	
	// get the first skeleton in the scene
	gef::Skeleton* skeleton = GetFirstSkeleton(model_scene_);

	// anims
	idleAnim = LoadAnimation("xbot/xbot@idle.scn", "");
	walkAnim = LoadAnimation("xbot/xbot@walking_inplace.scn", "");
	runAnim = LoadAnimation("xbot/xbot@running_inplace.scn", "");
	jumpAnim = LoadAnimation("xbot/xbot@jump.scn", "");

	if (skeleton)
	{
		player_ = new gef::SkinnedMeshInstance(*skeleton);
		blendedPose = player_->bind_pose();
		player_->set_mesh(mesh_);

		blendTree = new AnimationLib::BlendTree(player_->bind_pose());

		simulatingPhysics = false;
		enablePhysicsSimulation = new AnimationLib::BlendNodeBool(simulatingPhysics);
		blendTree->InsertVariable("enablePhysicsSimulation", enablePhysicsSimulation);

		blendAlpha = 0.5f;
		blendAlphaTreeVariable = new AnimationLib::BlendNodeFloat(blendAlpha);
		blendTree->InsertVariable("BlendAlpha", blendAlphaTreeVariable);

		blendUAxis = 0.0f;
		blendUAxisTreeVariable = new AnimationLib::BlendNodeFloat(blendUAxis);
		blendTree->InsertVariable("BlendUAxis", blendUAxisTreeVariable);

		blendVAxis = 1.0f;
		blendVAxisTreeVariable = new AnimationLib::BlendNodeFloat(blendVAxis);
		blendTree->InsertVariable("BlendVAxis", blendVAxisTreeVariable);


		idleClipNode = new AnimationLib::ClipNode(blendTree);
		idleClipNode->SetClip(idleAnim);
		idleClipNode->SetLooping(true);

		walkClipNode = new AnimationLib::ClipNode(blendTree);
		walkClipNode->SetClip(walkAnim);
		walkClipNode->SetLooping(true);

		runClipNode = new AnimationLib::ClipNode(blendTree);
		runClipNode->SetClip(runAnim);
		runClipNode->SetLooping(true);

		jumpClipNode = new AnimationLib::ClipNode(blendTree);
		jumpClipNode->SetClip(jumpAnim);
		jumpClipNode->SetLooping(true);
		
		linearBlendNode = new AnimationLib::LinearBlendNode(blendTree);
		linearBlendNode->SetInput(walkClipNode, 0);
		linearBlendNode->SetInput(runClipNode, 1);
		linearBlendNode->SetVariable("BlendAlpha", 0);

		bilinearBlendNode = new AnimationLib::BilinearBlendNode(blendTree);
		bilinearBlendNode->SetInput(walkClipNode, 0);
		bilinearBlendNode->SetInput(runClipNode, 1);
		bilinearBlendNode->SetInput(idleClipNode, 2);
		bilinearBlendNode->SetInput(jumpClipNode, 3);
		bilinearBlendNode->SetVariable("BlendUAxis", 0);
		bilinearBlendNode->SetVariable("BlendVAxis", 1);


		primitive_builder_ = new PrimitiveBuilder(platform_);
		primitive_renderer_ = new PrimitiveRenderer(platform_);

		InitPhysicsWorld();

		ragdollNode = new AnimationLib::RagdollNode(blendTree, dynamics_world_, "xbot/ragdoll.bullet");
		ragdollNode->SetScaleFactor(0.01f);

		btVector3 groundHalfExtents(btScalar(50.), btScalar(1), btScalar(50.));
		btCollisionShape* groundShape = new btBoxShape(groundHalfExtents);

		collision_shapes_.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -groundHalfExtents.y(), 0));

		floor_mesh_ = primitive_builder_->CreateBoxMesh(gef::Vector4(groundHalfExtents.x(), groundHalfExtents.y(), groundHalfExtents.z()));
		floor_gfx_.set_mesh(floor_mesh_);
		floor_gfx_.set_transform(ragdollNode->btTransform2Matrix(groundTransform));

		CreateRigidBodies();

		//ragdollNode->SetInput(linearBlendNode, 0);
		ragdollNode->SetInput(bilinearBlendNode, 0);

		blendTree->GetOutputNode().SetInput(ragdollNode, 0);
	}

	framedSpeed = 1.0f;
	skeletalSpeed = 10.0f;

	///2D animation stuff
	boyAttack = new AnimationLib::Character2DFramed();
	boyFall = new AnimationLib::Character2DFramed();
	boyWin = new AnimationLib::Character2DFramed();

	boyAttackTexture = CreateTextureFromPNG("boy-attack/boy-attack_tex.png", platform_);
	boyFallTexture = CreateTextureFromPNG("boy-fall/boy-fall_tex.png", platform_);
	boyWinTexture = CreateTextureFromPNG("boy-win/boy-win_tex.png", platform_);

	boyAttack->LoadData("boy-attack/boy-attack_tex.json", "boy-attack/boy-attack_ske.json");
	boyFall->LoadData("boy-fall/boy-fall_tex.json", "boy-fall/boy-fall_ske.json");
	boyWin->LoadData("boy-win/boy-win_tex.json", "boy-win/boy-win_ske.json");

	std::string attackFrames[] = { "4_attack_0", "4_attack_1", "4_attack_2", "4_attack_3", "4_attack_4" };
	std::string fallFrames[] = {"4_fall_6", "4_fall_0", "4_fall_1", "4_fall_4", "4_fall_5", "4_fall_3", "4_fall_2"};
	std::string winFrames[] = {"4_win_1", "4_win_5", "4_win_0", "4_win_2", "4_win_4"};

	for (auto frame : attackFrames)
	{
		boyAttack->SetFrameOrder(frame);
	}

	for (auto frame : fallFrames)
	{
		boyFall->SetFrameOrder(frame);
	}

	for (auto frame : winFrames)
	{
		boyWin->SetFrameOrder(frame);
	}

	framedSprite.set_texture(boyAttackTexture);

	framedSpritePos = gef::Vector2(platform_.width() * 0.33f, platform_.height() * 0.5f);

	framedSprite.set_position(gef::Vector4(framedSpritePos.x, framedSpritePos.y, 0.0f));
	framedSprite.set_width(512.0f);
	framedSprite.set_height(512.0f);

	framedAnimIndex = 0;
	skeletalAnimIndex = 0;

	skeletalSpriteTexture = CreateTextureFromPNG("dragon/Dragon_tex.png", platform_);
	skeletalSprite.set_texture(skeletalSpriteTexture);
	
	skeletalSpritePos = gef::Vector2(platform_.width() * 0.75f, platform_.height() * 0.5f);
	
	skeletalSprite.set_position(gef::Vector4(skeletalSpritePos.x, skeletalSpritePos.y, 0.0f));
	skeletalSprite.set_width(512.0f);
	skeletalSprite.set_height(512.0f);

	skeletalCharacter = new AnimationLib::Character2DSkeletal();
	skeletalCharacter->LoadData("dragon/Dragon_tex.json", "dragon/Dragon_ske.json");

	scale = 0.5f;
	
	rigTransform.SetIdentity();
	rigTransform.Scale(gef::Vector2(scale, scale));
	rigTransform.SetTranslation(gef::Vector2(skeletalSpritePos.x, skeletalSpritePos.y));

	render2D = true;
	render3D = true;

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	guiContext = ImGui::CreateContext();
	ImGui::SetCurrentContext(guiContext);

	ImGui_ImplWin32_Init(d3dPlatform.hwnd());
	ImGui_ImplDX11_Init(d3dPlatform.device(), d3dPlatform.device_context());

	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();

	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x / 4.0f, io.DisplaySize.y));

	editorContext = ax::NodeEditor::CreateEditor();
	ax::NodeEditor::SetCurrentEditor(editorContext);
}

void SceneApp::CleanUp()
{
	CleanUpFont();

	delete player_;
	player_ = NULL;

	delete walkAnim;
	walkAnim = NULL;

	delete mesh_;
	mesh_ = NULL;

	delete model_scene_;
	model_scene_ = NULL;

	delete input_manager_;
	input_manager_ = NULL;

	delete sprite_renderer_;
	sprite_renderer_ = NULL;

	delete renderer_3d_;
	renderer_3d_ = NULL;

	ax::NodeEditor::DestroyEditor(editorContext);
}

bool SceneApp::Update(float frame_time)
{
	fps_ = 1.0f / frame_time;

	UpdatePhysicsWorld(frame_time);
	UpdateRigidBodies();

	// read input devices
	if (input_manager_)
	{
		input_manager_->Update();

		// keyboard input
		gef::Keyboard* keyboard = input_manager_->keyboard();
		if (keyboard)
		{
			if (keyboard->IsKeyPressed(keyboard->KC_R))
				render2D = !render2D;

			float m = 1.0f;

			if (keyboard->IsKeyDown(keyboard->KC_LEFT))
				skeletalSpeed = (skeletalSpeed <= 0.0f) ? 0.0f : skeletalSpeed - 0.02f * m;

			if (keyboard->IsKeyDown(keyboard->KC_RIGHT))
				skeletalSpeed = (skeletalSpeed >= 10.0f) ? 10.0f : skeletalSpeed + 0.02f * m;
		}

		//Switching which animation is being played depending on the framed character animation index
		switch(framedAnimIndex)
		{
		case 0:
			boyAttack->PlayAnimation(framedSprite, *boyAttack->GetTextureAtlas(), frame_time);
			break;
		case 1:
			boyFall->PlayAnimation(framedSprite, *boyFall->GetTextureAtlas(), frame_time);
			break;
		case 2:
			boyWin->PlayAnimation(framedSprite, *boyWin->GetTextureAtlas(), frame_time);
			break;
		default:
			boyAttack->PlayAnimation(framedSprite, *boyAttack->GetTextureAtlas(), frame_time);
		}

		skeletalCharacter->PlayAnimation(skeletalAnimIndex, frame_time, skeletalSpeed);
	}

	if (player_)
	{
		//update the nodes in the blend tree
		blendTree->UpdateTree(frame_time);
		// update the bone matrices that are used for rendering the character
		player_->UpdateBoneMatrices(blendTree->GetPose());
	}

	// build a transformation matrix that will position the character
	// use this to move the player around, scale it, etc.
	if (player_)
	{
		gef::Matrix44 player_transform;
		gef::Matrix44 player_scale;
		gef::Matrix44 player_rotate;
		gef::Matrix44 player_translate;
	
		player_transform.SetIdentity();
		player_scale.SetIdentity();
		player_rotate.SetIdentity();
		player_translate.SetIdentity();
	
		/*
		 * Build transforms here
		 */
	
		player_scale.Scale(gef::Vector4(0.01f, 0.01f, 0.01f, 1.0f));
		//player_rotate.RotationY(gef::DegToRad(45.0f));
		//player_translate.SetTranslation(gef::Vector4(25.0f, 0.0f, -100.0f, 1.0f));
	
		//player_transform = player_scale * player_rotate * player_translate;
		player_transform = player_scale;
	
		player_->set_transform(player_transform);
	}

	return true;
}

void SceneApp::Render()
{
	// setup view and projection matrices
	gef::Matrix44 projection_matrix;
	gef::Matrix44 view_matrix;
	projection_matrix = platform_.PerspectiveProjectionFov(camera_fov_, (float)platform_.width() / (float)platform_.height(), near_plane_, far_plane_);
	view_matrix.LookAt(camera_eye_, camera_lookat_, camera_up_);
	renderer_3d_->set_projection_matrix(projection_matrix);
	renderer_3d_->set_view_matrix(view_matrix);
	
	// draw meshes here
	renderer_3d_->Begin();

	if (render3D)
	{
		renderer_3d_->DrawMesh(floor_gfx_);
		renderer_3d_->DrawMesh(sphere_gfx_);

		// draw the player, the pose is defined by the bone matrices
		if (player_)
			renderer_3d_->DrawSkinnedMesh(*player_, player_->bone_matrices());

		if (dynamics_world_)
			dynamics_world_->debugDrawWorld();
	}
	
	renderer_3d_->End();

	sprite_renderer_->Begin(false);

	if (render2D)
	{
		for (auto& item : skeletalCharacter->GetSlots())
		{
			skeletalCharacter->UpdateTextureTransforms(skeletalSprite, *skeletalCharacter->GetTextureAtlas(), item.name);

			//to display the sprite in the correct position, rotation, scale
			gef::Matrix33 frameTransform = skeletalCharacter->GetTextureAtlas()->GetSubTextures()[item.name].transform;
			gef::Matrix33 spriteOffset = skeletalCharacter->GetSkinSlotAt(0, item.name).transform;	//offset transform is so we crop the sprite correctly
			gef::Matrix33 boneWorldTransform = skeletalCharacter->GetBones()[item.name]->worldTransform;
			gef::Matrix33 transform = frameTransform * spriteOffset * boneWorldTransform * rigTransform;

			sprite_renderer_->DrawSprite(skeletalSprite, transform);
		}

		//Not an ideal way of animation switching for 2D frame-based, but Character class encapsulates animation functions and can act like a clip player
		if (framedAnimIndex == 0)
		{
			for (auto& item : boyAttack->GetTextureAtlas()->GetSubTextures())
			{
				if (boyAttack->GetFrameName() == item.first)
					boyAttack->UpdateTextureTransforms(framedSprite, framedSpritePos.x, framedSpritePos.y, *boyAttack->GetTextureAtlas(), item.first);
			}
		}

		else if (framedAnimIndex == 1)
		{
			for (auto& item : boyFall->GetTextureAtlas()->GetSubTextures())
			{
				if (boyFall->GetFrameName() == item.first)
					boyFall->UpdateTextureTransforms(framedSprite, framedSpritePos.x, framedSpritePos.y, *boyFall->GetTextureAtlas(), item.first);
			}
		}

		else if(framedAnimIndex == 2)
		{
			for (auto& item : boyWin->GetTextureAtlas()->GetSubTextures())
			{
				if (boyWin->GetFrameName() == item.first)
					boyWin->UpdateTextureTransforms(framedSprite, framedSpritePos.x, framedSpritePos.y, *boyWin->GetTextureAtlas(), item.first);
			}
		}

		sprite_renderer_->DrawSprite(framedSprite);
	}

	DrawHUD();

	sprite_renderer_->End();
}

void SceneApp::InitFont()
{
	font_ = new gef::Font(platform_);
	font_->Load("comic_sans");
}

void SceneApp::CleanUpFont()
{
	delete font_;
	font_ = NULL;
}

void SceneApp::DrawHUD()
{
	if(font_)
	{
		// display frame rate
		font_->RenderText(sprite_renderer_, gef::Vector4(950.0f, 510.0f, -0.9f), 1.0f, 0xffffffff, gef::TJ_RIGHT, "FPS: %.1f", fps_);
	}

	ImGuiRender();
}

void SceneApp::SetupLights()
{
	gef::PointLight default_point_light;
	default_point_light.set_colour(gef::Colour(0.7f, 0.7f, 1.0f, 1.0f));
	default_point_light.set_position(gef::Vector4(-300.0f, -500.0f, 100.0f));

	gef::Default3DShaderData& default_shader_data = renderer_3d_->default_shader_data();
	default_shader_data.set_ambient_light_colour(gef::Colour(0.5f, 0.5f, 0.5f, 1.0f));
	default_shader_data.AddPointLight(default_point_light);
}

void SceneApp::SetupCamera()
{
	// initialise the camera settings
	camera_eye_ = gef::Vector4(-1.0f, 1.0f, 4.0f);
	camera_lookat_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	camera_up_ = gef::Vector4(0.0f, 1.0f, 0.0f);
	camera_fov_ = gef::DegToRad(45.0f);
	near_plane_ = 0.01f;
	far_plane_ = 1000.f;
}

void SceneApp::InitPhysicsWorld()
{
	/// collision configuration contains default setup for memory , collision setup . Advanced users can create their own configuration .
	btDefaultCollisionConfiguration* collision_configuration = new btDefaultCollisionConfiguration();

	/// use the default collision dispatcher . For parallel processing you can use a diffent dispatcher(see Extras / BulletMultiThreaded)
	dispatcher_ = new btCollisionDispatcher(collision_configuration);

	/// btDbvtBroadphase is a good general purpose broadphase . You can also try out btAxis3Sweep .
	overlapping_pair_cache_ = new btDbvtBroadphase();

	/// the default constraint solver . For parallel processing you can use a different solver (see Extras / BulletMultiThreaded)
	solver_ = new btSequentialImpulseConstraintSolver;

	dynamics_world_ = new btDiscreteDynamicsWorld(dispatcher_, overlapping_pair_cache_, solver_, collision_configuration);
	dynamics_world_->setGravity(btVector3(0, -9.8f, 0));

	debug_drawer_ = new GEFDebugDrawer(renderer_3d_);
	debug_drawer_->setDebugMode(btIDebugDraw::DBG_DrawAabb | btIDebugDraw::DBG_DrawFrames);
	dynamics_world_->setDebugDrawer(debug_drawer_);
}

void SceneApp::CleanUpPhysicsWorld()
{
	delete debug_drawer_;
	debug_drawer_ = NULL;

	for (int i = dynamics_world_->getNumConstraints() - 1; i >= 0; i--)
	{
		btTypedConstraint* constraint = dynamics_world_->getConstraint(i);
		dynamics_world_->removeConstraint(constraint);
		delete constraint;
	}


	// remove the rigidbodies from the dynamics world and delete them
	for (int i = dynamics_world_->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = dynamics_world_->getCollisionObjectArray()[i];
		btRigidBody* body = btRigidBody::upcast(obj);
		if (body && body->getMotionState())
		{
			delete body->getMotionState();
		}
		dynamics_world_->removeCollisionObject(obj);
		delete obj;
	}

	// delete collision shapes
	for (int j = 0; j < collision_shapes_.size(); j++)
	{
		btCollisionShape* shape = collision_shapes_[j];
		collision_shapes_[j] = 0;
		delete shape;
	}

	// delete dynamics world
	delete dynamics_world_;

	// delete solver
	delete solver_;

	// delete broadphase
	delete overlapping_pair_cache_;

	// delete dispatcher
	delete dispatcher_;

	dynamics_world_ = NULL;
	solver_ = NULL;
	overlapping_pair_cache_ = NULL;
	dispatcher_ = NULL;

	// next line is optional : it will be cleared by the destructor when the array goes out of scope
	collision_shapes_.clear();
}

void SceneApp::UpdatePhysicsWorld(float delta_time)
{
	const btScalar simulation_time_step = 1.0f / 60.0f;
	const int max_sub_steps = 1;
	dynamics_world_->stepSimulation(simulation_time_step, max_sub_steps);
}

void SceneApp::CreateRigidBodies()
{
	//the ground is a cube of side 100 at position y = 0.
	{
		btVector3 groundHalfExtents(btScalar(50.), btScalar(1.), btScalar(50.));
		btCollisionShape* groundShape = new btBoxShape(groundHalfExtents);

		collision_shapes_.push_back(groundShape);

		btTransform groundTransform;
		groundTransform.setIdentity();
		groundTransform.setOrigin(btVector3(0, -groundHalfExtents.y(), 0));

		btScalar mass(0.);

		//rigidbody is dynamic if and only if mass is non zero, otherwise static
		bool isDynamic = (mass != 0.f);

		btVector3 localInertia(0, 0, 0);
		if (isDynamic)
			groundShape->calculateLocalInertia(mass, localInertia);

		//using motionstate is optional, it provides interpolation capabilities, and only synchronizes 'active' objects
		btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
		btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
		btRigidBody* body = new btRigidBody(rbInfo);

		//add the body to the dynamics world
		dynamics_world_->addRigidBody(body);

		floor_mesh_ = primitive_builder_->CreateBoxMesh(gef::Vector4(groundHalfExtents.x(), groundHalfExtents.y(), groundHalfExtents.z()));
		floor_gfx_.set_mesh(floor_mesh_);
		floor_gfx_.set_transform(ragdollNode->btTransform2Matrix(groundTransform));
	}
}

void SceneApp::CleanUpRigidBodies()
{
	delete sphere_mesh_;
	sphere_mesh_ = NULL;
	delete floor_mesh_;
	floor_mesh_ = NULL;
}

void SceneApp::UpdateRigidBodies()
{
	if (sphere_rb_)
	{
		btTransform world_transform;
		sphere_rb_->getMotionState()->getWorldTransform(world_transform);
		sphere_gfx_.set_transform(ragdollNode->btTransform2Matrix(world_transform));
	}
}

gef::Skeleton* SceneApp::GetFirstSkeleton(gef::Scene* scene)
{
	gef::Skeleton* skeleton = NULL;
	if (scene)
	{
		// check to see if there is a skeleton in the the scene file
		// if so, pull out the bind pose and create an array of matrices
		// that wil be used to store the bone transformations
		if (scene->skeletons.size() > 0)
			skeleton = scene->skeletons.front();
	}

	return skeleton;
}

gef::Mesh* SceneApp::GetFirstMesh(gef::Scene* scene)
{
	gef::Mesh* mesh = NULL;

	if (scene)
	{
		// now check to see if there is any mesh data in the file, if so lets create a mesh from it
		if (scene->mesh_data.size() > 0)
			mesh = model_scene_->CreateMesh(platform_, scene->mesh_data.front());
	}

	return mesh;
}

gef::Animation* SceneApp::LoadAnimation(const char* anim_scene_filename, const char* anim_name)
{
	gef::Animation* anim = NULL;

	gef::Scene anim_scene;
	if (anim_scene.ReadSceneFromFile(platform_, anim_scene_filename))
	{
		// if the animation name is specified then try and find the named anim
		// otherwise return the first animation if there is one
		std::map<gef::StringId, gef::Animation*>::const_iterator anim_node_iter;
		if (anim_name)
			anim_node_iter = anim_scene.animations.find(gef::GetStringId(anim_name));
		else
			anim_node_iter = anim_scene.animations.begin();

		if (anim_node_iter != anim_scene.animations.end())
			anim = new gef::Animation(*anim_node_iter->second);
	}

	return anim;
}

void SceneApp::ImGuiRender()
{
	ImGuiIO& io = ImGui::GetIO();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Content", nullptr, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

	ImGui::Text("2D Animation");

	if(ImGui::Button("Render 2D Assets"))
	{
		render2D = !render2D;
	}

	static float guiFramedSpeed = framedSpeed;

	if (ImGui::SliderFloat("Frame Anim. Speed", &guiFramedSpeed, 0.01f, 2.5f))
	{
		framedSpeed = guiFramedSpeed;

		boyAttack->SetCustomSpeed(framedSpeed);
		boyFall->SetCustomSpeed(framedSpeed);
		boyWin->SetCustomSpeed(framedSpeed);
	}

	static int guiFramedAnimIndex = framedAnimIndex;

	if (ImGui::SliderInt("Frame Anim. Index", &guiFramedAnimIndex, 0, 2))
	{
		framedAnimIndex = guiFramedAnimIndex;

		switch (framedAnimIndex)
		{
		case 0:
			framedSprite.set_texture(boyAttackTexture);
			break;
		case 1:
			framedSprite.set_texture(boyFallTexture);
			break;
		case 2:
			framedSprite.set_texture(boyWinTexture);
			break;
		default:
			framedSprite.set_texture(boyAttackTexture);
		}
	}

	static float guiSkeletalSpeed = skeletalSpeed;

	if (ImGui::SliderFloat("Skeletal. Anim. Speed", &guiSkeletalSpeed, 0.0f, 25.0f))
	{
		skeletalSpeed = guiSkeletalSpeed;
	}

	static int guiSkeletalAnimIndex = skeletalAnimIndex;

	if (ImGui::SliderInt("Skeletal Anim. Index", &guiSkeletalAnimIndex, 0, 3))
	{
		skeletalAnimIndex = guiSkeletalAnimIndex;
	}

	ImGui::Separator();
	ImGui::Text("3D Animation");

	if (ImGui::Button("Render 3D Assets"))
	{
		render3D = !render3D;
	}

	static bool guiSimulatePhysics = simulatingPhysics;

	if(ImGui::Checkbox("Simulate Physics", &guiSimulatePhysics))
	{
		simulatingPhysics = guiSimulatePhysics;
		enablePhysicsSimulation->boolVariable = simulatingPhysics;
	}

	//Omitting this as default application is set up to use bilinear blend

	//static float guiBlendAlpha = blendAlpha;
	//
	//if (ImGui::SliderFloat("Blend Alpha", &guiBlendAlpha, 0.0f, 1.0f))
	//{
	//	blendAlpha = guiBlendAlpha;
	//	blendAlphaTreeVariable->floatVariable = blendAlpha;
	//}

	static float guiUAxis = blendUAxis;

	if (ImGui::SliderFloat("U Axis Blend", &guiUAxis, 0.0f, 1.0f))
	{
		blendUAxis = guiUAxis;
		blendUAxisTreeVariable->floatVariable = blendUAxis;
	}

	static float guiVAxis = blendVAxis;

	if (ImGui::SliderFloat("V Axis Blend", &guiVAxis, 0.0f, 1.0f))
	{
		blendVAxis = guiVAxis;
		blendVAxisTreeVariable->floatVariable = blendVAxis;
	}


	//Removed the node editor for now

	//ImGui::Text("Node Graph");
	//
	//ax::NodeEditor::SetCurrentEditor(editorContext);
	//ax::NodeEditor::Begin("My Editor", ImVec2(0.0, 0.0f));
	//int uniqueId = 1;
	//// Start drawing nodes.
	//ax::NodeEditor::BeginNode(uniqueId++);
	//ImGui::Text("Clip Node");
	//ax::NodeEditor::BeginPin(uniqueId++, ax::NodeEditor::PinKind::Input);
	//ImGui::Text("-> In");
	//ax::NodeEditor::EndPin();
	//ImGui::SameLine();
	//ax::NodeEditor::BeginPin(uniqueId++, ax::NodeEditor::PinKind::Output);
	//ImGui::Text("Out ->");
	//ax::NodeEditor::EndPin();
	//ax::NodeEditor::EndNode();
	//
	//ax::NodeEditor::BeginNode(uniqueId++);
	//ImGui::Text("Output Node");
	//ax::NodeEditor::BeginPin(uniqueId++, ax::NodeEditor::PinKind::Input);
	//ImGui::Text("-> In");
	//ax::NodeEditor::EndPin();
	//ax::NodeEditor::EndNode();
	//
	//ax::NodeEditor::End();
	//ax::NodeEditor::SetCurrentEditor(nullptr);

	ImGui::End();

	// Rendering
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}