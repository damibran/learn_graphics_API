#pragma once

#include "Enttity.h"
#include "Componenets/RelationshipComponent.h"
#include "Componenets/TagComponent.h"
#include "Componenets/BoneComponenet.h"
#include "Componenets/SkeletalModelComponent.h"
#include "Utils/AI_GLM_utils.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/DefaultLogger.hpp>

#include "Wrappers/Animation.h"
#include "Componenets/AnimationComponent.h"
#include "AnimationSequence.h"


namespace dmbrn
{
	// need interface implementation for scene tree
	class Scene
	{
	public:
		Scene():
			scene_root_(registry_, "SceneRoot")
		{
			//ModelImporter::Import(*this, true, "Models\\SkinTest\\RiggedSimple.gltf");

			//ModelImporter::Import(*this,false,"Models\\DoubleTestCube\\DoubleTestCube.dae");

			//ModelImporter::Import(*this,true,"Models\\anim_test.fbx");

			//ModelImporter::Import(*this, true, "Models\\MyTest\\Test.dae");

			//ModelImporter::Import(*this, true, "Models\\Char\\TwoChar@Taunt.gltf");

			ModelImporter::Import(*this, "Models\\Char\\Defeated.dae", true, true);

			//ModelImporter::Import(*this, false,"Models\\DoubleTestCube\\QuadTestCube.dae");

			printSceneRecursively(scene_root_, "");

			animation_sequence_.mFrameMin = 0;
			animation_sequence_.mFrameMax = 800;

			auto view = registry_.view<AnimationComponent>();
			for (auto ent : view)
			{
				AnimationComponent& anim = view.get<AnimationComponent>(ent);

				for (int i = 1; i < 4; ++i)
				{
					animation_sequence_.entries_[Enttity{registry_, ent}].insert({i * 200, &anim.animation_clips[0]});
				}
			}

			//addModel("Models\\Char\\Warrok W Kurniawan.fbx"); //Models\Char\Warrok W Kurniawan.fbx
			//addModel("Models\\Char\\TwoChar.fbx"); //Models\Char\Warrok W Kurniawan.fbx
			//addModel("Models\\SkinTest\\RiggedSimple.dae");
			//addModel("F:\\3D_Scenes\\Sponza\\NewSponza_Main_glTF_002.gltf");
			//addModel("Models\\DoubleTestCube\\DoubleTestCube.fbx");
		}

		void printSceneRecursively(Enttity node, std::string tab)
		{
			const TagComponent& tag = node.getComponent<TagComponent>();
			const RelationshipComponent& node_rc = node.getComponent<RelationshipComponent>();

			std::cout << tab + tag.tag << std::endl;

			Enttity cur_child = node_rc.first;

			while (cur_child)
			{
				printSceneRecursively(cur_child, tab + " ");
				cur_child = cur_child.getComponent<RelationshipComponent>().next;
			}
		}

		size_t getCountOfEntities()
		{
			return registry_.size();
		}

		Enttity addNewEntityToRoot(const std::string& name = std::string{})
		{
			return Enttity{registry_, name, scene_root_};
		}

		Enttity addNewEntityAsChild(Enttity parent, const std::string& name = std::string{})
		{
			return Enttity{registry_, name, parent};
		}

		void deleteEntity(Enttity enttity)
		{
			enttity.destroy();
		}

		void updateGlobalTransforms(uint32_t frame)
		{
			dirtyTraverseTree(scene_root_, frame);
		}

		// for now updates data for all entities
		void updatePerStaticModelData(uint32_t frame)
		{
			char* data = Renderer::per_static_data_buffer_.map(frame);

			if (data)
			{
				auto group = registry_.group<StaticModelComponent>(entt::get<TransformComponent>);

				for (auto entity : group)
				{
					auto [model, transform] = group.get(entity);

					if (model.need_GPU_state_update)
					{
						model.need_GPU_state_update = false;
						auto ubo_data = reinterpret_cast<PerStaticModelData::UBODynamicData*>(data + model.
							inGPU_transform_offset);
						ubo_data->model = transform.globalTransformMatrix;
					}
				}

				Renderer::per_static_data_buffer_.unMap(frame);
			}
		}

