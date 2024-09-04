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
        ImGui::BeginChild("ChildL", ImVec2(ImGui::GetContentRegionAvail().x * 0.4f, pixelHeight));
        
        for (size_t i = 0; i < VariableNode::variableNodes.size(); i++)
        {
            VariableNode* node = VariableNode::variableNodes[i];
            ImGui::PushID((int)i);
            ImGui::BeginChild("AAAA", ImVec2(ImGui::GetContentRegionAvail().x, 40.0f));

            // name editing
            char buf[32]{ };
            strcpy_s<32>(buf, node->id.c_str());
            ImGui::InputText("id", buf, 32);
            node->id = std::string(buf);

            ImGui::EndChild();
            ImGui::PopID();
        }
        
        ImGui::EndChild();
    }

    ImGui::SameLine();

    // Child 2: node data
    {
        ImGui::BeginChild("ChildR", ImVec2(0.0f, pixelHeight));

        canvasPixelPos = (v2)ImGui::GetCursorScreenPos();
        canvasPixelSize = v2(ImGui::GetContentRegionAvail().x, pixelHeight);
        if (canvasPixelSize.x < 50.0f) canvasPixelSize.x = 50.0f;
        v2 canvasBottomRight = canvasPixelPos + canvasPixelSize;

        // Draw border and background color
        ImGuiIO& io = ImGui::GetIO();
        drawList.dl = ImGui::GetWindowDrawList();
        drawList.style = drawStyle;
        drawList.convertPosition = false;
        drawList.RectFilled(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_BG);
        drawList.Rect(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_Edge);
        drawList.scaleFactor = scale;

        // This will catch our interactions
        ImGui::InvisibleButton("canvas", canvasPixelSize.ImGui(), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
        const bool isHovered = ImGui::IsItemHovered(); // Hovered
        const bool isActive = ImGui::IsItemActive();   // Held
        const v2 mouseCanvasPos = ScreenToCanvas((v2)io.MousePos);
        const v2 mousePos = CanvasToPosition(mouseCanvasPos);

        // Pan
        if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            position -= io.MouseDelta.x * scale;
        }

        if (isActive && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
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
            scalingLevel -= (int)io.MouseWheel;
            // clamp(zoomLevel, 0, 31) inclusive
            scalingLevel = scalingLevel >= 0 ? (scalingLevel < NUM_SCALING_LEVELS ? scalingLevel : NUM_SCALING_LEVELS - 1) : 0;
            // 1.1 ^ -15
            float prevScale = scale;
            scale = GetSFFromScalingLevel(scalingLevel);
            // position + (mousePosBefore = canvasPos * scaleBefore + position) - (mousePosAfter = canvasPos * scaleAfter + position)
            // position + canvasPos * (scaleBefore - scaleAfter)
            // somehow it has to be negative... I hate you linear algebra!!!
            position -= mouseCanvasPos.x * (prevScale - scale);
        }
        position = std::max(0.0f, position);

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
        float gridStep = 1.0f / scale * pixelsPerBeat * 4.0f;
        float gridStepSmall = 1.0f / scale * pixelsPerBeat;
        for (float x = fmodf(-position / scale, gridStep) - gridStep; x < canvasPixelSize.x; x += gridStep)
        {
            drawList.Line(v2(padding + canvasPixelPos.x + x, canvasPixelPos.y), v2(padding + canvasPixelPos.x + x, canvasBottomRight.y), DrawColour::Canvas_GridLinesHeavy);
            if (scalingLevel < 21)
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
            if (position <= node->points[0].x * pixelsPerBeat)
                drawList.Line(
                    v2(padding, padding + 40.0f * j) + ptcts(v2(position, node->points[0].y * 40.0f)),
                    v2(padding, padding + 40.0f * j) + ptcts(node->points[0].scale(v2(pixelsPerBeat, 40.0f))),
                    ImColor(1.0f, 1.0f, 1.0f)
                );
            if (position + canvasPixelSize.x * scale >= node->points[node->points.size() - 1].x * pixelsPerBeat)
                drawList.Line(
                    v2(padding, padding + 40.0f * j) + ptcts(node->points[node->points.size() - 1].scale(v2(pixelsPerBeat, 40.0f))),
                    v2(padding, padding + 40.0f * j) + ptcts(v2(position + canvasPixelSize.x * scale, node->points[node->points.size() - 1].y * 40.0f)),
                    ImColor(1.0f, 1.0f, 1.0f)
                );

            for (size_t i = 0; i < node->points.size() - 1; i++)
                drawList.Line(
                    v2(padding, padding + 40.0f * j) + ptcts(node->points[i].scale(v2(pixelsPerBeat, 40.0f))),
                    v2(padding, padding + 40.0f * j) + ptcts(node->points[i + 1].scale(v2(pixelsPerBeat, 40.0f))),
                    ImColor(1.0f, 1.0f, 1.0f)
                );

            drawList.Line(
                canvasPixelPos + v2(0.0f, padding + 40.0f * (j + 1)),
                canvasPixelPos + v2(canvasPixelSize.x, padding + 40.0f * (j + 1)),
                DrawColour::Canvas_GridLinesHeavy
            );
        }

        drawList.Line(
            v2(padding, 0.0f) + ptcts(v2(time * pixelsPerBeat, 0.0f)),
            v2(padding, 0.0f) + ptcts(v2(time * pixelsPerBeat, canvasPixelSize.y)),
            DrawColour::Node_IOFloat
        );

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

v2 Arranger::ScreenToCanvas(const v2& pos) const // c = s + p
{
    return canvasPixelPos - pos;
}

v2 Arranger::CanvasToScreen(const v2& pos) const // s = c - p
{
    return canvasPixelPos - pos;
}

v2 Arranger::CanvasToPosition(const v2& pos) const // position = offset - canvas * scale
{
    return v2(position, 0.0f) - pos.scale(v2(scale, 1.0f));
}

v2 Arranger::PositionToCanvas(const v2& pos) const // canvas = (offset - position) / scale
{
    return (v2(position, 0.0f) - pos).scale(v2(scale, 1.0f).reciprocal());
}