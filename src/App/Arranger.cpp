#include "App/Arranger.h"
#include "App/Nodes/NodeTypes/VariableNode.h"

#include "imgui.h"

Arranger* Arranger::instance = nullptr;

Arranger::Arranger()
{
	instance = this;
}

void Arranger::Work()
{
	for (VariableNode* node : VariableNode::variableNodes)
		node->output = node->getValue(time);
	time += AudioChannel::dt * tempo / 60.0f;
}

// in our own window
void Arranger::UI(DrawStyle* drawStyle)
{
	ImGui::InputFloat("tempo", &tempo, 20.0f, 300.0f);
	tempo = clamp(tempo, 20.0f, 600.0f);

    ImGui::BeginDisabled(playing);
    if (ImGui::Button("Play"))
        playing = true;
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::BeginDisabled(!playing);
    if (ImGui::Button("Stop"))
        playing = false;
    ImGui::EndDisabled();
    ImGui::SameLine();
    if (ImGui::Button("Reset"))
        time = 0.0f;

    if (!ImGui::GetIO().WantTextInput && ImGui::IsKeyPressed(ImGuiKey_Space))
        playing = !playing;

    // Child 1: list of input nodes
    {
        
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, padding));
        ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, ImGui::GetContentRegionAvail().y), true);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 4.0f));

        for (size_t i = 0; i < VariableNode::variableNodes.size(); i++)
        {
            VariableNode* node = VariableNode::variableNodes[i];
            ImGui::PushID((int)i);
            ImGui::BeginChild("AAAA", ImVec2(ImGui::GetContentRegionAvail().x, rowHeight / scale.y), true);

            // name editing
            char buf[32]{ };
            strcpy_s<32>(buf, node->id.c_str());
            ImGui::InputText("id", buf, 32);
            node->id = std::string(buf);

            ImGui::EndChild();
            ImGui::PopID();
        }

        ImGui::PopStyleVar(2);
        ImGui::EndChild();
        ImGui::PopStyleVar();
    }

    ImGui::SameLine();

    // Child 2: node data
    {
        ImGui::BeginChild("ChildR", ImVec2(0.0f, 0.0f));

        canvasPixelPos = (v2)ImGui::GetCursorScreenPos();
        canvasPixelSize = v2(ImGui::GetContentRegionAvail());
        if (canvasPixelSize.x < 50.0f) canvasPixelSize.x = 50.0f;
        v2 canvasBottomRight = canvasPixelPos + canvasPixelSize;

        // Draw border and background color
        ImGuiIO& io = ImGui::GetIO();
        drawList.dl = ImGui::GetWindowDrawList();
        drawList.style = drawStyle;
        drawList.convertPosition = false;
        drawList.RectFilled(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_BG);
        drawList.Rect(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_Edge);
        drawList.scaleFactor = scale.x;

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvasPixelSize.ImGui(), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool isHovered = ImGui::IsItemHovered(); // Hovered
        const bool isActive = ImGui::IsItemActive();   // Held
        const v2 mouseCanvasPos = ScreenToCanvas((v2)io.MousePos);
        const v2 mousePos = CanvasToPosition(mouseCanvasPos);

        // DOESNT WORK
        int hoveredID = (int)(mousePos.y / rowHeight);
        if (hoveredID < 0 || hoveredID >= VariableNode::variableNodes.size() || !isHovered)
            hoveredID = -1;

        // Pan
        if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            position.x -= io.MouseDelta.x * scale.x;
            position.y -= io.MouseDelta.y * scale.y;
        }

        // test for node changes
        int closestCell = -1;
        float closestDist = 1000.0f;
        if (hoveredID != -1)
        {
            VariableNode* hoveredNode = VariableNode::variableNodes[hoveredID];
            for (size_t i = 0; i < hoveredNode->points.size(); i++)
            {
                v2 centre = ptcts(hoveredNode->points[i].scale(pixelsPerBeat, rowHeight) + v2(0.0f, rowHeight * hoveredID));
                float dist = centre.distanceTo((v2)io.MousePos);
                if (dist < closestDist && dist < 8.0f)
                {
                    closestDist = dist;
                    closestCell = (int)i;
                }
            }
        }

        // selection box
        if (!selectingStuff && isActive && closestCell == -1 && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            selectingStuff = true;
            selectionStart = mousePos;
        }
        else if (selectingStuff && ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            selectingStuff = false;
            // inefficient but shouldn't matter
            if (!io.KeyShift)
                selectedStack.clear();
            
            bbox2 box = bbox2(mousePos, selectionStart);
            for (size_t i = 0; i < VariableNode::variableNodes.size(); i++)
            {
                VariableNode* variable = VariableNode::variableNodes[i];
                for (size_t j = 0; j < variable->points.size(); j++)
                {
                    if (box.contains(variable->points[j].scale(pixelsPerBeat, rowHeight) + v2(0.0f, rowHeight * i)))
                        selectedStack.push_back({ i, j });
                }
            }
        }
        // moving nodes
        else if (!isDraggingNode && closestCell != -1 && isActive && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
        {
            isDraggingNode = true;

            // put clicked on node onto stack if not already
            bool selectedAlready = inSelectedStack(hoveredID, closestCell);
            if (!io.KeyShift && !selectedAlready)
                selectedStack.clear();
            if (!selectedAlready)
                selectedStack.push_back({ hoveredID, closestCell });
        }
        else if (isDraggingNode && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            // check through every node and find if the current move is possible, and if not, work out how far it can move
            // then actually move all the nodes, knowing it will work.
            v2 diff = v2(io.MouseDelta.x * scale.x / pixelsPerBeat, io.MouseDelta.y * scale.y / rowHeight);
            for (auto& pair : selectedStack)
            {
                auto& ref = VariableNode::variableNodes[pair.first]->points;
                v2 np = ref[pair.second] + diff;
                if (np.x < 0.0f)
                    diff.x = -ref[pair.second].x;

                if (pair.second > 0 && !inSelectedStack(pair.first, pair.second - 1) && np.x < ref[pair.second - 1].x)
                    diff.x = ref[pair.second - 1].x - ref[pair.second].x;
                else if (pair.second < ref.size() - 1 && !inSelectedStack(pair.first, pair.second + 1) && np.x > ref[pair.second + 1].x)
                    diff.x = ref[pair.second + 1].x - ref[pair.second].x;

                if (np.y < 0.0f)
                    diff.y = -ref[pair.second].y;
                else if (np.y > 1.0f)
                    diff.y = 1.0f - ref[pair.second].y;
            }
            for (auto& pair : selectedStack)
            {
                auto& ref = VariableNode::variableNodes[pair.first]->points;
                ref[pair.second] += diff;
            }
        }
        else if (isDraggingNode)
            isDraggingNode = false;

        // moving time cursor
        else if (isActive && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            draggingTimeCursor = true;
        else if (draggingTimeCursor && ImGui::IsMouseDown(ImGuiMouseButton_Left))
            time = std::max(0.0f, mousePos.x / pixelsPerBeat);
        else if (draggingTimeCursor)
        {
            // snap time to quarter notes
            time = std::roundf(time);
            draggingTimeCursor = false;
        }

        // taken from LevelEditor\...\Editor.cpp
        if (isHovered && io.MouseWheel != 0.0f)
        {
            if (io.KeyShift)
                scalingLevel.y -= (int)io.MouseWheel;
            else
                scalingLevel.x -= (int)io.MouseWheel;

            // clamp(zoomLevel, 0, 31) inclusive
            scalingLevel.x = scalingLevel.x >= 0 ? (scalingLevel.x < NUM_SCALING_LEVELS ? scalingLevel.x : NUM_SCALING_LEVELS - 1) : 0;
            scalingLevel.y = scalingLevel.y >= 0 ? (scalingLevel.y < NUM_SCALING_LEVELS ? scalingLevel.y : NUM_SCALING_LEVELS - 1) : 0;
            // 1.1 ^ -15
            v2 prevScale = scale;
            scale.x = GetSFFromScalingLevel(scalingLevel.x);
            scale.y = GetSFFromScalingLevel(scalingLevel.y);
            // position + (mousePosBefore = canvasPos * scaleBefore + position) - (mousePosAfter = canvasPos * scaleAfter + position)
            // position + canvasPos * (scaleBefore - scaleAfter)
            // somehow it has to be negative... I hate you linear algebra!!!
            position -= mouseCanvasPos.scale(prevScale - scale);
        }
        position.x = std::max(0.0f, position.x);
        position.y = std::max(0.0f, position.y);

        // Context menu (under default mouse threshold)
        ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
        if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
            ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
        if (ImGui::BeginPopup("context"))
        {
            ImGui::EndPopup();
        }

        // Draw grid + all lines in the canvas
        drawList.dl->PushClipRect((canvasPixelPos + 1.0f).ImGui(), (canvasBottomRight - 1.0f).ImGui(), true);
        float gridStep = 1.0f / scale.x * pixelsPerBeat * 4.0f;
        float gridStepSmall = 1.0f / scale.x * pixelsPerBeat;
        for (float x = fmodf(-position.x / scale.x, gridStep) - gridStep; x < canvasPixelSize.x; x += gridStep)
        {
            drawList.Line(v2(padding + canvasPixelPos.x + x, canvasPixelPos.y), v2(padding + canvasPixelPos.x + x, canvasBottomRight.y), DrawColour::Canvas_GridLinesHeavy);
            if (scalingLevel.x < 21)
                for (int dx = 1; dx < 4; dx++)
                    drawList.Line(
                        ImVec2(padding + canvasPixelPos.x + x + dx * gridStepSmall, canvasPixelPos.y),
                        ImVec2(padding + canvasPixelPos.x + x + dx * gridStepSmall, canvasBottomRight.y), DrawColour::Canvas_GridLinesLight);
        }

        drawList.Line(
            canvasPixelPos + v2(0.0f, padding),
            canvasPixelPos + v2(canvasPixelSize.x, padding),
            DrawColour::Canvas_GridLinesHeavy
        );

        for (size_t j = 0; j < VariableNode::variableNodes.size(); j++)
        {
            VariableNode* node = VariableNode::variableNodes[j];

            if (node->points.size() == 0)
                continue;
            if (position.x <= node->points[0].x * pixelsPerBeat)
                drawList.Line(
                    ptcts(v2(position.x, node->points[0].y * rowHeight + rowHeight * j)),
                    ptcts(node->points[0].scale(pixelsPerBeat, rowHeight) + v2(0.0f, rowHeight * j)),
                    ImColor(1.0f, 1.0f, 1.0f)
                );
            if (position.x + canvasPixelSize.x * scale.x >= node->points[node->points.size() - 1].x * pixelsPerBeat)
                drawList.Line(
                    ptcts(node->points[node->points.size() - 1].scale(pixelsPerBeat, rowHeight) + v2(0.0f, rowHeight * j)),
                    ptcts(v2(position.x + canvasPixelSize.x * scale.x, node->points[node->points.size() - 1].y * rowHeight + rowHeight * j)),
                    ImColor(1.0f, 1.0f, 1.0f)
                );

            for (size_t i = 0; i < node->points.size() - 1; i++)
                drawList.Line(
                    ptcts(node->points[i].scale(pixelsPerBeat, rowHeight) + v2(0.0f, rowHeight * j)),
                    ptcts(node->points[i + 1].scale(pixelsPerBeat, rowHeight) + v2(0.0f, rowHeight * j)),
                    ImColor(1.0f, 1.0f, 1.0f)
                );

            drawList.Line(
                canvasPixelPos + v2(0.0f, -position.y / scale.y + padding + rowHeight * (j + 1) / scale.y - 1.0f),
                canvasPixelPos + v2(canvasPixelSize.x, -position.y / scale.y + padding + rowHeight * (j + 1) / scale.y - 1.0f),
                DrawColour::Canvas_GridLinesHeavy
            );

            const float cellR = 3.0f;
            for (size_t i = 0; i < node->points.size(); i++)
            {
                v2 centre = ptcts(node->points[i].scale(pixelsPerBeat, rowHeight) + v2(0.0f, rowHeight * j));

                bool selected = inSelectedStack(j, i);

                if (selected)
                    drawList.Rect(centre - cellR - 2.0f, centre + cellR + 2.0f, ImColor(1.0f, 0.0f, 1.0f));
                if (j == hoveredID)
                    drawList.RectFilled(centre - cellR, centre + cellR, ImColor(1.0f, i == closestCell ? 0.0f : 1.0f, 1.0f));
                else
                    drawList.Rect(centre - cellR, centre + cellR, ImColor(1.0f, 1.0f, 1.0f));
            }
        }

        drawList.Line(
            canvasPixelPos + v2(padding + (time * pixelsPerBeat - position.x) / scale.x, 0.0f),
            canvasPixelPos + v2(padding + (time * pixelsPerBeat - position.x) / scale.x, canvasPixelSize.y),
            DrawColour::Node_IOFloat
        );

        // draw selection box
        if (selectingStuff)
        {
            bbox2 box = bbox2(mousePos, selectionStart);
            drawList.RectFilled(ptcts(box.a), ptcts(box.b), DrawColour::Node_SelectionFill);
            drawList.Rect(ptcts(box.a), ptcts(box.b), DrawColour::Node_SelectionOutline);
        }

        drawList.dl->PopClipRect();
        ImGui::EndChild();
    }
}

int Arranger::getBeat(int mod, float division) const
{
	return (int)(time / division) % mod;
}

float Arranger::getTime(float period) const
{
	return fmodf(time / (float)period, 1.0f);
}

float Arranger::GetSFFromScalingLevel(int scaling)
{
    float z = MIN_SCALE;
    for (int i = 0; i < scaling; i++)
        z *= 1.1f;
    return z;
}

bool Arranger::inSelectedStack(int i, int j) const
{
    for (auto& pair : selectedStack)
        if (pair.first == i && pair.second == j)
            return true;
    return false;
}

v2 Arranger::ScreenToCanvas(const v2& pos) const // c = s + p
{
    return canvasPixelPos + padding - pos;
}

v2 Arranger::CanvasToScreen(const v2& pos) const // s = c - p
{
    return canvasPixelPos + padding - pos;
}

v2 Arranger::CanvasToPosition(const v2& pos) const // position = offset - canvas * scale
{
    return position - pos.scale(scale);
}

v2 Arranger::PositionToCanvas(const v2& pos) const // canvas = (offset - position) / scale
{
    return (position - pos).scale(scale.reciprocal());
}