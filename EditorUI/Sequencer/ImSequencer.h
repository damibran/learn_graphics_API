// https://github.com/CedricGuillemet/ImGuizmo
// v 1.89 WIP
//
// The MIT License(MIT)
//
// Copyright(c) 2021 Cedric Guillemet
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
#pragma once

#include <cstddef>
#include <cmath>
#include "Main/Enttity.h"
#include "Main/AnimationSequence.h"

struct ImDrawList;
struct ImRect;

#ifndef IMGUI_DEFINE_MATH_OPERATORS
	static ImVec2 operator+(const ImVec2& a, const ImVec2& b)
	{
		return ImVec2(a.x + b.x, a.y + b.y);
	}
#endif

namespace dmbrn
{
	class Sequencer
	{
	public:
		const float cursorWidth = 8.f;
		const float MinBarWidth = 80.f;
		const float ItemHeight = 20.;
		const float legendWidth = 200.f;
		const float text_offset = 2.f;

		Sequencer(AnimationSequence& sequence):
			sequence(sequence),
			selectedEntity(sequence.end())
		{
		}

		float getCurrentFrame()
		{
			return currentFrame;
		}

		bool playing = false;

		AnimationSequence& sequence;
		AnimationSequence::EntityIterator selectedEntity;
		float currentFrame = 100.f;
		bool expanded = true;
		int firstFrame = 0;

		ImVec2 frameBarPixelOffsets = {0, 150};
		ImVec2 frameBarPixelOffsetsTarget = {0, 150};

		std::pair<AnimationSequence::EntityIterator, std::optional<AnimationSequence::ClipIterator>>
		movingEntry = {sequence.end(), std::nullopt};

		float movingPos = -1.f;

		bool MovingScrollBar = false;
		bool MovingCurrentFrame = false;

		bool panningView = false;
		ImVec2 panningViewSource;
		int panningViewFrame = 0;

		bool sizingRBar = false;
		bool sizingLBar = false;

		AnimationSequence::EntityIterator expanded_ent = sequence.end();
		std::unordered_set<Enttity, Enttity::hash> expanded_transform_ents;

		enum SEQUENCER_OPTIONS
		{
			SEQUENCER_EDIT_NONE = 0,
			SEQUENCER_EDIT_STARTEND = 1 << 1,
			SEQUENCER_CHANGE_FRAME = 1 << 3,
			SEQUENCER_ADD = 1 << 4,
			SEQUENCER_DEL = 1 << 5,
			SEQUENCER_COPYPASTE = 1 << 6,
			SEQUENCER_EDIT_ALL = SEQUENCER_EDIT_STARTEND | SEQUENCER_CHANGE_FRAME
		};

		void drawControls()
		{
			ImGui::PushItemWidth(130);

			ImGui::InputInt("Frame Min", &sequence.mFrameMin);
			ImGui::SameLine();

			ImGui::InputInt("Frame Max", &sequence.mFrameMax);
			ImGui::SameLine();

			int t_current_frame = static_cast<int>(currentFrame);
			if (ImGui::InputInt("Frame ", &t_current_frame))
				currentFrame = static_cast<float>(t_current_frame);
			ImGui::SameLine();


			if (!playing)
				playing = ImGui::Button("Play");
			if (playing)
			{
				ImGui::SameLine();
				playing = !ImGui::Button("Stop");
			}

			ImGui::PopItemWidth();
		}