		void updatePerSkeletalData(uint32_t frame)
		{
			char* data = Renderer::per_skeleton_data_.map(frame);

			if (data)
			{
				auto view = registry_.view<SkeletalModelComponent>();

				for (auto entt : view)
				{
					SkeletalModelComponent& skeletal_comp = view.get<SkeletalModelComponent>(entt);

					// if is visible
					for (Enttity enttity : skeletal_comp.bone_enttities)
					{
						auto [bone, trans] = enttity.getComponent<BoneComponent, TransformComponent>();

						if (bone.need_gpu_state_update)
						{
							bone.need_gpu_state_update = false;
							auto ubo_data = reinterpret_cast<PerSkeletonData::UBODynamicData*>(data + skeletal_comp.
								in_GPU_mtxs_offset);
							ubo_data->model[bone.bone_ind] = trans.globalTransformMatrix * skeletal_comp.mesh.
								render_data_->
								getOffsetMtxs()[bone.bone_ind]; //glm::mat4(1.f);//
						}
					}
				}

				Renderer::per_skeleton_data_.unMap(frame);
			}
		}


		void updateAnimations(time_point g_time, uint32_t frame)
		{
			auto view = registry_.view<AnimationComponent>();

			//for (auto ent : view)
			//{
			//	AnimationComponent& anim = view.get<AnimationComponent>(ent);
			//
			//	if (anim.playing)
			//	{
			//		AnimationClip& playing_clip = anim.animation_clips[anim.playing_ind];
			//
			//		playing_clip.updateTransforms(anim.getLocalTime(g_time), frame);
			//	}
			//}
		}

		// may perform culling
		auto getModelsToDraw()
		{
			return registry_.view<StaticModelComponent>();
		}

		// may perform culling
		auto getSkeletalModelsToDraw()
		{
			return registry_.view<SkeletalModelComponent>();
		}

		Enttity getNullEntt()
		{
			return Enttity{registry_};
		}

		RelationshipComponent& getRootRelationshipComponent()
		{
			return scene_root_.getComponent<RelationshipComponent>();
		}

		AnimationSequence& getAnimationSequence()
		{
			return animation_sequence_;
		}

	private:
		entt::registry registry_;
		Enttity scene_root_;
		AnimationSequence animation_sequence_;

		class ModelImporter
		{
		public:
			static void Import(Scene& scene, const std::string& path, bool with_anim = false, bool with_bones = false)
			{
				Assimp::DefaultLogger::create("", Assimp::DefaultLogger::VERBOSE, aiDefaultLogStream_STDOUT);

				Assimp::Importer importer;
				const aiScene* ai_scene = importer.ReadFile(
					path, aiProcess_Triangulate | aiProcess_ValidateDataStructure | aiProcess_FlipUVs |
					aiProcess_GlobalScale | (with_bones
						                         ? (aiProcess_PopulateArmatureData | aiProcess_LimitBoneWeights)
						                         : 0));
				//| aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace

				if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
				{
					throw std::runtime_error(std::string("ERROR::ASSIMP:: ") + importer.GetErrorString());
				}

				printAiScene(ai_scene);
				printAnimations(ai_scene);

				// retrieve the directory path of the filepath
				const std::string directory = path.substr(0, path.find_last_of('\\'));
				const std::string model_name = path.substr(path.find_last_of('\\') + 1,
				                                           path.find_last_of('.') - path.find_last_of('\\') - 1);

				with_bones_ = with_bones;
				with_anim_ = with_anim;

				// create root
				Enttity root_ent = scene.addNewEntityToRoot(model_name);
				importNodeTransform(root_ent, ai_scene->mRootNode);

				if (with_anim_)
					populateAnimNodes(ai_scene);

				populateTree(scene, ai_scene->mRootNode, root_ent);

				// TODO check if there is no empty anim node 

				if (with_anim_)
					importAnimations(root_ent, ai_scene);

				// process ASSIMP's root node recursively
				processNodeData(scene, ai_scene->mRootNode, ai_scene, directory, model_name,
				                root_ent);

				Assimp::DefaultLogger::kill();
				ainode_to_enttity.clear();
				anim_node_name_to_enttity.clear();
			}

		private:
			static inline std::unordered_map<aiNode*, Enttity> ainode_to_enttity;
			static inline std::unordered_map<std::string, Enttity> anim_node_name_to_enttity;
			static inline bool with_bones_ = false;
			static inline bool with_anim_ = false;

