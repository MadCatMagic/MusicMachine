#include "App/Canvas.h"
#include "App/NodeNetwork.h"
#include "imgui.h"

#include "Engine/Console.h"
#include "BBox.h"

Canvas::~Canvas()
{
    if (nodes != nullptr)
        nodes->UnassignCanvas();
}

// a lot of this code is taken from the ImGui canvas example
void Canvas::CreateWindow()
{
    ImGui::Begin("Canvas");
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
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(canvasPixelPos.ImGui(), canvasBottomRight.ImGui(), IM_COL32(50, 50, 50, 255));
    drawList->AddRect(canvasPixelPos.ImGui(), canvasBottomRight.ImGui(), IM_COL32(255, 255, 255, 255));

    // This will catch our interactions
    ImGui::InvisibleButton("canvas", canvasPixelSize.ImGui(), ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
    const bool isHovered = ImGui::IsItemHovered(); // Hovered
    const bool isActive = ImGui::IsItemActive();   // Held
    const v2 mouseCanvasPos = ScreenToCanvas((v2)io.MousePos);
    const v2 mousePos = CanvasToPosition(mouseCanvasPos);

    // always the top element is the selected item
    static std::vector<Node*> selectedStack = std::vector<Node*>();
    static bool selectingArea = false;
    static v2 selectionStart = v2::zero;

    static bool draggingConnection = false;
    static bool connectionReversed = false;
    static std::string connectionOriginName = "";
    static Node* connectionOrigin = nullptr;

    // Pan
    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        position.x -= io.MouseDelta.x * scale.x;
        position.y -= io.MouseDelta.y * scale.y;
    }
    // select thing to drag around
    else if (isActive && !selectingArea && !draggingConnection && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        Node* node = nodes->GetNodeAtPosition(mousePos, nullptr);
        if (node != nullptr)
        {
            NodeClickResponse r = node->HandleClick(mousePos - node->position);
            // dragging a node
            if (!r.handled && !selectingArea)
            {
                // put straight onto stack
                if (io.KeyShift)
                    selectedStack.clear();
                auto i = std::find(selectedStack.begin(), selectedStack.end(), node);
                if (i != selectedStack.end())
                    selectedStack.erase(i);
                selectedStack.push_back(node);
            }
            // dragging a connection
            if (r.handled && (r.type == NodeClickResponseType::BeginConnection || r.type == NodeClickResponseType::BeginConnectionReversed))
            {
                draggingConnection = true;
                connectionOrigin = r.origin;
                connectionOriginName = r.originName;
                connectionReversed = r.type == NodeClickResponseType::BeginConnectionReversed;
            }
        }
        else if (!selectingArea)
        {
            selectingArea = true;
            selectionStart = mousePos;
        }
    }

    // handle selection area
    if (selectingArea && !isActive)
    {
        selectingArea = false;
        std::vector<Node*> nodesToAdd = nodes->FindNodesInArea(selectionStart, mousePos);
        if (selectedStack.size() > 0)
        {
            Node* last = selectedStack[selectedStack.size() - 1];
            selectedStack.pop_back();
            for (Node* k : nodesToAdd)
            {
                auto i = std::find(selectedStack.begin(), selectedStack.end(), k);
                if (i != selectedStack.end())
                    selectedStack.erase(i);
                selectedStack.push_back(k);
            }
            selectedStack.push_back(last);
        }
    }

    // taken from LevelEditor\...\Editor.cpp
    if (isHovered && io.MouseWheel != 0.0f)
    {
        scalingLevel -= (int)io.MouseWheel;
        // clamp(zoomLevel, 0, 31) inclusive
        scalingLevel = scalingLevel >= 0 ? (scalingLevel < NUM_SCALING_LEVELS ? scalingLevel : NUM_SCALING_LEVELS - 1) : 0;
        // 1.1 ^ -15
        v2 prevScale = scale;
        scale = GetSFFromScalingLevel(scalingLevel);
        // position + (mousePosBefore = canvasPos * scaleBefore + position) - (mousePosAfter = canvasPos * scaleAfter + position)
        // position + canvasPos * (scaleBefore - scaleAfter)
        // somehow it has to be negative... I hate you linear algebra!!!
        position -= v2::Scale(mouseCanvasPos, prevScale - scale);
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    if (ImGui::BeginPopup("context"))
    {
        nodes->DrawContextMenu();
        ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    drawList->PushClipRect((canvasPixelPos + 1.0f).ImGui(), (canvasBottomRight - 1.0f).ImGui(), true);
    const v2 gridStep = scale.reciprocal() * 16.0f;
    const v2 gridStepSmall = scale.reciprocal() * 4.0f;
    for (float x = fmodf(-position.x / scale.x, gridStep.x); x < canvasPixelSize.x; x += gridStep.x)
    {
        drawList->AddLine(ImVec2(canvasPixelPos.x + x, canvasPixelPos.y), ImVec2(canvasPixelPos.x + x, canvasBottomRight.y), IM_COL32(200, 200, 200, 40));
        if (scalingLevel < 15)
            for (int dx = 1; dx < 4; dx++)
                drawList->AddLine(
                    ImVec2(canvasPixelPos.x + x + dx * gridStepSmall.x, canvasPixelPos.y), 
                    ImVec2(canvasPixelPos.x + x + dx * gridStepSmall.x, canvasBottomRight.y), IM_COL32(200, 200, 200, 20));
    }
    for (float y = fmodf(-position.y / scale.y, gridStep.x); y < canvasPixelSize.y; y += gridStep.x)
    {
        drawList->AddLine(ImVec2(canvasPixelPos.x, canvasPixelPos.y + y), ImVec2(canvasBottomRight.x, canvasPixelPos.y + y), IM_COL32(200, 200, 200, 40));
        if (scalingLevel < 15)
            for (int dy = 1; dy < 4; dy++)
                drawList->AddLine(
                    ImVec2(canvasPixelPos.x, canvasPixelPos.y + y + dy * gridStepSmall.y),
                    ImVec2(canvasBottomRight.x, canvasPixelPos.y + y + dy * gridStepSmall.y), IM_COL32(200, 200, 200, 20));
    }

    ImGui::PushFont(textLODs[scalingLevel]);
    nodes->Draw(drawList, this);

    // draw dragged connection
    if (draggingConnection)
    {
        if (isActive)
        {
            if (connectionReversed)
                nodes->DrawConnection(
                    connectionOrigin->GetInputPos(connectionOriginName),
                    mousePos,
                    connectionOrigin->GetInputType(connectionOriginName)
                );
            else
                nodes->DrawConnection(
                    mousePos,
                    connectionOrigin->GetOutputPos(connectionOriginName),
                    connectionOrigin->GetOutputType(connectionOriginName)
                );
        }
        else
        {
            nodes->TryEndConnection(connectionOrigin, connectionOriginName, mousePos, connectionReversed);
            connectionOrigin = nullptr;
            connectionOriginName = "";
            draggingConnection = false;
        }
    }
    nodes->ClearDrawList();
    
    // draw selection box
    if (selectingArea)
    {
        bbox2 box = bbox2(mousePos, selectionStart);
        drawList->AddRectFilled(ptcts(box.a).ImGui(), ptcts(box.b).ImGui(), nodes->GetCol(NodeNetwork::SelectionFill));
        drawList->AddRect(ptcts(box.a).ImGui(), ptcts(box.b).ImGui(), nodes->GetCol(NodeNetwork::SelectionOutline));
    }

    ImGui::PopFont();
    drawList->PopClipRect();

    ImGui::End();
}

float Canvas::GetSFFromScalingLevel(int scaling)
{
    float z = MIN_SCALE;
    for (int i = 0; i < scaling; i++)
        z *= 1.1f;
    return z;
}

v2 Canvas::ScreenToCanvas(const v2& pos) const // c = s + p
{
    return canvasPixelPos - pos;
}

v2 Canvas::CanvasToScreen(const v2& pos) const // s = c - p
{
    return canvasPixelPos - pos;
}

v2 Canvas::CanvasToPosition(const v2& pos) const // position = offset - canvas * scale
{
    return position - v2::Scale(pos, scale);
}

v2 Canvas::PositionToCanvas(const v2& pos) const // canvas = (offset - position) / scale
{
    return v2::Scale(position - pos, v2::Reciprocal(scale));
}

void Canvas::GenerateAllTextLODs()
{
    // Init
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    for (int i = 0; i < NUM_SCALING_LEVELS; i++)
        textLODs[i] = io.Fonts->AddFontFromFileTTF("res/fonts/Cousine-Regular.ttf", 12.0f / GetSFFromScalingLevel(i));
}
