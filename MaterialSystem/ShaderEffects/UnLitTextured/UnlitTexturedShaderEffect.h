#pragma once

#include "MaterialSystem/ShaderEffects/ShaderEffect.h"
#include"MaterialSystem/ShaderEffects/UnLitTextured/UnLitTexturedGraphicsPipelineStatics.h"
#include "Wrappers/Singletons/PerSkeletonData.h"

namespace dmbrn
{
	/**
	 * \brief describes shader effect for drawing object as unlited textured
	 */
	struct UnlitTexturedShaderEffect : public ShaderEffect
	{
		//UnlitTexturedShaderEffect()=default;

		UnlitTexturedShaderEffect(PerStaticModelData& per_object_data_buffer,
		                          PerSkeletonData& per_skeleton_data):
			per_object_data_buffer_(per_object_data_buffer),
			per_skeleton_data_(per_skeleton_data)
		{
		}

		~UnlitTexturedShaderEffect() override = default;

		UnlitTexturedShaderEffect(UnlitTexturedShaderEffect&& other) = default;
		UnlitTexturedShaderEffect& operator=(UnlitTexturedShaderEffect&& other) = default;

		UnlitTexturedShaderEffect(const UnlitTexturedShaderEffect& other) = delete;
		UnlitTexturedShaderEffect& operator=(const UnlitTexturedShaderEffect& other) = delete;

		/**
		 * \brief record commands to draw all objects in render queues
		 * \param frame current frame index to bind proper descriptor sets
		 * \param command_buffer command buffer record commands to
		 */
		void draw(int frame, const vk::raii::CommandBuffer& command_buffer) override
		{
			// draw all in static queue
			drawStatic(frame, command_buffer, per_object_data_buffer_);
			// draw all in skeletal queue
			drawSkeletal(frame, command_buffer, per_skeleton_data_);
		}

	private:
		PerStaticModelData& per_object_data_buffer_;
		PerSkeletonData& per_skeleton_data_;

		/**
		 * \brief draw all in static queue
		 * \param frame current frame index to bind proper descriptor sets
		 * \param command_buffer command buffer record commands to
		 * \param per_renderable_data_buffer ref to Per Static Model Data to bind
		 */
		void drawStatic(int frame, const vk::raii::CommandBuffer& command_buffer,
		                const PerStaticModelData& per_renderable_data_buffer)
		{
			// ptr of previously binded mesh and material
			const Mesh::MeshRenderData* prev_mesh = nullptr;
			const DiffusionMaterial* prev_mat = nullptr;

			// bind shader effect data
			un_lit_graphics_pipeline_statics_.bindStaticPipeline(command_buffer);
			un_lit_graphics_pipeline_statics_.bindStaticShaderData(frame, command_buffer);

			// linearly iterate all objects in queue
			for (auto& [mesh, offset] : static_render_queue)
			{
				// if mesh doesn't change no need to rebind it 
				if (mesh->render_data_ != prev_mesh)
				{
					mesh->bind(command_buffer);
					prev_mesh = mesh->render_data_;
				}

				// if material doesn't change no need to rebind it 
				if (mesh->material_ != prev_mat)
				{
					mesh->material_->bindMaterialData(frame, command_buffer,
					                                  *un_lit_graphics_pipeline_statics_.static_pipeline_layout_);
					prev_mat = mesh->material_;
				}

				// bind per object data with given offset
				per_renderable_data_buffer.bindDataFor(frame, command_buffer,
				                                       *un_lit_graphics_pipeline_statics_.static_pipeline_layout_,
				                                       offset);

				// issue draw command
				mesh->drawIndexed(command_buffer);
			}

			// clear queue for next frame
			static_render_queue.clear();
		}

		void drawSkeletal(int frame, const vk::raii::CommandBuffer& command_buffer,
		                  const PerSkeletonData& per_skeletal_data_buffer)
		{
			const SkeletalMesh::SkeletalMeshRenderData* prev_mesh = nullptr;
			const DiffusionMaterial* prev_mat = nullptr;
			un_lit_graphics_pipeline_statics_.bindSkeletalPipeline(command_buffer);
			un_lit_graphics_pipeline_statics_.bindSkeletalShaderData(frame, command_buffer);

			for (auto& [mesh, offset] : skeletal_render_queue)
			{
				if (mesh->render_data_ != prev_mesh)
				{
					mesh->bind(command_buffer);
					prev_mesh = mesh->render_data_;
				}

				if (mesh->material_ != prev_mat)
				{
					mesh->material_->bindMaterialData(frame, command_buffer,
					                                  *un_lit_graphics_pipeline_statics_.static_pipeline_layout_);
					prev_mat = mesh->material_;
				}

				per_skeletal_data_buffer.bindDataFor(frame, command_buffer,
				                                     *un_lit_graphics_pipeline_statics_.skeletal_pipeline_layout_,
				                                     offset);

				mesh->drawIndexed(command_buffer);
			}

			skeletal_render_queue.clear();
		}

	public:
		static void setRenderPass(const vk::raii::RenderPass& render_pass)
		{
			un_lit_graphics_pipeline_statics_.setRenderPass(render_pass);
		}

		static inline UnLitTexturedGraphicsPipelineStatics un_lit_graphics_pipeline_statics_{};
	};
}
