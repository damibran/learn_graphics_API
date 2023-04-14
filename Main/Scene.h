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


namespace dmbrn
{
	// need interface implementation for scene tree
	class Scene
	{
	public:
		Scene():
			scene_root_(registry_, "SceneRoot")
		{
			ModelImporter::Import(*this, true, "Models\\SkinTest\\RiggedSimple.gltf");
			//ModelImporter::Import(*this,false,"Models\\DoubleTestCube\\DoubleTestCube.fbx");

			//ModelImporter::Import(*this,true,"Models\\anim_test.fbx");

			//ModelImporter::Import(*this, true, "Models\\Dragon\\2dragon.gltf");

			//ModelImporter::Import(*this, true, "Models\\Char\\TwoChar@Taunt.gltf");

			//ModelImporter::Import(*this, true,"Models\\Char\\Warrok W Kurniawan.fbx");

			RelationshipComponent root_rc = scene_root_.getComponent<RelationshipComponent>();

			printSceneRecursively(scene_root_, "");


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

	private:
		entt::registry registry_;
		Enttity scene_root_;

		class ModelImporter
		{
		public:
			static void Import(Scene& scene, bool with_bones, const std::string& path)
			{
				Assimp::DefaultLogger::create("", Assimp::DefaultLogger::VERBOSE, aiDefaultLogStream_STDOUT);

				Assimp::Importer importer;
				const aiScene* ai_scene = importer.ReadFile(
					path, aiProcess_Triangulate | aiProcess_ValidateDataStructure | aiProcess_FlipUVs | (with_bones
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
				const std::string extension = path.substr(path.find_last_of('.') + 1,
				                                          path.size() - path.find_last_of('.'));

				with_bones_ = with_bones;

				scale_factor_ = 1.0f;
				if (extension == "fbx")
					scale_factor_ = 0.01;

				// create root
				Enttity root_ent = scene.addNewEntityToRoot(model_name);
				TransformComponent& trans = root_ent.getComponent<TransformComponent>();

				aiVector3D translation;
				aiVector3D orientation;
				aiVector3D scale;

				ai_scene->mRootNode->mTransformation.Decompose(scale, orientation, translation);

				trans.position = toGlm(translation);
				trans.rotation = toGlm(orientation);
				trans.scale = toGlm(scale);
				// create root

				populateTree(scene, ai_scene->mRootNode, root_ent);

				// process ASSIMP's root node recursively
				processNodeData(scene, ai_scene->mRootNode, ai_scene, directory, model_name,
				                root_ent);

				Assimp::DefaultLogger::kill();
			}

		private:
			static inline std::unordered_map<aiNode*, Enttity> ainode_to_enttity;
			static inline bool with_bones_ = false;
			static inline float scale_factor_ = 1.;

			static void populateTree(Scene& scene, aiNode* ai_node, Enttity curent)
			{
				ainode_to_enttity[ai_node] = curent;

				for (unsigned int i = 0; i < ai_node->mNumChildren; i++)
				{
					Enttity child = scene.addNewEntityAsChild(curent, ai_node->mChildren[i]->mName.C_Str());
					TransformComponent& trans = child.getComponent<TransformComponent>();

					aiVector3D translation;
					aiVector3D orientation;
					aiVector3D scale;

					ai_node->mTransformation.Decompose(scale, orientation, translation);

					trans.position = toGlm(translation);
					trans.rotation = toGlm(orientation);
					trans.scale = toGlm(scale);

					populateTree(scene, ai_node->mChildren[i], child);
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
							for (int j = 0; j < mesh->mNumBones; ++j)
							{
								aiBone* bone = mesh->mBones[j];

								Enttity entt = ainode_to_enttity[bone->mNode];
								entt.addComponent<BoneComponent>(j);

								bone_entts.push_back(entt);
							}

							aiVector3D translation;
							aiVector3D orientation;
							aiVector3D scale;

							ai_node->mTransformation.Decompose(scale, orientation, translation);

							std::string ent_mesh_name = std::string(mesh->mName.C_Str())+":Mesh";

							Enttity new_entty = scene.addNewEntityAsChild(parent, ent_mesh_name);

							TransformComponent& trans = new_entty.getComponent<TransformComponent>();

							trans.position = toGlm(translation) * (scale_factor_);
							trans.rotation = toGlm(orientation);
							trans.scale = toGlm(scale) * (scale_factor_);

							new_entty.addComponent<SkeletalModelComponent>(
								SkeletalMesh(material, mesh_name, mesh), bone_entts,
								&Renderer::un_lit_textured
							);
						}
						else if (!mesh->HasBones() || !with_bones_)
						{
							// import as static mesh

							aiVector3D translation;
							aiVector3D orientation;
							aiVector3D scale;

							ai_node->mTransformation.Decompose(scale, orientation, translation);

							std::string ent_mesh_name = std::string(mesh->mName.C_Str())+":Mesh";

							Enttity new_entty = scene.addNewEntityAsChild(parent, ent_mesh_name);

							TransformComponent& trans = new_entty.getComponent<TransformComponent>();

							trans.position = toGlm(translation) * (scale_factor_);
							trans.rotation = toGlm(orientation);
							trans.scale = toGlm(scale) * (scale_factor_);

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
