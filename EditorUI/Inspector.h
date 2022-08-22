#pragma once

#include "SceneTree.h"

namespace dmbrn
{
	class Inspector
	{
	public:
		Inspector(SceneTree& scene_tree):
		scene_tree_(scene_tree){}

		void newImGuiFrame()
		{
			ImGui::Begin("Inspector");

			if(scene_tree_.getSelected())
			{
				Enttity& entity = scene_tree_.getSelected();

				if(auto* comp = entity.tryGetComponent<TagComponent>())
				{
					char buf[256];
					memset(buf,0,sizeof(buf));
					strcpy_s(buf,sizeof(buf),comp->tag.c_str());

					if(ImGui::InputText("Tag",buf,sizeof(buf)))
					{
						comp->tag=std::string(buf);
					}
				}
			}

			ImGui::End();
		}

	private:
		SceneTree& scene_tree_;
	};
}