		bool draw(float d_time, int sequenceOptions)
		{
			drawControls();

			bool ret = false;
			const ImGuiIO& io = ImGui::GetIO();
			const float mouse_x = io.MousePos.x;
			const float mouse_y = io.MousePos.y;

			const bool popupOpened = false;
			const int sequenceCount = sequence.getAnimationComponentCount();
			if (!sequenceCount)
				return false;
			ImGui::BeginGroup();

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 canvas_pos = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
			ImVec2 canvas_size = ImGui::GetContentRegionAvail(); // Resize canvas to what's available

			float controlHeight = sequenceCount * ItemHeight;
			uint32_t expanded_ent_child_count = 0;
			// TODO add expanded height
			//for (auto ent_it = sequence.begin(); ent_it != sequence.end(); ++ent_it)
			//	controlHeight += int(sequence.GetCustomHeight(i));
			if (expanded_ent != sequence.end())
			{
				expanded_ent_child_count = expanded_ent->first.getCountOfAllChildEnts();
				controlHeight += static_cast<float>(expanded_ent_child_count) * ItemHeight;
				controlHeight += 3*static_cast<float>(expanded_transform_ents.size()) * ItemHeight;
			}
			const int frameCount = ImMax(sequence.mFrameMax - sequence.mFrameMin, 1);

			// zoom in/out
			frameBarPixelOffsets.x = ImLerp(frameBarPixelOffsets.x, frameBarPixelOffsetsTarget.x, 0.33f);
			frameBarPixelOffsets.y = ImLerp(frameBarPixelOffsets.y, frameBarPixelOffsetsTarget.y, 0.33f);

			firstFrame = ImLerp(sequence.mFrameMin, sequence.mFrameMax,
			                    frameBarPixelOffsets.x / (canvas_size.x - legendWidth));
			const int lastFrame = ImLerp(sequence.mFrameMin, sequence.mFrameMax,
			                             frameBarPixelOffsets.y / (canvas_size.x - legendWidth));

			float framePixelWidth = (canvas_size.x - legendWidth) / (lastFrame - firstFrame);

			// --
			if (!expanded)
			{
				ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, ItemHeight));
				draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight),
				                         0xFF3D3837, 0);
				char tmps[512];
				ImFormatString(tmps, IM_ARRAYSIZE(tmps), GetCollapseFmt(), frameCount, sequenceCount);
				draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
			}
			else
			{
				// test scroll area
				const ImVec2 headerSize(canvas_size.x, ItemHeight);
				const ImVec2 scrollBarSize(canvas_size.x, 14.f);

				ImGui::InvisibleButton("topBar", headerSize);
				draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);

				const ImVec2 childFramePos = ImGui::GetCursorScreenPos();
				const ImVec2 childFrameSize(canvas_size.x,
				                            canvas_size.y - 8.f - headerSize.y - scrollBarSize.y);

				ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
				ImGui::BeginChildFrame(889, childFrameSize, ImGuiWindowFlags_AlwaysVerticalScrollbar);
				ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x, controlHeight));

				const ImVec2 contentMin = ImGui::GetItemRectMin();
				const ImVec2 contentMax = ImGui::GetItemRectMax();
				const float contentHeight = contentMax.y - contentMin.y;

				// full background
				draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, 0xFF242424, 0);

				// current frame top
				const ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y),
				                     ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight));

				// current frame  change
				if (!MovingCurrentFrame && !MovingScrollBar && movingEntry.first == sequence.end() &&
					sequenceOptions &
					SEQUENCER_CHANGE_FRAME && topRect.Contains(io.MousePos) && io.MouseDown[0])
				{
					MovingCurrentFrame = true;
				}

				if (MovingCurrentFrame)
				{
					if (frameCount)
					{
						currentFrame = std::floorf((io.MousePos.x - topRect.Min.x) /
							framePixelWidth) + static_cast<float>(firstFrame);
					}
					if (!io.MouseDown[0])
						MovingCurrentFrame = false;
				}

				if (playing)
					currentFrame = currentFrame + 30 * d_time;

				// looping ?
				currentFrame = ImClamp(currentFrame, static_cast<float>(sequence.mFrameMin),
				                       static_cast<float>(sequence.mFrameMax));

				//header
				draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight),
				                         0xFF3D3837, 0);

				//header frame number and lines
				int modFrameCount = 10;
				int frameStep = 1;
				while ((modFrameCount * framePixelWidth) < 150)
				{
					modFrameCount *= 2;
					frameStep *= 2;
				}
				int halfModFrameCount = modFrameCount / 2;

				auto drawLine = [&](int i, float regionHeight)
				{
					const bool baseIndex = ((i % modFrameCount) == 0) || (i == sequence.mFrameMax || i == sequence.
						mFrameMin);
					const bool halfIndex = (i % halfModFrameCount) == 0;
					const float px = canvas_pos.x + i * framePixelWidth + legendWidth -
						static_cast<float>(firstFrame) * framePixelWidth;
					const float tiretStart = baseIndex ? 4.f : (halfIndex ? 10.f : 14.f);
					const float tiretEnd = baseIndex ? regionHeight : ItemHeight;

					if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
					{
						draw_list->AddLine(ImVec2(px, canvas_pos.y + tiretStart),
						                   ImVec2(px, canvas_pos.y + tiretEnd - 1), 0xFF606060, 1);

						draw_list->AddLine(ImVec2(px, canvas_pos.y + ItemHeight),
						                   ImVec2(px, canvas_pos.y + regionHeight - 1), 0x30606060, 1);
					}

					if (baseIndex && px > (canvas_pos.x + legendWidth))
					{
						char tmps[512];
						ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
						draw_list->AddText(ImVec2(px + 3.f, canvas_pos.y), 0xFFBBBBBB, tmps);
					}
				};

				auto drawLineInContentRect = [&](int i, int /*regionHeight*/)
				{
					const float px = canvas_pos.x + i * framePixelWidth + legendWidth -
						static_cast<float>(firstFrame) * framePixelWidth;
					const float tiretStart = contentMin.y;
					const float tiretEnd = contentMax.y;

					if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
					{
						draw_list->AddLine(ImVec2(px, tiretStart), ImVec2(px, tiretEnd),
						                   0x30606060, 1);
					}
				};
				for (int i = sequence.mFrameMin - sequence.mFrameMin % frameStep; i <= sequence.mFrameMax; i +=
				     frameStep)
				{
					drawLine(i, ItemHeight);
				}
				drawLine(sequence.mFrameMin, ItemHeight);
				drawLine(sequence.mFrameMax, ItemHeight);

				// clip content
				draw_list->PushClipRect(childFramePos, childFramePos + childFrameSize, true);

				const ImVec2 legendMax = {childFramePos.x + legendWidth, (childFramePos + childFrameSize).y};
				draw_list->PushClipRect(childFramePos, legendMax);

				// draw item names in the legend rect on the left
				ImVec2 current_min{contentMin.x, contentMin.y};
				const float expanded_ent_indend_size = 15;
				const float expanded_transform_indend_size = expanded_ent_indend_size + 15;
				int i = 0;
				for (auto ent_it = sequence.begin(); ent_it != sequence.end(); ++ent_it, ++i)
				{
					ImVec2 tpos(
						current_min.x + 3,
						current_min.y + 2);

					ImVec2 ent_rect_max = {
						current_min.x + legendWidth,
						current_min.y + ItemHeight
					};

					ImRect ent_rect{current_min, ent_rect_max};

					if (ent_rect.Contains(io.MousePos))
						draw_list->AddRectFilled(current_min, ent_rect_max, 0x5042c4c2);

					if (ent_rect.Contains(io.MousePos) && io.MouseDoubleClicked[0])
					{
						if (expanded_ent != ent_it)
						{
							expanded_transform_ents.clear();
							expanded_ent = ent_it;
						}
						else
						{
							expanded_transform_ents.clear();
							expanded_ent = sequence.end();
						}
					}

					draw_list->AddText(tpos, 0xFFFFFFFF, ent_it->first.getComponent<TagComponent>().tag.c_str());

					current_min.y += ItemHeight;

					// TODO CustomHeight
					if (ent_it == expanded_ent)
					{
						current_min.x += expanded_ent_indend_size;
						int j = 0;
						for (auto child_it = expanded_ent->first.beginChild(); *child_it; ++child_it, ++j)
						{
							tpos = {
								current_min.x + 3,
								current_min.y + 2
							};

							const ImVec2 child_rect_max = {
								current_min.x + legendWidth,
								current_min.y + ItemHeight
							};

							const ImRect child_rect{current_min, child_rect_max};

							if (child_rect.Contains(io.MousePos))
								draw_list->AddRectFilled(current_min, child_rect_max, 0x5042c4c2);

							if (child_rect.Contains(io.MousePos) && io.MouseDoubleClicked[0])
							{
								auto it = expanded_transform_ents.find(*child_it);
								if (it == expanded_transform_ents.end())
									expanded_transform_ents.insert(*child_it);
								else
									expanded_transform_ents.erase(it);
							}

							draw_list->AddText(tpos, 0xFFFFFFFF, child_it->getComponent<TagComponent>().tag.c_str());

							current_min.y += ItemHeight;

							if (expanded_transform_ents.find(*child_it) != expanded_transform_ents.end())
							{
								current_min.x += expanded_transform_indend_size;

								tpos = {
									current_min.x + 3,
									current_min.y + 2
								};
								draw_list->AddText(tpos, 0xFFFFFFFF, "Postition");
								current_min.y += ItemHeight;

								tpos = {
									current_min.x + 3,
									current_min.y + 2
								};
								draw_list->AddText(tpos, 0xFFFFFFFF, "Rotation");
								current_min.y += ItemHeight;

								tpos = {
									current_min.x + 3,
									current_min.y + 2
								};
								draw_list->AddText(tpos, 0xFFFFFFFF, "Scale");
								current_min.y += ItemHeight;

								current_min.x -= expanded_transform_indend_size;
							}
						}

						current_min.x -= expanded_ent_indend_size;
					}
				}

				draw_list->PopClipRect();

				// slots background
				i = 0;
				float customHeight = 0;
				for (auto ent_it = sequence.begin(); ent_it != sequence.end(); ++ent_it, ++i)
				{
					unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

					// TODO CustomHeight only one entity can be expanded
					float localCustomHeight = 0; // sequence.GetCustomHeight(i);
					if (ent_it == expanded_ent)
					{
						localCustomHeight += static_cast<float>(expanded_ent_child_count) * ItemHeight;
						localCustomHeight += 3*static_cast<float>(expanded_transform_ents.size()) * ItemHeight;
					}

					ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
					const ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1 + localCustomHeight);

					draw_list->AddRectFilled(pos, sz, col, 0);

					customHeight += localCustomHeight;
				}

				draw_list->PushClipRect(childFramePos + ImVec2(legendWidth, 0.f), childFramePos + childFrameSize,
				                        true);

				// vertical frame lines in content area
				for (int i = sequence.mFrameMin; i <= sequence.mFrameMax; i += frameStep)
				{
					drawLineInContentRect(i, int(contentHeight));
				}
				drawLineInContentRect(sequence.mFrameMin, int(contentHeight));
				drawLineInContentRect(sequence.mFrameMax, int(contentHeight));

				// for every entity
				customHeight = 0;
				i = 0;
				for (auto ent_it = sequence.begin(); ent_it != sequence.end(); ++ent_it, ++i)
				{
					// selection
					if (selectedEntity != sequence.end() && selectedEntity == ent_it)
					{
						customHeight = 0;
						// TODO CustomHeight
						//for (int i = 0; i < *selectedEntry; i++)
						//	customHeight += sequence.GetCustomHeight(i);
						draw_list->AddRectFilled(
							ImVec2(contentMin.x, contentMin.y + ItemHeight * i + customHeight),
							ImVec2(contentMin.x + canvas_size.x,
							       contentMin.y + ItemHeight * (i + 1) + customHeight), 0x801080FF,
							1.f);
					}

					float localCustomHeight = 0.f; //sequence.GetCustomHeight(i);
					if (ent_it == expanded_ent)
					{
						localCustomHeight += static_cast<float>(expanded_ent_child_count) * ItemHeight;
						localCustomHeight += 3*static_cast<float>(expanded_transform_ents.size()) * ItemHeight;
					}

					// draw clips
					for (auto clip_it = ent_it->second.begin(); clip_it != ent_it->second.end(); ++clip_it)
					{
						const float start = clip_it->first;
						const float end = clip_it->first + clip_it->second.duration_;
						const unsigned color = 0xFFAA8080;
						// TODO CustomHeight

						const ImVec2 pos = ImVec2(
							contentMin.x + legendWidth - static_cast<float>(firstFrame) * framePixelWidth,
							contentMin.y + ItemHeight * i + 1 + customHeight);

						const ImVec2 slotP1(pos.x + start * framePixelWidth, pos.y + 2);
						const ImVec2 slotP2(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
						const ImVec2 slotP3(pos.x + end * framePixelWidth + framePixelWidth,
						                    pos.y + ItemHeight - 2 + localCustomHeight);

						const unsigned int slotColor = color | 0xFF000000;
						const unsigned int slotColorHalf = (color & 0xFFFFFF) | 0x40000000;

						if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
						{
							draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
							draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
						}

						if (movingEntry.first == sequence.end() && (sequenceOptions &
							SEQUENCER_EDIT_STARTEND))
						{
							const ImRect rc = ImRect(slotP1, slotP2);
							if (rc.Contains(io.MousePos))
							{
								draw_list->AddRectFilled(rc.Min, rc.Max, 0xFFFFFFFF, 2);

								if (ImRect(childFramePos, childFramePos + childFrameSize).Contains(io.MousePos))
								{
									if (ImGui::IsMouseClicked(0) && !MovingScrollBar && !MovingCurrentFrame)
									{
										movingEntry = {ent_it, clip_it};
										movingPos = mouse_x;
										break;
									}
								}
							}
						}

						draw_list->PushClipRect(slotP1, slotP2, true);
						draw_list->AddText({slotP1.x + text_offset, slotP1.y}, 0xFF000000,
						                   clip_it->second.name.c_str());
						draw_list->PopClipRect();

						if (expanded_ent == ent_it)
						{
							int j = 0;
							for (auto child_it = ent_it->first.beginChild(); *child_it; ++child_it, ++j)
							{
								bool is_expanded_transform = false;
								const ImVec2 child_min = pos + ImVec2{0, ItemHeight + j * ItemHeight};

								if (expanded_transform_ents.find(*child_it) != expanded_transform_ents.end())
									is_expanded_transform = true;

								std::unordered_set<float> existing_keyframe_times;

								/*
								for (auto key_frame : clip_it->second.channels[child_it->getId()].positions)
								{
									if (existing_keyframe_times.find(key_frame.first) != existing_keyframe_times.end())
										existing_keyframe_times.insert(key_frame.first);

									if (is_expanded_transform)
										//draw keyframe rect
								
								}

								for (auto key_frame : clip_it->second.channels[child_it->getId()].rotations)
								{
									if (existing_keyframe_times.find(key_frame.first) != existing_keyframe_times.end())
										existing_keyframe_times.insert(key_frame.first);

									if (is_expanded_transform)
										//draw keyframe rect
								
								}

								for (auto key_frame : clip_it->second.channels[child_it->getId()].scales)
								{
									if (existing_keyframe_times.find(key_frame.first) != existing_keyframe_times.end())
										existing_keyframe_times.insert(key_frame.first);

									if (is_expanded_transform)
										//draw keyframe rect
								
								}*/
							}
						}
					}

					customHeight += localCustomHeight;
				}

				if (ImGui::BeginDragDropTarget())
				{
					i = 0;
					for (auto ent_it = sequence.begin(); ent_it != sequence.end(); ++ent_it, ++i)
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(
							"Animation_clip_DnD", ImGuiDragDropFlags_AcceptBeforeDelivery))
						{
							IM_ASSERT(payload->DataSize == sizeof(std::pair<Enttity, const AnimationClip*>));
							const auto payload_data = static_cast<std::pair<Enttity, const AnimationClip*>*>(payload->
								Data);

							if (ent_it->first == payload_data->first)
							{
								if (payload->IsPreview())
								{
									const float start = currentFrame;
									const float end = currentFrame + payload_data->second->duration_;

									const ImVec2 pos = ImVec2(
										contentMin.x + legendWidth - static_cast<float>(firstFrame) * framePixelWidth,
										contentMin.y + ItemHeight * i + 1 + customHeight);
									const ImVec2 slotP1(pos.x + start * framePixelWidth, pos.y + 2);
									const ImVec2 slotP2(pos.x + end * framePixelWidth + framePixelWidth,
									                    pos.y + ItemHeight - 2);

									draw_list->AddRectFilled(slotP1, slotP2, 0xFF885050, 2);

									draw_list->PushClipRect(slotP1, slotP2, true);
									draw_list->AddText({slotP1.x + text_offset, slotP1.y}, 0xFF303030,
									                   payload_data->second->name.c_str());
									draw_list->PopClipRect();
								}

								if (payload->IsDelivery())
								{
									sequence.add(ent_it->first, currentFrame, *payload_data->second);
								}
							}
						}
					}
					if (!MovingCurrentFrame)
						MovingCurrentFrame = true;
					ImGui::EndDragDropTarget();
				}

				// moving
				if (movingEntry.first != sequence.end()) ///*backgroundRect.Contains(io.MousePos) && */
				{
					ImGui::SetNextFrameWantCaptureMouse(true);
					const float diffFrame = std::round((mouse_x - movingPos) / framePixelWidth);
					if (std::abs(diffFrame) > 0)
					{
						float start = movingEntry.second.value()->first;

						start += diffFrame;

						selectedEntity = movingEntry.first;

						movingPos += diffFrame * framePixelWidth;

						movingEntry.second = sequence.updateStart(movingEntry.first, std::move(*movingEntry.second),
						                                          start);
					}
					if (!io.MouseDown[0])
					{
						// single select
						if (!diffFrame)
						{
							selectedEntity = movingEntry.first;
							ret = true;
						}

						movingEntry = {sequence.end(), std::nullopt};
					}
				}

				// draw cursor
				if (currentFrame >= static_cast<float>(firstFrame) && currentFrame <= static_cast<float>(sequence.
					mFrameMax))
				{
					const float cursorOffset = contentMin.x +
						legendWidth +
						(currentFrame - static_cast<float>(firstFrame)) * framePixelWidth +
						framePixelWidth / 2 -
						cursorWidth * 0.5f;
					draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y),
					                   0xA02A2AFF,
					                   cursorWidth);
					char tmps[512];
					ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", static_cast<int>(currentFrame));
					draw_list->AddText(ImVec2(cursorOffset + 10, canvas_pos.y + 2), 0xFF2A2AFF, tmps);
				}

				draw_list->PopClipRect();
				draw_list->PopClipRect();

				//for (auto& customDraw : customDraws)
				//	sequence.CustomDraw(customDraw.index, draw_list, customDraw.customRect, customDraw.legendRect,
				//	                    customDraw.clippingRect, customDraw.legendClippingRect);
				//for (auto& customDraw : compactCustomDraws)
				//	sequence.CustomDrawCompact(customDraw.index, draw_list, customDraw.customRect,
				//	                           customDraw.clippingRect);

				ImGui::EndChildFrame();
				ImGui::PopStyleColor();

				// draw scroll bar
				ImGui::InvisibleButton("scrollBar", scrollBarSize);
				const ImVec2 scrollBarMin = ImGui::GetItemRectMin();
				const ImVec2 scrollBarMax = ImGui::GetItemRectMax();

				// ratio = number of frames visible in control / number to total frames
				const ImVec2 scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
				const ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
				draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

				const ImRect scrollBarRect(scrollBarA, scrollBarB);
				const bool inScrollBar = scrollBarRect.Contains(io.MousePos);

				draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);

				const ImVec2 scrollBarC(scrollBarMin.x + legendWidth + frameBarPixelOffsets.x, scrollBarMin.y);
				const ImVec2 scrollBarD(scrollBarMin.x + legendWidth + frameBarPixelOffsets.y,
				                        scrollBarMax.y - 2);
				draw_list->AddRectFilled(scrollBarC, scrollBarD,
				                         (inScrollBar || MovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

				const ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
				const ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

				const bool onLeft = barHandleLeft.Contains(io.MousePos);
				const bool onRight = barHandleRight.Contains(io.MousePos);

				draw_list->AddRectFilled(barHandleLeft.Min, barHandleLeft.Max,
				                         (onLeft || sizingLBar) ? 0xFFAAAAAA : 0xFF666666, 6);
				draw_list->AddRectFilled(barHandleRight.Min, barHandleRight.Max,
				                         (onRight || sizingRBar) ? 0xFFAAAAAA : 0xFF666666, 6);

				if (ImGui::IsWindowFocused() && io.KeyAlt && io.MouseDown[2])
				{
					if (!panningView)
					{
						panningView = true;
					}

					frameBarPixelOffsetsTarget.y += io.MouseDelta.x;
					frameBarPixelOffsetsTarget.y = std::clamp(
						frameBarPixelOffsetsTarget.y, frameBarPixelOffsetsTarget.x + MinBarWidth,
						scrollBarB.x - (scrollBarMin.x + legendWidth));
					frameBarPixelOffsetsTarget.x += io.MouseDelta.x;
					frameBarPixelOffsetsTarget.x = std::clamp(
						frameBarPixelOffsetsTarget.x, 0.f, frameBarPixelOffsetsTarget.y - MinBarWidth);
				}
				if (panningView && !io.MouseDown[2])
				{
					panningView = false;
				}

				const ImRect scrollBarThumb(scrollBarC, scrollBarD);
				if (sizingRBar)
				{
					if (!io.MouseDown[0])
					{
						sizingRBar = false;
					}
					else
					{
						frameBarPixelOffsetsTarget.y += io.MouseDelta.x;
						frameBarPixelOffsetsTarget.y = std::clamp(frameBarPixelOffsetsTarget.y,
						                                          frameBarPixelOffsetsTarget.x + MinBarWidth,
						                                          scrollBarB.x - (scrollBarMin.x + legendWidth));
					}
				}
				else if (sizingLBar)
				{
					if (!io.MouseDown[0])
					{
						sizingLBar = false;
					}
					else
					{
						if (fabsf(io.MouseDelta.x) > FLT_EPSILON)
						{
							frameBarPixelOffsetsTarget.x += io.MouseDelta.x;
							frameBarPixelOffsetsTarget.x = std::clamp(
								frameBarPixelOffsetsTarget.x, 0.f, frameBarPixelOffsetsTarget.y - MinBarWidth);
						}
					}
				}
				else
				{
					if (MovingScrollBar)
					{
						if (!io.MouseDown[0])
						{
							MovingScrollBar = false;
						}
						else
						{
							frameBarPixelOffsetsTarget.y += io.MouseDelta.x;
							frameBarPixelOffsetsTarget.y = std::clamp(
								frameBarPixelOffsetsTarget.y, frameBarPixelOffsetsTarget.x + MinBarWidth,
								scrollBarB.x - (scrollBarMin.x + legendWidth));
							frameBarPixelOffsetsTarget.x += io.MouseDelta.x;
							frameBarPixelOffsetsTarget.x = std::clamp(
								frameBarPixelOffsetsTarget.x, 0.f, frameBarPixelOffsetsTarget.y - MinBarWidth);
						}
					}
					else
					{
						if (scrollBarThumb.Contains(io.MousePos) && ImGui::IsMouseClicked(0) && !
							MovingCurrentFrame && movingEntry.first == sequence.end())
						{
							MovingScrollBar = true;
							panningViewSource = io.MousePos;
							panningViewFrame = -firstFrame;
						}
						if (!sizingRBar && onRight && ImGui::IsMouseClicked(0))
							sizingRBar = true;
						if (!sizingLBar && onLeft && ImGui::IsMouseClicked(0))
							sizingLBar = true;
					}
				}
			}

			ImGui::EndGroup();

			return ret;
		}

	private:
		const char* GetCollapseFmt() { return "%d Frames / %d entries"; }
	}; //
}
