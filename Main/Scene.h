#pragma once

#include <entt/entt.hpp>
#include "Enttity.h"
#include "Wrappers/Singletons/Renderer.h"
#include "Wrappers/ModelImporter.h"
#include "Wrappers/SkeletalMesh.h"

namespace dmbrn
{
	// need interface implementation for scene tree
	class Scene
	{
	public:
		Scene():
			scene_root_(registry_, "SceneRoot")
		{
			ModelImporter::Import(*this, false, "Models\\SkinTest\\RiggedSimple.dae");
			//ModelImporter::Import(*this,false,"Models\\DoubleTestCube\\DoubleTestCube.fbx");

			//ModelImporter::Import(*this, false, "Models\\Char\\TwoChar.fbx");

			RelationshipComponent root_rc = scene_root_.getComponent<RelationshipComponent>();

			printSceneRecursively(scene_root_,"");

			//ModelImporter::Import(*this, false,"Models\\Char\\Warrok W Kurniawan.fbx");

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

			std::cout<<tab+tag.tag<<std::endl;

			Enttity cur_child = node_rc.first;

			while(cur_child)
			{
				printSceneRecursively(cur_child, tab+" ");
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

		void recursivelyAddTo(SceneNode& node, Enttity parent)
		{
			Enttity child_ent{registry_, node.name, parent};

			if (node.mesh.render_data_)
				child_ent.addModelComponent(std::move(node.mesh), &Renderer::un_lit_textured);

			TransformComponent& t = child_ent.getComponent<TransformComponent>();
			t.position = node.transform.position;
			t.rotation = node.transform.rotation;
			t.scale = node.transform.scale;

			for (auto& child : node.children)
			{
				recursivelyAddTo(child, child_ent);
			}
		}

		void updateGlobalTransforms(uint32_t frame)
		{
			dirtyTraverseTree(scene_root_, frame);
		}

		// for now updates data for all entities
		void updatePerObjectData(uint32_t frame)
		{
			char* data = RenderableComponent::per_object_data_buffer_.map(frame);

			// TODO: iterate all renderable, update mtxs
			auto group = registry_.group<RenderableComponent>(entt::get<TransformComponent>);

			for (auto entity : group)
			{
				auto [model, transform] = group.get(entity);

				if (model.need_GPU_state_update)
				{
					model.need_GPU_state_update = false;
					auto ubo_data = reinterpret_cast<PerRenderableData::UBODynamicData*>(data + model.
						inGPU_transform_offset);
					ubo_data->model = transform.globalTransformMatrix;
				}
			}

			RenderableComponent::per_object_data_buffer_.unMap(frame);
		}

		// may perform culling
		auto getModelsToDraw()
		{
			return registry_.group<ModelComponent>(entt::get<RenderableComponent>);
		}

		// may perform culling
		auto getSkeletalModelsToDraw()
		{
			return registry_.group<SkeletalModelComponent>(entt::get<RenderableComponent>);
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
				Assimp::Importer importer;
				const aiScene* ai_scene = importer.ReadFile(
					path, aiProcess_Triangulate | aiProcess_FlipUVs | (with_bones
						                                                   ? aiProcess_PopulateArmatureData
						                                                   : 0));
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


				scale_factor_ = 1.0f;
				if (extension == "fbx")
					scale_factor_ = 0.01;

				// process ASSIMP's root node recursively
				processNode(scene, ai_scene->mRootNode, ai_scene, directory, model_name,
				            scene.addNewEntityToRoot(model_name));

				if(with_bones)
					armature_to_bones_to_id_.clear();
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

							Enttity new_entty = scene.addNewEntityAsChild(parent, mesh->mName.C_Str());

							TransformComponent& trans = new_entty.getComponent<TransformComponent>();

							trans.position = toGlm(translation) * (scale_factor_);
							trans.rotation = toGlm(orientation);
							trans.scale = toGlm(scale) * (scale_factor_);

							Material* material = DiffusionMaterial::GetMaterialPtr(directory, ai_scene, ai_material);

							new_entty.addModelComponent(Mesh(material, mesh_name, mesh),
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
					parent_trans = ent_rc.parent.getComponent<TransformComponent>().globalTransformMatrix;//ent_rc.parent.getComponent<TransformComponent>().getMatrix();
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

			if (RenderableComponent* renderable = ent.tryGetComponent<RenderableComponent>())
			{
				renderable->need_GPU_state_update = true;
			}

			const RelationshipComponent& ent_rc = ent.getComponent<RelationshipComponent>();
			Enttity cur_child = ent_rc.first;
			while (cur_child )
			{
				editedTraverseTree(cur_child, this_matrix, frame);
				cur_child = cur_child.getComponent<RelationshipComponent>().next;
			}
		}
	};
}