			static void importAnimations(Enttity root_ent, const aiScene* ai_scene)
			{
				std::vector<AnimationClip> animation_clips(ai_scene->mNumAnimations);

				for (unsigned i = 0; i < ai_scene->mNumAnimations; ++i)
				{
					aiAnimation* anim = ai_scene->mAnimations[i];
					AnimationClip& clip = animation_clips[i];

					// TODO make it parametarazible or deduced from file 
					double sample_period = 1. / 30.;

					clip.name = anim->mName.C_Str();
					clip.channels.reserve(anim->mNumChannels);

					for (unsigned j = 0; j < anim->mNumChannels; ++j)
					{
						aiNodeAnim* node_anim = anim->mChannels[j];

						Enttity node_entt = anim_node_name_to_enttity[node_anim->mNodeName.C_Str()];

						AnimationChannels channels;

						channels.enttity = node_entt;

						for (unsigned k = 0; k < node_anim->mNumPositionKeys; ++k)
						{
							aiVectorKey pos_k = node_anim->mPositionKeys[k];
							channels.positions.insert({
								static_cast<uint32_t>(std::round(pos_k.mTime / anim->mTicksPerSecond / sample_period)),
								toGlm(pos_k.mValue)
							});
						}

						for (unsigned k = 0; k < node_anim->mNumRotationKeys; ++k)
						{
							aiQuatKey rot_k = node_anim->mRotationKeys[k];
							channels.rotations.insert({
								static_cast<uint32_t>(std::round(rot_k.mTime / anim->mTicksPerSecond / sample_period)),
								toGlm(rot_k.mValue)
							});
						}

						for (unsigned k = 0; k < node_anim->mNumScalingKeys; ++k)
						{
							aiVectorKey scale_k = node_anim->mScalingKeys[k];
							channels.scales.insert({
								static_cast<uint32_t>(
									std::round(scale_k.mTime / anim->mTicksPerSecond / sample_period)),
								toGlm(scale_k.mValue)
							});
						}

						uint32_t min = std::min(channels.positions.begin()->first,std::min(channels.rotations.begin()->first,channels.scales.begin()->first));

						uint32_t max = std::max((--channels.positions.end())->first,
						                             std::max((--channels.rotations.end())->first,
						                                      (--channels.scales.end())->first));

						clip.duration_ = max-min;

						clip.channels.push_back(std::move(channels));
					}
				}

				root_ent.addComponent<AnimationComponent>(std::move(animation_clips));
			}

			static void populateAnimNodes(const aiScene* ai_scene)
			{
				for (unsigned i = 0; i < ai_scene->mNumAnimations; ++i)
				{
					for (unsigned i = 0; i < ai_scene->mNumAnimations; ++i)
					{
						aiAnimation* anim = ai_scene->mAnimations[i];

						for (unsigned j = 0; j < anim->mNumChannels; ++j)
						{
							aiNodeAnim* node_anim = anim->mChannels[j];
							if (anim_node_name_to_enttity.find(node_anim->mNodeName.C_Str()) ==
								anim_node_name_to_enttity.end())
								anim_node_name_to_enttity.insert({node_anim->mNodeName.C_Str(), Enttity{}});
						}
					}
				}
			}

			static void populateTree(Scene& scene, aiNode* ai_node, Enttity curent)
			{
				ainode_to_enttity[ai_node] = curent;

				for (unsigned int i = 0; i < ai_node->mNumChildren; i++)
				{
					aiNode* ai_child = ai_node->mChildren[i];
					std::string child_name = ai_child->mName.C_Str();
					Enttity child = scene.addNewEntityAsChild(curent, child_name);
					importNodeTransform(child, ai_child);

					if (with_anim_)
					{
						if (anim_node_name_to_enttity.find(child_name) != anim_node_name_to_enttity.end())
							anim_node_name_to_enttity[child_name] = child;
					}

					populateTree(scene, ai_child, child);
				}
			}

