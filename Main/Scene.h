#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"
#include "Wrappers/Singletons/Renderer.h"
#include "Wrappers/ModelImporter.h"

namespace dmbrn
{
	// need interface implementation for scene tree
	class Scene
	{
		friend class SceneTree;
	public:
		Scene():
			scene_root_(registry_, "SceneRoot")
		{
			ModelImporter::Import(*this, false, "Models\\SkinTest\\RiggedSimple.dae");

			//ModelImporter::Import(*this, true, "Models\\Char\\TwoChar.fbx");

			//addModel("Models\\Char\\Warrok W Kurniawan.fbx"); //Models\Char\Warrok W Kurniawan.fbx
			//addModel("Models\\Char\\TwoChar.fbx"); //Models\Char\Warrok W Kurniawan.fbx
			//addModel("Models\\SkinTest\\RiggedSimple.dae");
			//addModel("F:\\3D_Scenes\\Sponza\\NewSponza_Main_glTF_002.gltf");
			//addModel("Models\\DoubleTestCube\\DoubleTestCube.fbx");
		}

		size_t getCountOfEntities()
		{
			return registry_.size();
		}

		Enttity addNewEntityToRoot(const std::string& name = std::string{})
		{
			return Enttity{registry_, name, scene_root_};
		}

		Enttity addNewEntityAsChild(Enttity& parent, const std::string& name = std::string{})
		{
			return Enttity{registry_, name, parent};
		}

		void deleteEntity(Enttity& enttity)
		{
			registry_.destroy(enttity);
		}

		void recursivelyAddTo(SceneNode& node, Enttity& parent)
		{
			Enttity child_ent{registry_, node.name, parent};

			if (node.mesh.render_data_)
				child_ent.addComponent<ModelComponent>(std::move(node.mesh), &Renderer::un_lit_textured);

			TransformComponent& t = child_ent.getComponent<TransformComponent>();
			t.position = node.transform.position;
			t.rotation = node.transform.rotation;
			t.scale = node.transform.scale;

			for (auto& child : node.children)
			{
				recursivelyAddTo(child, child_ent);
			}
		}

		// for now updates data for all entities
		void updatePerObjectData(uint32_t frame)
		{
			auto group = registry_.group<ModelComponent>(entt::get<TransformComponent>);

			char* data = ModelComponent::per_object_data_buffer_.map(frame);

			recursivelyUpdateMatrix(scene_root_, glm::mat4{1.0f}, data);

			ModelComponent::per_object_data_buffer_.unMap(frame);
		}

		void recursivelyUpdateMatrix(const Enttity& ent, const glm::mat4& parent_matrix, char* data_map)
		{
			const TransformComponent& this_trans = ent.getComponent<TransformComponent>();
			glm::mat4 this_matrix = parent_matrix * this_trans.getMatrix();

			if (const ModelComponent* model = ent.tryGetComponent<ModelComponent>())
			{
				auto ubo_data = reinterpret_cast<PerObjectDataBuffer::UBODynamicData*>(data_map + model->
					inGPU_transform_offset);
				ubo_data->model = this_matrix;
			}

			auto& cur_comp = ent.getComponent<RelationshipComponent>();
			auto cur_id = cur_comp.first;

			while (cur_id != entt::null)
			{
				recursivelyUpdateMatrix(Enttity{registry_, cur_id}, this_matrix, data_map);

				cur_id = registry_.get<RelationshipComponent>(cur_id).next;
			}
		}

		// may perform culling
		auto getModelsToDraw()
		{
			return registry_.group<ModelComponent>(entt::get<TransformComponent>);
		}

	private:
		entt::registry registry_;
		Enttity scene_root_;

