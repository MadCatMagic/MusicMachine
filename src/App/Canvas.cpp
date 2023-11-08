#include "App/Canvas.h"
#include "imgui.h"


// a lot of this code is taken from the ImGui canvas example
void Canvas::CreateWindow()
{
    ImGui::Begin("Canvas");
    ImGui::Text("Mouse Left: drag to add lines,\nMouse Right: drag to scroll, click for context menu.");
    ImGui::InputFloat2("position", &position.x);

    // Using InvisibleButton() as a convenience 
    // 1) it will advance the layout cursor and 
    // 2) allows us to use IsItemHovered()/IsItemActive()
    canvasPixelPos = (v2)ImGui::GetCursorScreenPos();
    canvasPixelSize = ImGui::GetContentRegionAvail();
    if (canvasPixelSize.x < 50.0f) canvasPixelSize.x = 50.0f;
    if (canvasPixelSize.y < 50.0f) canvasPixelSize.y = 50.0f;
    v2 canvasBottomRight = canvasPixelPos + canvasPixelSize;

    // Draw border and background color
    ImGuiIO& io = ImGui::GetIO();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddRectFilled(canvasPixelPos.ImGui(), canvasBottomRight.ImGui(), IM_COL32(50, 50, 50, 255));
    draw_list->AddRect(canvasPixelPos.ImGui(), canvasBottomRight.ImGui(), IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvasPixelSize.ImGui(), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool isHovered = ImGui::IsItemHovered(); // Hovered
    const bool isActive = ImGui::IsItemActive();   // Held
    const v2 mouseCanvasPos = ScreenToCanvas((v2)io.MousePos);
    const v2 mousePos = CanvasToPosition(mouseCanvasPos);

    // Pan (we use a zero mouse threshold when there's no context menu)
    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        position.x += io.MouseDelta.x * scale.x;
        position.y += io.MouseDelta.y * scale.y;
    }

    // taken from LevelEditor\...\Editor.cpp
    if (isHovered && io.MouseWheel != 0.0f)
        scalingLevel -= (int)io.MouseWheel;
    // clamp(zoomLevel, 0, 15) inclusive
    scalingLevel = scalingLevel >= 0 ? (scalingLevel < 16 ? scalingLevel : 15) : 0;
    // 1.1 ^ -7
    float z = 0.51315811823f;
    for (int i = 0; i < scalingLevel; i++)
        z *= 1.1f;
    v2 prevScale = scale;
    scale = z;
    // position + (mousePosBefore = canvasPos * scaleBefore + position) - (mousePosAfter = canvasPos * scaleAfter + position)
    // position + canvasPos * (scaleBefore - scaleAfter)
    position += v2::Scale(mouseCanvasPos, scale - prevScale);

    /*
    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    if (ImGui::BeginPopup("context"))
    {
        if (adding_line)
            points.resize(points.size() - 2);
        adding_line = false;
        if (ImGui::MenuItem("Remove one", NULL, false, points.Size > 0)) { points.resize(points.size() - 2); }
        if (ImGui::MenuItem("Remove all", NULL, false, points.Size > 0)) { points.clear(); }
        ImGui::EndPopup();
    }*/

    // Draw grid + all lines in the canvas
    draw_list->PushClipRect(canvasPixelPos.ImGui(), canvasBottomRight.ImGui(), true);
    const v2 gridStep = v2::Reciprocal(scale) * 32.0f;
    for (float x = fmodf(position.x / scale.x, gridStep.x); x < canvasPixelSize.x; x += gridStep.x)
        draw_list->AddLine(ImVec2(canvasPixelPos.x + x, canvasPixelPos.y), ImVec2(canvasPixelPos.x + x, canvasBottomRight.y), IM_COL32(200, 200, 200, 40));
    for (float y = fmodf(position.y / scale.y, gridStep.x); y < canvasPixelSize.y; y += gridStep.x)
        draw_list->AddLine(ImVec2(canvasPixelPos.x, canvasPixelPos.y + y), ImVec2(canvasBottomRight.x, canvasPixelPos.y + y), IM_COL32(200, 200, 200, 40));
    draw_list->PopClipRect();

    ImGui::End();
}

v2 Canvas::ScreenToCanvas(const v2& pos) const
{
    return pos - canvasPixelPos;
}

v2 Canvas::CanvasToScreen(const v2& pos) const
{
    return pos + canvasPixelPos;
}

v2 Canvas::CanvasToPosition(const v2& pos) const
{
    return v2::Scale(pos, scale) + position;
}

v2 Canvas::PositionToCanvas(const v2& pos) const
{
    return v2::Scale(pos - position, v2::Reciprocal(pos));
}
