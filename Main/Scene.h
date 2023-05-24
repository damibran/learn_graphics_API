#pragma once

#include "Enttity.h"
#include "Componenets/RelationshipComponent.h"
#include "Componenets/TagComponent.h"
#include "Componenets/SkeletonComponent.h"
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
	/**
	 * \brief is container and manager of all entities 
	 * need interface implementation for scene tree
	 */
	class Scene
	{
	public:
		Scene():
			scene_root_(registry_, "SceneRoot")
		{
			ModelImporter::ImportModel(*this, "Models\\Char\\Defeated.dae", true, true);
			ModelImporter::ImportModel(*this, "Models\\Char2\\Rumba Dancing.dae", true, true);
			//ModelImporter::ImportModel(*this, "Models\\Remy\\Remy.dae", true, true);

			animation_sequence_.mFrameMin = 0;
			animation_sequence_.mFrameMax = 800;
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

		/**
		 * \brief hierarchically update transforms
		 * \param frame index of current in flight frame
		 */
		void updateGlobalTransforms(uint32_t frame)
		{
			// begin traversing tree starting from scene root
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
				auto view = registry_.view<SkeletonComponent>();

				for (auto entt : view)
				{
					SkeletonComponent& skeleton_component = view.get<SkeletonComponent>(entt);

					// if is visible
					for (Enttity enttity : skeleton_component.bone_enttities)
					{
						auto [bone, trans] = enttity.getComponent<BoneComponent, TransformComponent>();

						if (bone.need_gpu_state_update)
						{
							bone.need_gpu_state_update = false;

							auto ubo_data = reinterpret_cast<PerSkeletonData::UBODynamicData*>(data +
								skeleton_component.in_GPU_mtxs_offset);

							ubo_data->model[bone.bone_ind] = trans.globalTransformMatrix * bone.offset_mat;
						}
					}
				}

				Renderer::per_skeleton_data_.unMap(frame);
			}
		}

		/**
		 * \brief update transforms according to current animation states
		 * \param anim_frame current global animation frame
		 * \param frame index of inflight frame
		 */
		void updateAnimations(float anim_frame, uint32_t frame)
		{
			auto view = registry_.view<AnimationComponent>();

			// iterate all animated entities
			for (auto ent : view)
			{
				// if it has some animations in sequence
				if (!animation_sequence_.entries_[Enttity{registry_, ent}].empty())
				{
					// get clip which start time is grater or equal to global time 
					auto clip_it = animation_sequence_.entries_[Enttity{registry_, ent}].lower_bound(
						anim_frame);

					// if it is past-the-end or (is not the first in sequence and not equal)
					if (clip_it == animation_sequence_.entries_[Enttity{registry_, ent}].end() ||
						clip_it != animation_sequence_.entries_[Enttity{registry_, ent}].begin() && clip_it->first !=
						anim_frame)
						// move back to get less not equal
						--clip_it;

					// here clip_it have less or equal

					// calculate clip local time
					const float local_time = glm::clamp(clip_it->second.min + anim_frame - clip_it->first,
					                                    clip_it->second.min,
					                                    clip_it->second.max);

					// actually updating transform with local time
					clip_it->second.updateTransforms(local_time, frame);
				}
			}
		}

		void importAnimationTo(Enttity ent, const std::string& file_path)
		{
			ModelImporter::ImportAnimationTo(*this, ent, file_path);
		}

		void importModel(const std::string& path, bool with_bones, bool with_anim)
		{
			ModelImporter::ImportModel(*this, path, with_anim, with_bones);
			for (int i = 0; i < Singletons::device.MAX_FRAMES_IN_FLIGHT; ++i)
				scene_root_.markTransformAsEdited(i);
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

		/**
		 * \brief performs importing of model data from file
		 */
		class ModelImporter
		{
		public:
			static void ImportModel(Scene& scene, const std::string& path, bool with_anim = false,
			                        bool with_bones = false)
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
				{
					std::set<AnimationClip> clips = importAnimations(ai_scene);
					root_ent.addComponent<AnimationComponent>(std::move(clips));
				}

				// process ASSIMP's root node recursively
				processNodeData(scene, ai_scene->mRootNode, ai_scene, directory, model_name,
				                root_ent);

				Assimp::DefaultLogger::kill();
				ainode_to_enttity.clear();
				anim_node_name_to_enttity.clear();
			}

			static void ImportAnimationTo(Scene& scene, Enttity root_ent, const std::string& path)
			{
				Assimp::DefaultLogger::create("", Assimp::DefaultLogger::VERBOSE, aiDefaultLogStream_STDOUT);

				Assimp::Importer importer;
				const aiScene* ai_scene = importer.ReadFile(
					path, aiProcess_ValidateDataStructure | aiProcess_GlobalScale);

				if (!ai_scene || !ai_scene->mRootNode)
				{
					throw std::runtime_error(std::string("ERROR::ASSIMP:: ") + importer.GetErrorString());
				}

				collectAnimNodeToEnttity(root_ent, scene);
				// TODO check if there is no empty anim node (that means, that user renamed entities)

				std::set<AnimationClip> clips = importAnimations(ai_scene);
				root_ent.getComponent<AnimationComponent>().insert(std::move(clips));

				Assimp::DefaultLogger::kill();
				anim_node_name_to_enttity.clear();
			}

		private:
			static inline std::unordered_map<const aiNode*, Enttity> ainode_to_enttity;
			static inline std::unordered_map<std::string, Enttity> anim_node_name_to_enttity;
			static inline bool with_bones_ = false;
			static inline bool with_anim_ = false;

			static std::set<AnimationClip> importAnimations(const aiScene* ai_scene)
			{
				std::set<AnimationClip> animation_clips;

				for (unsigned i = 0; i < ai_scene->mNumAnimations; ++i)
				{
					aiAnimation* anim = ai_scene->mAnimations[i];
					AnimationClip clip;

					// TODO make it parametarazible or deduced from file 
					const double sample_period = 1. / 30.;

					clip.name = anim->mName.C_Str();
					clip.channels.reserve(anim->mNumChannels);

					for (unsigned j = 0; j < anim->mNumChannels; ++j)
					{
						const aiNodeAnim* node_anim = anim->mChannels[j];

						const Enttity node_entt = anim_node_name_to_enttity[node_anim->mNodeName.C_Str()];

						AnimationChannels channels;

						for (unsigned k = 0; k < node_anim->mNumPositionKeys; ++k)
						{
							const aiVectorKey pos_k = node_anim->mPositionKeys[k];
							channels.positions.insert({
								static_cast<float>(std::round(pos_k.mTime / anim->mTicksPerSecond / sample_period)),
								toGlm(pos_k.mValue)
							});
						}

						for (unsigned k = 0; k < node_anim->mNumRotationKeys; ++k)
						{
							const aiQuatKey rot_k = node_anim->mRotationKeys[k];
							channels.rotations.insert({
								static_cast<float>(std::round(rot_k.mTime / anim->mTicksPerSecond / sample_period)),
								toGlm(rot_k.mValue)
							});
						}

						for (unsigned k = 0; k < node_anim->mNumScalingKeys; ++k)
						{
							const aiVectorKey scale_k = node_anim->mScalingKeys[k];
							channels.scales.insert({
								static_cast<float>(std::round(scale_k.mTime / anim->mTicksPerSecond / sample_period)),
								toGlm(scale_k.mValue)
							});
						}

						const float min = std::min(channels.positions.begin()->first,
						                           std::min(channels.rotations.begin()->first,
						                                    channels.scales.begin()->first));

						const float max = std::max((--channels.positions.end())->first,
						                           std::max((--channels.rotations.end())->first,
						                                    (--channels.scales.end())->first));

						clip.min = min;
						clip.max = max;

						clip.channels.insert({node_entt, std::move(channels)});
					}
					animation_clips.insert(std::move(clip));
				}
				return animation_clips;
			}

			static void populateAnimNodes(const aiScene* ai_scene)
			{
				for (unsigned i = 0; i < ai_scene->mNumAnimations; ++i)
				{
					for (unsigned j = 0; j < ai_scene->mNumAnimations; ++j)
					{
						const aiAnimation* anim = ai_scene->mAnimations[j];

						for (unsigned k = 0; k < anim->mNumChannels; ++k)
						{
							const aiNodeAnim* node_anim = anim->mChannels[k];
							if (anim_node_name_to_enttity.find(node_anim->mNodeName.C_Str()) ==
								anim_node_name_to_enttity.end())
								anim_node_name_to_enttity.insert({node_anim->mNodeName.C_Str(), Enttity{}});
						}
					}
				}
			}

			static void collectAnimNodeToEnttity(Enttity root_ent, Scene& scene)
			{
				const RelationshipComponent& root_rc = root_ent.getComponent<RelationshipComponent>();

				Enttity cur_child = root_rc.first;
				while (cur_child)
				{
					anim_node_name_to_enttity[cur_child.getComponent<TagComponent>().tag] = cur_child;
					collectAnimNodeToEnttity(cur_child, scene);
					cur_child = cur_child.getComponent<RelationshipComponent>().next;
				}
			}

			static void populateTree(Scene& scene, const aiNode* ai_node, Enttity curent)
			{
				ainode_to_enttity[ai_node] = curent;

				for (unsigned int i = 0; i < ai_node->mNumChildren; i++)
				{
					const aiNode* ai_child = ai_node->mChildren[i];
					std::string child_name = ai_child->mName.C_Str();
					const Enttity child = scene.addNewEntityAsChild(curent, child_name);
					importNodeTransform(child, ai_child);

					if (with_anim_)
					{
						if (anim_node_name_to_enttity.find(child_name) != anim_node_name_to_enttity.end())
							anim_node_name_to_enttity[child_name] = child;
					}

					populateTree(scene, ai_child, child);
				}
			}

			static void processNodeData(Scene& scene, const aiNode* ai_node, const aiScene* ai_scene,
			                            const std::string& directory,
			                            const std::string& parentName, Enttity parent)
			{
				std::string name_this = parentName + "." + ai_node->mName.C_Str();

				if (ai_node->mNumMeshes)
				{
					// process each mesh located at the current node
					for (unsigned int i = 0; i < ai_node->mNumMeshes; i++)
					{
						const aiMesh* mesh = ai_scene->mMeshes[ai_node->mMeshes[i]];
						std::string mesh_name = name_this + "." + std::string(mesh->mName.C_Str());

						const aiMaterial* ai_material = ai_scene->mMaterials[mesh->mMaterialIndex];
						const DiffusionMaterial* material = DiffusionMaterial::GetMaterialPtr(
							directory, ai_scene, ai_material);

						if (mesh->HasBones() && with_bones_)
						{
							// import as skeletal mesh

							const aiNode* arm_node;
							Enttity arm_ent;
							std::unordered_map<uint32_t, uint32_t> local_to_global_bone_ind;

							assert(mesh->mBones[0]);
							arm_node = mesh->mBones[0]->mArmature;
							arm_ent = ainode_to_enttity[arm_node];

							SkeletonComponent* skeleton_comp = arm_ent.tryGetComponent<SkeletonComponent>();

							if (skeleton_comp)
							{
								uint32_t bone_ent_end = skeleton_comp->bone_enttities.size();
								std::vector<Enttity> bone_entts;

								for (unsigned j = 0; j < mesh->mNumBones; ++j)
								{
									aiBone* bone = mesh->mBones[j];

									Enttity entt = ainode_to_enttity[bone->mNode];

									if (auto bone_comp = entt.tryGetComponent<BoneComponent>())
									{
										local_to_global_bone_ind[j] = bone_comp->bone_ind;
									}
									else
									{
										local_to_global_bone_ind[j] = bone_ent_end;
										entt.addComponent<BoneComponent>(toGlm(bone->mOffsetMatrix), bone_ent_end++);
										bone_entts.push_back(entt);
									}
								}

								skeleton_comp->bone_enttities.insert(skeleton_comp->bone_enttities.end(),
								                                     std::make_move_iterator(bone_entts.begin()),
								                                     std::make_move_iterator(bone_entts.end()));
							}
							else
							{
								std::vector<Enttity> bone_entts;
								for (unsigned j = 0; j < mesh->mNumBones; ++j)
								{
									aiBone* bone = mesh->mBones[j];

									Enttity entt = ainode_to_enttity[bone->mNode];
									entt.addComponent<BoneComponent>(toGlm(bone->mOffsetMatrix), j);
									local_to_global_bone_ind[j] = j;

									bone_entts.push_back(entt);
								}

								arm_ent.addComponent<SkeletonComponent>(std::move(bone_entts));
							}

							std::string ent_mesh_name = std::string(mesh->mName.C_Str()) + ":Mesh";

							Enttity new_entty = scene.addNewEntityAsChild(parent, ent_mesh_name);

							new_entty.addComponent<SkeletalModelComponent>(arm_ent,
							                                               SkeletalMesh(
								                                               material, mesh_name, mesh,
								                                               local_to_global_bone_ind),
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

			static void importNodeTransform(Enttity ent, const aiNode* ainode)
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

		/**
		 * \brief traversing all dirty paths in tree to find edited entities
		 * \param ent current entity
		 * \param frame index of current in flight frame
		 */
		void dirtyTraverseTree(Enttity ent, uint32_t frame)
		{
			// transform component of this entity
			TransformComponent& this_trans = ent.getComponent<TransformComponent>();

			if (this_trans.isEditedForFrame(frame))
			// if is edited traverse tree up until leaves to update global trans mtx's
			{
				// unedit and clear
				this_trans.edited[frame] = false;
				this_trans.dirty[frame] = false;

				// get relationship component of entity
				const RelationshipComponent& ent_rc = ent.getComponent<RelationshipComponent>();
				glm::mat4 parent_trans = glm::mat4(1.0f);

				// if this is not a root, parent could be null only for scene root
				if (ent_rc.parent)
				{
					parent_trans = ent_rc.parent.getComponent<TransformComponent>().globalTransformMatrix;
					//ent_rc.parent.getComponent<TransformComponent>().getMatrix();
				}

				// traverse tree up until leaves to update global trans mtx's
				editedTraverseTree(ent, parent_trans, frame);
			}
			else if (this_trans.isDirtyForFrame(frame))
			// if is dirty traverse tree while edited not found
			{
				// clear transform of this
				this_trans.dirty[frame] = false;

				// get relationship component of this entity
				const RelationshipComponent& cur_comp = ent.getComponent<RelationshipComponent>();
				// get first child
				Enttity cur_child = cur_comp.first;

				// recursively call dirtyTraverseTree for all children
				while (cur_child)
				{
					dirtyTraverseTree(cur_child, frame);
					cur_child = cur_child.getComponent<RelationshipComponent>().next;
				}
			}
		}

		/**
		 * \brief traverse tree accumulating transformation matrix of each entity
		 * \param ent current entity
		 * \param parent_trans_mtx parent transformation matrix
		 * \param frame index of current in flight frame
		 */
		void editedTraverseTree(Enttity ent, glm::mat4 parent_trans_mtx, uint32_t frame)
		{
			// transformation of this node is mul of parent and this
			TransformComponent& ent_tc = ent.getComponent<TransformComponent>();
			const glm::mat4 this_matrix = parent_trans_mtx * ent_tc.getMatrix();

			// unedit and clear
			ent_tc.edited[frame] = false;
			ent_tc.dirty[frame] = false;

			// memorize new global transformation matrix
			ent_tc.globalTransformMatrix = this_matrix;

			// if this node have model its model matrix GPU state should be updated too
			if (StaticModelComponent* static_model_component = ent.tryGetComponent<StaticModelComponent>())
			{
				static_model_component->need_GPU_state_update = true;
			}

			// if this node is bone its transform matrix GPU state should be updated too
			if (BoneComponent* bone = ent.tryGetComponent<BoneComponent>())
			{
				bone->need_gpu_state_update = true;
			}

			// further traverse tree up util the leaves
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