		class ModelImporter
		{
		public:
			static void Import(Scene& scene, bool with_bones, const std::string& path)
			{
				Assimp::Importer importer;
				const aiScene* ai_scene = importer.ReadFile(
					path, aiProcess_Triangulate | aiProcess_FlipUVs | with_bones ? aiProcess_PopulateArmatureData : 0);
				//| aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace

				if (!ai_scene || ai_scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !ai_scene->mRootNode)
				{
					throw std::runtime_error(std::string("ERROR::ASSIMP:: ") + importer.GetErrorString());
				}

				// retrieve the directory path of the filepath
				const std::string directory = path.substr(0, path.find_last_of('\\'));
				const std::string model_name = path.substr(path.find_last_of('\\') + 1,
				                                           path.find_last_of('.') - path.find_last_of('\\') - 1);
				const std::string extension = path.substr(path.find_last_of('.') + 1,
				                                          path.size() - path.find_last_of('.'));

				with_bones_ = with_bones;

				if (extension == "fbx")
					scale_factor_ = 1. / 100.;


				// process ASSIMP's root node recursively
				processNode(scene, ai_scene->mRootNode, ai_scene, directory, model_name,
				            scene.addNewEntityToRoot(model_name));
			}

		private:
			static inline std::unordered_map<std::string, std::unordered_map<std::string, uint32_t>>
			armature_to_bones_to_id_;
			static inline bool with_bones_ = false;
			static inline float scale_factor_ = 1.;

			static void processNode(Scene& scene, aiNode* ai_node, const aiScene* ai_scene,
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
						std::string mesh_name = name_this + + "." + std::string(mesh->mName.C_Str());

						aiMaterial* ai_material = ai_scene->mMaterials[mesh->mMaterialIndex];
						Material* material = DiffusionMaterial::GetMaterialPtr(directory, ai_scene, ai_material);

						// impotrt as skeletal mesh
						if (mesh->HasBones() && with_bones_)
						{
							aiBone* root_bone = mesh->mBones[0];
							std::string armature_name = root_bone->mArmature->mName.C_Str();

							// assume invariant that armatures have different names
							if (auto it = armature_to_bones_to_id_.find(armature_name); it ==
								armature_to_bones_to_id_.end())
							{
								armature_to_bones_to_id_[armature_name] = {};
								armature_to_bones_to_id_[armature_name][root_bone->mName.C_Str()]
									= 0;

								std::vector<glm::mat4> bones_offset_mtxs;
								std::unordered_map<std::string, uint32_t>& bone_name_to_id = armature_to_bones_to_id_[
									armature_name];

								for (int j = 1; j < mesh->mNumBones; ++j)
								{
									aiBone* bone = mesh->mBones[j];

									uint32_t cur_id = bone_name_to_id.size();

									bone_name_to_id[bone->mName.C_Str()] = cur_id;

									bones_offset_mtxs.push_back(toGlm(bone->mOffsetMatrix));
								}

								Enttity new_entty = scene.addNewEntityAsChild(parent, mesh->mName.C_Str());
								//new_entty.addComponent<SkeletalMeshComp>();
							}
							else
							{
								throw std::runtime_error("Armatures must have different names");
							}
						}
						else if (!mesh->HasBones() || !with_bones_)
						{
							aiVector3D translation;
							aiVector3D orientation;
							aiVector3D scale;

							ai_node->mTransformation.Decompose(scale, orientation, translation);

							Enttity new_entty = scene.addNewEntityAsChild(parent,mesh->mName.C_Str());

							TransformComponent& trans = new_entty.getComponent<TransformComponent>();

							trans.position = toGlm(translation);
							trans.rotation = toGlm(orientation);
							trans.scale = toGlm(scale);

							Material* material = DiffusionMaterial::GetMaterialPtr(directory, ai_scene, ai_material);

							new_entty.addComponent<ModelComponent>(Mesh(material,mesh_name,mesh),&Renderer::un_lit_textured);
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
					Enttity child = scene.addNewEntityAsChild(parent, ai_node->mChildren[i]->mName.C_Str());
					TransformComponent& trans = child.getComponent<TransformComponent>();

					aiVector3D translation;
					aiVector3D orientation;
					aiVector3D scale;

					ai_node->mTransformation.Decompose(scale, orientation, translation);

					trans.position = toGlm(translation);
					trans.rotation = toGlm(orientation);
					trans.scale = toGlm(scale);

					processNode(scene, ai_node->mChildren[i], ai_scene, directory, name_this, child);
				}
			}
		};
	};
}
