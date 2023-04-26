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
		const float MinBarWidth = 44.f;
		const int ItemHeight = 20;
		const int legendWidth = 200;

		Sequencer(AnimationSequence& sequence):
			sequence(sequence),
			selectedEntity(sequence.end())
		{
		}

		AnimationSequence& sequence;
		AnimationSequence::EntityIterator selectedEntity;
		int currentFrame = 100;
		bool expanded = true;
		int firstFrame = 0;

		//float framePixelWidth = 10.f;
		//float framePixelWidthTarget = 10.f;

		ImVec2 frameBarPixelOffsets = {0, 150};
		ImVec2 frameBarPixelOffsetsTarget = {0, 150};

		std::pair<AnimationSequence::EntityIterator, std::optional<AnimationSequence::ClipIterator>>
		movingEntry = {sequence.end(), std::nullopt};

		int movingPos = -1;

		bool MovingScrollBar = false;
		bool MovingCurrentFrame = false;

		bool panningView = false;
		ImVec2 panningViewSource;
		int panningViewFrame;

		bool sizingRBar = false;
		bool sizingLBar = false;

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

		bool draw(int sequenceOptions)
		{
			ImGui::PushItemWidth(130);
			ImGui::InputInt("Frame Min", &sequence.mFrameMin);
			ImGui::SameLine();
			ImGui::InputInt("Frame ", &currentFrame);
			ImGui::SameLine();
			ImGui::InputInt("Frame Max", &sequence.mFrameMax);
			ImGui::PopItemWidth();

			bool ret = false;
			ImGuiIO& io = ImGui::GetIO();
			int mouse_x = (int)(io.MousePos.x);
			int mouse_y = (int)(io.MousePos.y);

			int delEntry = -1;
			int dupEntry = -1;

			bool popupOpened = false;
			int sequenceCount = sequence.getAnimationComponentCount();
			if (!sequenceCount)
				return false;
			ImGui::BeginGroup();

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			ImVec2 canvas_pos = ImGui::GetCursorScreenPos(); // ImDrawList API uses screen coordinates!
			ImVec2 canvas_size = ImGui::GetContentRegionAvail(); // Resize canvas to what's available

			int controlHeight = sequenceCount * ItemHeight;
			// TODO add expanded height
			//for (int i = 0; i < sequenceCount; i++)
			//	controlHeight += int(sequence.GetCustomHeight(i));
			int frameCount = ImMax(sequence.mFrameMax - sequence.mFrameMin, 1);

			// zoom in/out
			frameBarPixelOffsets.x = ImLerp(frameBarPixelOffsets.x, frameBarPixelOffsetsTarget.x, 0.33f);
			frameBarPixelOffsets.y = ImLerp(frameBarPixelOffsets.y, frameBarPixelOffsetsTarget.y, 0.33f);

			firstFrame = ImLerp(sequence.mFrameMin, sequence.mFrameMax,
			                    frameBarPixelOffsets.x / (canvas_size.x - legendWidth));
			int lastFrame = ImLerp(sequence.mFrameMin, sequence.mFrameMax,
			                       frameBarPixelOffsets.y / (canvas_size.x - legendWidth));

			const int visibleFrameCount = lastFrame - firstFrame;
			float framePixelWidth = (canvas_size.x - legendWidth) / visibleFrameCount;

			ImRect regionRect(canvas_pos, canvas_pos + canvas_size);


			frameCount = sequence.mFrameMax - sequence.mFrameMin;
			if (visibleFrameCount >= frameCount && firstFrame)
				firstFrame = sequence.mFrameMin;

			int firstFrameUsed = firstFrame;

			// --
			if (!expanded)
			{
				ImGui::InvisibleButton("canvas", ImVec2(canvas_size.x - canvas_pos.x, (float)ItemHeight));
				draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight),
				                         0xFF3D3837, 0);
				char tmps[512];
				ImFormatString(tmps, IM_ARRAYSIZE(tmps), GetCollapseFmt(), frameCount, sequenceCount);
				draw_list->AddText(ImVec2(canvas_pos.x + 26, canvas_pos.y + 2), 0xFFFFFFFF, tmps);
			}
			else
			{
				bool hasScrollBar(true);
				/*
				int framesPixelWidth = int(frameCount * framePixelWidth);
				if ((framesPixelWidth + legendWidth) >= canvas_size.x)
				{
				    hasScrollBar = true;
				}
				*/
				// test scroll area
				ImVec2 headerSize(canvas_size.x, (float)ItemHeight);
				ImVec2 scrollBarSize(canvas_size.x, 14.f);
				ImGui::InvisibleButton("topBar", headerSize);
				draw_list->AddRectFilled(canvas_pos, canvas_pos + headerSize, 0xFFFF0000, 0);
				ImVec2 childFramePos = ImGui::GetCursorScreenPos();
				ImVec2 childFrameSize(canvas_size.x,
				                      canvas_size.y - 8.f - headerSize.y - (hasScrollBar ? scrollBarSize.y : 0));
				ImGui::PushStyleColor(ImGuiCol_FrameBg, 0);
				ImGui::BeginChildFrame(889, childFrameSize);
				//sequence.focused = ImGui::IsWindowFocused();
				ImGui::InvisibleButton("contentBar", ImVec2(canvas_size.x, float(controlHeight)));
				const ImVec2 contentMin = ImGui::GetItemRectMin();
				const ImVec2 contentMax = ImGui::GetItemRectMax();
				const ImRect contentRect(contentMin, contentMax);
				const float contentHeight = contentMax.y - contentMin.y;

				// full background
				draw_list->AddRectFilled(canvas_pos, canvas_pos + canvas_size, 0xFF242424, 0);

				// current frame top
				ImRect topRect(ImVec2(canvas_pos.x + legendWidth, canvas_pos.y),
				               ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + ItemHeight));

				if (!MovingCurrentFrame && !MovingScrollBar && movingEntry.first == sequence.end() &&
					sequenceOptions &
					SEQUENCER_CHANGE_FRAME && topRect.Contains(io.MousePos) && io.MouseDown[0])//&& currentFrame >= 0
				{
					MovingCurrentFrame = true;
				}
				if (MovingCurrentFrame)
				{
					if (frameCount)
					{
						currentFrame = (int)((io.MousePos.x - topRect.Min.x) / framePixelWidth) + firstFrameUsed;
						if (currentFrame < sequence.mFrameMin)
							currentFrame = sequence.mFrameMin;
						if (currentFrame >= sequence.mFrameMax)
							currentFrame = sequence.mFrameMax;
					}
					if (!io.MouseDown[0])
						MovingCurrentFrame = false;
				}

				//header
				draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_size.x + canvas_pos.x, canvas_pos.y + ItemHeight),
				                         0xFF3D3837, 0);
				//if (sequenceOptions & SEQUENCER_ADD)
				//{
				//	if (SequencerAddDelButton(
				//		draw_list, ImVec2(canvas_pos.x + legendWidth - ItemHeight, canvas_pos.y + 2),
				//		true))
				//		ImGui::OpenPopup("addEntry");
				//
				//	if (ImGui::BeginPopup("addEntry"))
				//	{
				//		for (int i = 0; i < sequence.GetItemTypeCount(); i++)
				//			if (ImGui::Selectable(sequence.GetItemTypeName(i)))
				//			{
				//				sequence.Add(i);
				//				*selectedEntry = sequence.GetItemCount() - 1;
				//			}
				//
				//		ImGui::EndPopup();
				//		popupOpened = true;
				//	}
				//}

				//header frame number and lines
				int modFrameCount = 10;
				int frameStep = 1;
				while ((modFrameCount * framePixelWidth) < 150)
				{
					modFrameCount *= 2;
					frameStep *= 2;
				}
				int halfModFrameCount = modFrameCount / 2;

				auto drawLine = [&](int i, int regionHeight)
				{
					bool baseIndex = ((i % modFrameCount) == 0) || (i == sequence.mFrameMax || i == sequence.mFrameMin);
					bool halfIndex = (i % halfModFrameCount) == 0;
					int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(
						firstFrameUsed * framePixelWidth);
					int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
					int tiretEnd = baseIndex ? regionHeight : ItemHeight;

					if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
					{
						draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart),
						                   ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);

						draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)ItemHeight),
						                   ImVec2((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
					}

					if (baseIndex && px > (canvas_pos.x + legendWidth))
					{
						char tmps[512];
						ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
						draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), 0xFFBBBBBB, tmps);
					}
				};

				auto drawLineContent = [&](int i, int /*regionHeight*/)
				{
					int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(
						firstFrameUsed * framePixelWidth);
					int tiretStart = int(contentMin.y);
					int tiretEnd = int(contentMax.y);

					if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
					{
						draw_list->AddLine(ImVec2(float(px), float(tiretStart)), ImVec2(float(px), float(tiretEnd)),
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

				// draw item names in the legend rect on the left
				size_t customHeight = 0;
				int i = 0;
				for (auto ent_it = sequence.begin(); ent_it != sequence.end(); ++ent_it, ++i)
				{
					ImVec2 tpos(contentMin.x + 3, contentMin.y + i * ItemHeight + 2 + customHeight);
					draw_list->AddText(tpos, 0xFFFFFFFF, ent_it->first.getComponent<TagComponent>().tag.c_str());

					if (sequenceOptions & SEQUENCER_DEL)
					{
						if (SequencerAddDelButton(
							draw_list, ImVec2(contentMin.x + legendWidth - ItemHeight + 2 - 10, tpos.y + 2), false))
							delEntry = i;

						if (SequencerAddDelButton(
							draw_list,
							ImVec2(contentMin.x + legendWidth - ItemHeight - ItemHeight + 2 - 10, tpos.y + 2),
							true))
							dupEntry = i;
					}
					// TODO CustomHeight
					//customHeight += sequence.GetCustomHeight(i);
				}

				// slots background
				customHeight = 0;
				for (int i = 0; i < sequenceCount; i++)
				{
					unsigned int col = (i & 1) ? 0xFF3A3636 : 0xFF413D3D;

					// TODO CustomHeight
					size_t localCustomHeight = 0; // sequence.GetCustomHeight(i);
					ImVec2 pos = ImVec2(contentMin.x + legendWidth, contentMin.y + ItemHeight * i + 1 + customHeight);
					ImVec2 sz = ImVec2(canvas_size.x + canvas_pos.x, pos.y + ItemHeight - 1 + localCustomHeight);
					if (!popupOpened && mouse_y >= pos.y && mouse_y < pos.y + (ItemHeight + localCustomHeight) &&
						movingEntry.first != sequence.end() &&
						mouse_x > contentMin.x && mouse_x < contentMin.x + canvas_size.x)
					{
						col += 0x80201008;
						pos.x -= legendWidth;
					}
					draw_list->AddRectFilled(pos, sz, col, 0);
					customHeight += localCustomHeight;
				}

				draw_list->PushClipRect(childFramePos + ImVec2(float(legendWidth), 0.f), childFramePos + childFrameSize,
				                        true);

				// vertical frame lines in content area
				for (int i = sequence.mFrameMin; i <= sequence.mFrameMax; i += frameStep)
				{
					drawLineContent(i, int(contentHeight));
				}
				drawLineContent(sequence.mFrameMin, int(contentHeight));
				drawLineContent(sequence.mFrameMax, int(contentHeight));

				// slots
				customHeight = 0;
				i = 0;
				for (auto ent_it = sequence.begin(); ent_it != sequence.end(); ++ent_it, ++i)
				{
					// TODO selection
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

					for (auto clip_it = ent_it->second.begin(); clip_it != ent_it->second.end(); ++clip_it)
					{
						int start = clip_it->first;
						int end = clip_it->first + clip_it->second.duration_;
						unsigned color = 0xFFAA8080;
						// TODO CustomHeight
						size_t localCustomHeight = 0; //sequence.GetCustomHeight(i);

						ImVec2 pos = ImVec2(contentMin.x + legendWidth - firstFrameUsed * framePixelWidth,
						                    contentMin.y + ItemHeight * i + 1 + customHeight);
						ImVec2 slotP1(pos.x + start * framePixelWidth, pos.y + 2);
						ImVec2 slotP2(pos.x + end * framePixelWidth + framePixelWidth, pos.y + ItemHeight - 2);
						ImVec2 slotP3(pos.x + end * framePixelWidth + framePixelWidth,
						              pos.y + ItemHeight - 2 + localCustomHeight);
						unsigned int slotColor = color | 0xFF000000;
						unsigned int slotColorHalf = (color & 0xFFFFFF) | 0x40000000;
						float text_offset = 2;
						if (slotP1.x <= (canvas_size.x + contentMin.x) && slotP2.x >= (contentMin.x + legendWidth))
						{
							draw_list->AddRectFilled(slotP1, slotP3, slotColorHalf, 2);
							draw_list->AddRectFilled(slotP1, slotP2, slotColor, 2);
						}
						//if (ImRect(slotP1, slotP2).Contains(io.MousePos) && io.MouseDoubleClicked[0])
						//{
						//	sequence.DoubleClick(i);
						//}

						// Ensure grabbable handles
						if (movingEntry.first == sequence.end() && (sequenceOptions &
							SEQUENCER_EDIT_STARTEND))
						// TODOFOCUS && backgroundRect.Contains(io.MousePos))
						{
							ImRect rc = ImRect(slotP1, slotP2);
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

						// custom draw
						//if (localCustomHeight > 0)
						//{
						//	ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + 1 + customHeight);
						//	ImRect customRect(
						//		rp + ImVec2(
						//			legendWidth - (firstFrameUsed - sequence.mFrameMin - 0.5f) * framePixelWidth,
						//			float(ItemHeight)),
						//		rp + ImVec2(
						//			legendWidth + (sequence.mFrameMax - firstFrameUsed - 0.5f + 2.f) *
						//			framePixelWidth,
						//			float(localCustomHeight + ItemHeight)));
						//	ImRect clippingRect(rp + ImVec2(float(legendWidth), float(ItemHeight)),
						//	                    rp + ImVec2(canvas_size.x, float(localCustomHeight + ItemHeight)));
						//
						//	ImRect legendRect(rp + ImVec2(0.f, float(ItemHeight)),
						//	                  rp + ImVec2(float(legendWidth), float(localCustomHeight)));
						//	ImRect legendClippingRect(canvas_pos + ImVec2(0.f, float(ItemHeight)),
						//	                          canvas_pos + ImVec2(float(legendWidth),
						//	                                              float(localCustomHeight + ItemHeight)));
						//	customDraws.push_back({i, customRect, legendRect, clippingRect, legendClippingRect});
						//}
						//else
						//{
						//	ImVec2 rp(canvas_pos.x, contentMin.y + ItemHeight * i + customHeight);
						//	ImRect customRect(
						//		rp + ImVec2(
						//			legendWidth - (firstFrameUsed - sequence.mFrameMin - 0.5f) * framePixelWidth,
						//			float(0.f)),
						//		rp + ImVec2(
						//			legendWidth + (sequence.mFrameMax - firstFrameUsed - 0.5f + 2.f) *
						//			framePixelWidth,
						//			float(ItemHeight)));
						//	ImRect clippingRect(rp + ImVec2(float(legendWidth), float(0.f)),
						//	                    rp + ImVec2(canvas_size.x, float(ItemHeight)));
						//
						//	compactCustomDraws.push_back({i, customRect, ImRect(), clippingRect, ImRect()});
						//}
						customHeight += localCustomHeight;
					}
				}

				// moving
				if (movingEntry.first != sequence.end()) ///*backgroundRect.Contains(io.MousePos) && */
				{
					ImGui::SetNextFrameWantCaptureMouse(true);
					int diffFrame = static_cast<int>((mouse_x - movingPos) / framePixelWidth);
					if (std::abs(diffFrame) > 0)
					{
						int start = movingEntry.second.value()->first;

						start += diffFrame;

						selectedEntity = movingEntry.first;

						movingPos += static_cast<int>(diffFrame * framePixelWidth);

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

				// cursor
				if (currentFrame >= firstFrame && currentFrame <= sequence.mFrameMax)
				{
					float cursorOffset = contentMin.x + legendWidth + (currentFrame - firstFrameUsed) * framePixelWidth
						+
						framePixelWidth / 2 - cursorWidth * 0.5f;
					draw_list->AddLine(ImVec2(cursorOffset, canvas_pos.y), ImVec2(cursorOffset, contentMax.y),
					                   0xA02A2AFF,
					                   cursorWidth);
					char tmps[512];
					ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", currentFrame);
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

				// copy paste
				if (sequenceOptions & SEQUENCER_COPYPASTE)
				{
					ImRect rectCopy(ImVec2(contentMin.x + 100, canvas_pos.y + 2)
					                , ImVec2(contentMin.x + 100 + 30, canvas_pos.y + ItemHeight - 2));
					bool inRectCopy = rectCopy.Contains(io.MousePos);
					unsigned int copyColor = inRectCopy ? 0xFF1080FF : 0xFF000000;
					draw_list->AddText(rectCopy.Min, copyColor, "Copy");

					ImRect rectPaste(ImVec2(contentMin.x + 140, canvas_pos.y + 2)
					                 , ImVec2(contentMin.x + 140 + 30, canvas_pos.y + ItemHeight - 2));
					bool inRectPaste = rectPaste.Contains(io.MousePos);
					unsigned int pasteColor = inRectPaste ? 0xFF1080FF : 0xFF000000;
					draw_list->AddText(rectPaste.Min, pasteColor, "Paste");

					if (inRectCopy && io.MouseReleased[0])
					{
						//sequence.Copy();
					}
					if (inRectPaste && io.MouseReleased[0])
					{
						//sequence.Paste();
					}
				}
				//

				ImGui::EndChildFrame();
				ImGui::PopStyleColor();
				if (hasScrollBar)
				{
					ImGui::InvisibleButton("scrollBar", scrollBarSize);
					ImVec2 scrollBarMin = ImGui::GetItemRectMin();
					ImVec2 scrollBarMax = ImGui::GetItemRectMax();

					// ratio = number of frames visible in control / number to total frames
					ImVec2 scrollBarA(scrollBarMin.x + legendWidth, scrollBarMin.y - 2);
					ImVec2 scrollBarB(scrollBarMin.x + canvas_size.x, scrollBarMax.y - 1);
					draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF222222, 0);

					ImRect scrollBarRect(scrollBarA, scrollBarB);
					bool inScrollBar = scrollBarRect.Contains(io.MousePos);

					draw_list->AddRectFilled(scrollBarA, scrollBarB, 0xFF101010, 8);

					ImVec2 scrollBarC(scrollBarMin.x + legendWidth + frameBarPixelOffsets.x, scrollBarMin.y);
					ImVec2 scrollBarD(scrollBarMin.x + legendWidth + frameBarPixelOffsets.y,
					                  scrollBarMax.y - 2);
					draw_list->AddRectFilled(scrollBarC, scrollBarD,
					                         (inScrollBar || MovingScrollBar) ? 0xFF606060 : 0xFF505050, 6);

					ImRect barHandleLeft(scrollBarC, ImVec2(scrollBarC.x + 14, scrollBarD.y));
					ImRect barHandleRight(ImVec2(scrollBarD.x - 14, scrollBarC.y), scrollBarD);

					bool onLeft = barHandleLeft.Contains(io.MousePos);
					bool onRight = barHandleRight.Contains(io.MousePos);

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

					ImRect scrollBarThumb(scrollBarC, scrollBarD);
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
			}

			ImGui::EndGroup();

			if (SequencerAddDelButton(draw_list, ImVec2(canvas_pos.x + 2, canvas_pos.y + 2), !expanded))
				expanded = !expanded;

			if (delEntry != -1)
			{
				//sequence.Del(delEntry);
				//if (selectedEntry && (*selectedEntry == delEntry || *selectedEntry >= sequence.getClipCount()))
				//	*selectedEntry = -1;
			}

			if (dupEntry != -1)
			{
				//sequence.Duplicate(dupEntry);
			}
			return ret;
		}

	private:
		const char* GetCollapseFmt() { return "%d Frames / %d entries"; }

		static bool SequencerAddDelButton(ImDrawList* draw_list, ImVec2 pos, bool add = true)
		{
			ImGuiIO& io = ImGui::GetIO();
			ImRect btnRect(pos, ImVec2(pos.x + 16, pos.y + 16));
			bool overBtn = btnRect.Contains(io.MousePos);
			bool containedClick = overBtn && btnRect.Contains(io.MouseClickedPos[0]);
			bool clickedBtn = containedClick && io.MouseReleased[0];
			int btnColor = overBtn ? 0xAAEAFFAA : 0x77A3B2AA;
			if (containedClick && io.MouseDownDuration[0] > 0)
				btnRect.Expand(2.0f);

			float midy = pos.y + 16 / 2 - 0.5f;
			float midx = pos.x + 16 / 2 - 0.5f;
			draw_list->AddRect(btnRect.Min, btnRect.Max, btnColor, 4);
			draw_list->AddLine(ImVec2(btnRect.Min.x + 3, midy), ImVec2(btnRect.Max.x - 3, midy), btnColor, 2);
			if (add)
				draw_list->AddLine(ImVec2(midx, btnRect.Min.y + 3), ImVec2(midx, btnRect.Max.y - 3), btnColor, 2);
			return clickedBtn;
		}

		//void drawLine(int i, int regionHeight)
		//{
		//	int modFrameCount = 10;
		//	int frameStep = 1;
		//	while ((modFrameCount * framePixelWidth) < 150)
		//	{
		//		modFrameCount *= 2;
		//		frameStep *= 2;
		//	};
		//	int halfModFrameCount = modFrameCount / 2;
		//
		//	bool baseIndex = ((i % modFrameCount) == 0) || (i == sequence.mFrameMax || i == sequence.mFrameMin);
		//	bool halfIndex = (i % halfModFrameCount) == 0;
		//	int px = (int)canvas_pos.x + int(i * framePixelWidth) + legendWidth - int(
		//		firstFrameUsed * framePixelWidth);
		//	int tiretStart = baseIndex ? 4 : (halfIndex ? 10 : 14);
		//	int tiretEnd = baseIndex ? regionHeight : ItemHeight;
		//
		//	if (px <= (canvas_size.x + canvas_pos.x) && px >= (canvas_pos.x + legendWidth))
		//	{
		//		draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)tiretStart),
		//		                   ImVec2((float)px, canvas_pos.y + (float)tiretEnd - 1), 0xFF606060, 1);
		//
		//		draw_list->AddLine(ImVec2((float)px, canvas_pos.y + (float)ItemHeight),
		//		                   ImVec2((float)px, canvas_pos.y + (float)regionHeight - 1), 0x30606060, 1);
		//	}
		//
		//	if (baseIndex && px > (canvas_pos.x + legendWidth))
		//	{
		//		char tmps[512];
		//		ImFormatString(tmps, IM_ARRAYSIZE(tmps), "%d", i);
		//		draw_list->AddText(ImVec2((float)px + 3.f, canvas_pos.y), 0xFFBBBBBB, tmps);
		//	}
		//};
	}; //
}