			static void processNodeData(Scene& scene, aiNode* ai_node, const aiScene* ai_scene,
			                            const std::string& directory,
			                            const std::string& parentName, Enttity parent)
			{
				std::string name_this = parentName + "." + ai_node->mName.C_Str();

				if (ai_node->mNumMeshes)
				{
					// process each mesh located at the current node
					for (unsigned int i = 0; i < ai_node->mNumMeshes; i++)
					{
						aiMesh* mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
						std::string mesh_name = name_this + "." + std::string(mesh->mName.C_Str());

						aiMaterial* ai_material = ai_scene->mMaterials[mesh->mMaterialIndex];
						Material* material = DiffusionMaterial::GetMaterialPtr(directory, ai_scene, ai_material);

						if (mesh->HasBones() && with_bones_)
						{
							// import as skeletal mesh
							std::vector<Enttity> bone_entts;
							for (unsigned j = 0; j < mesh->mNumBones; ++j)
							{
								aiBone* bone = mesh->mBones[j];

								Enttity entt = ainode_to_enttity[bone->mNode];
								entt.addComponent<BoneComponent>(j);

								bone_entts.push_back(entt);
							}

							std::string ent_mesh_name = std::string(mesh->mName.C_Str()) + ":Mesh";

							Enttity new_entty = scene.addNewEntityAsChild(parent, ent_mesh_name);

							new_entty.addComponent<SkeletalModelComponent>(
								SkeletalMesh(material, mesh_name, mesh), bone_entts,
								&Renderer::un_lit_textured
							);
						}
						else if (!mesh->HasBones() || !with_bones_)
						{
							// import as static mesh
							std::string ent_mesh_name = std::string(mesh->mName.C_Str()) + ":Mesh";

							Enttity new_entty = scene.addNewEntityAsChild(parent, ent_mesh_name);

							new_entty.addComponent<StaticModelComponent>(Mesh(material, mesh_name, mesh),
							                                             &Renderer::un_lit_textured);
						}
						else
						{
							throw std::runtime_error("Error reading bone info");
						}
					}
				}

				// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
				for (unsigned int i = 0; i < ai_node->mNumChildren; i++)
				{
					processNodeData(scene, ai_node->mChildren[i], ai_scene, directory, name_this,
					                ainode_to_enttity[ai_node->mChildren[i]]);
				}
			}

			static void importNodeTransform(Enttity ent, aiNode* ainode)
			{
				TransformComponent& trans = ent.getComponent<TransformComponent>();

				aiVector3D translation;
				aiVector3D orientation;
				aiVector3D scale;

				ainode->mTransformation.Decompose(scale, orientation, translation);

				trans.position = toGlm(translation);
				trans.setQuat(toGlm(orientation));
				trans.scale = toGlm(scale);
			}
		};

		void dirtyTraverseTree(Enttity ent, uint32_t frame)
		{
			TransformComponent& this_trans = ent.getComponent<TransformComponent>();

			if (this_trans.isEditedForFrame(frame))
			// if is edited treaverse tree up until leaves to update global trans mtx's
			{
				this_trans.edited[frame] = false;
				this_trans.dirty[frame] = false;

				const RelationshipComponent& ent_rc = ent.getComponent<RelationshipComponent>();
				glm::mat4 parent_trans;

				if (ent_rc.parent)
				{
					parent_trans = ent_rc.parent.getComponent<TransformComponent>().globalTransformMatrix;
					//ent_rc.parent.getComponent<TransformComponent>().getMatrix();
				}
				else // this is root
					parent_trans = glm::mat4(1.0f);

				editedTraverseTree(ent, parent_trans, frame);
			}
			else if (this_trans.isDirtyForFrame(frame)) // if is dirty traverse tree while edited not found
			{
				this_trans.dirty[frame] = false;

				const RelationshipComponent& cur_comp = ent.getComponent<RelationshipComponent>();
				Enttity cur_child = cur_comp.first;

				while (cur_child)
				{
					dirtyTraverseTree(cur_child, frame);
					cur_child = cur_child.getComponent<RelationshipComponent>().next;
				}
			}
		}

		void editedTraverseTree(Enttity ent, glm::mat4 parent_trans_mtx, uint32_t frame)
		{
			TransformComponent& ent_tc = ent.getComponent<TransformComponent>();
			glm::mat4 this_matrix = parent_trans_mtx * ent_tc.getMatrix();

			ent_tc.edited[frame] = false;
			ent_tc.dirty[frame] = false;

			ent_tc.globalTransformMatrix = this_matrix;

			if (StaticModelComponent* static_model_component = ent.tryGetComponent<StaticModelComponent>())
			{
				static_model_component->need_GPU_state_update = true;
			}

			if (BoneComponent* bone = ent.tryGetComponent<BoneComponent>())
			{
				bone->need_gpu_state_update = true;
			}

			const RelationshipComponent& ent_rc = ent.getComponent<RelationshipComponent>();
			Enttity cur_child = ent_rc.first;
			while (cur_child)
			{
				editedTraverseTree(cur_child, this_matrix, frame);
				cur_child = cur_child.getComponent<RelationshipComponent>().next;
			}
		}
	};
}
