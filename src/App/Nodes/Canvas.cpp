#include "App/Nodes/Canvas.h"
#include "App/Nodes/NodeNetwork.h"
#include "imgui.h"

#include "Engine/Console.h"
#include "BBox.h"

#include "Engine/Input.h"
#include "Engine/DrawList.h"

Canvas::~Canvas()
{
    if (nodeRenderer != nullptr)
        nodeRenderer->UnassignCanvas();
    if (nodes != nullptr)
        nodes->UnassignCanvas();
}

void Canvas::InitCanvas()
{
    drawList.SetConversionCallback([this](const v2& p) -> v2 { return this->ptcts(p); });
    nodes->AssignCanvas(this);
    nodeRenderer = new NodeNetworkRenderer(nodes, this);
}

// a lot of this code is taken from the ImGui canvas example
void Canvas::CreateWindow(DrawStyle* drawStyle)
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
    drawList.dl = ImGui::GetWindowDrawList();
    drawList.style = drawStyle;
    drawList.convertPosition = false;
    drawList.RectFilled(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_BG);
    drawList.Rect(canvasPixelPos, canvasBottomRight, DrawColour::Canvas_Edge);

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

    // connections
    static bool draggingConnection = false;
    static bool connectionReversed = false;
    static std::string connectionOriginName = "";
    static Node* connectionOrigin = nullptr;

    // dragging stuff
    static bool draggingNodes = false;
    static float draggingDistance = 0.0f;

    // sliders
    static bool draggingSlider = false;
    static bool sliderIsInt = false;
    static NodeClickResponse::sliderValueType sliderValue{};
    static float sliderDelta = 0.0f;
    static float totalSliderMovement = 0.0f;
    static float originalSliderValue = 0.0f;

    // Pan
    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right))
    {
        position.x -= io.MouseDelta.x * scale.x;
        position.y -= io.MouseDelta.y * scale.y;
    }
    // select thing to drag around
    else if (isActive && !selectingArea && !draggingConnection && ImGui::IsMouseClicked(ImGuiMouseButton_Left))
    {
        Node* node = nodes->GetNodeAtPosition(mousePos, selectedStack.size() > 0 ? selectedStack[selectedStack.size() - 1] : nullptr, 0);
        if (node != nullptr)
        {
            NodeClickResponse r = node->HandleClick(mousePos - node->position);
            // dragging a node
            if (!r.handled && !selectingArea)
            {
                // put straight onto stack
                auto i = std::find(selectedStack.begin(), selectedStack.end(), node);
                if (i != selectedStack.end())
                {
                    selectedStack.erase(i);
                    // start dragging nodes
                    draggingNodes = true;
                }
                else if (!io.KeyShift)
                {
                    selectedStack.clear();
                    draggingNodes = true;
                }
                selectedStack.push_back(node);
                nodes->PushNodeToTop(node);
            }
            // dragging a connection
            else if (r.handled && (r.type == NodeClickResponseType::BeginConnection || r.type == NodeClickResponseType::BeginConnectionReversed))
            {
                draggingConnection = true;
                connectionOrigin = r.origin;
                connectionOriginName = r.originName;
                connectionReversed = r.type == NodeClickResponseType::BeginConnectionReversed;
            }
            // dragging a node slider
            else if (r.handled && (r.type == NodeClickResponseType::InteractWithIntSlider || r.type == NodeClickResponseType::InteractWithFloatSlider))
            {
                draggingSlider = true;
                sliderValue = r.sliderValue;
                sliderDelta = r.sliderDelta;
                sliderIsInt = r.type == NodeClickResponseType::InteractWithIntSlider;
                totalSliderMovement = 0.0f;
                originalSliderValue = sliderIsInt ? (float)*sliderValue.i : *sliderValue.f;
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
            bool addLast = io.KeyShift;
            selectedStack.pop_back();
            if (!io.KeyShift)
            {
                selectedStack.clear();
                for (Node* k : nodesToAdd)
                {
                    if (k == last)
                    {
                        addLast = true;
                        continue;
                    }
                    selectedStack.push_back(k);
                }
            }
            else
            {
                for (Node* k : nodesToAdd)
                {
                    auto i = std::find(selectedStack.begin(), selectedStack.end(), k);
                    if (i != selectedStack.end())
                        selectedStack.erase(i);
                    selectedStack.push_back(k);
                }
            }
            if (addLast)
                selectedStack.push_back(last);
        }
        else
            for (Node* k : nodesToAdd)
                selectedStack.push_back(k);
    }
    
    // drag nodes
    if (draggingNodes)
    {
        if (!isActive)
        {
            draggingNodes = false;
            for (Node* n : selectedStack)
                n->position = v2(roundf(n->position.x * 0.125f), roundf(n->position.y * 0.125f)) * 8.0f;

            // select only the node you click on if you do not drag very much
            if (draggingDistance < 2.0f)
            {
                if (selectedStack.size() > 1)
                    selectedStack.erase(selectedStack.begin(), selectedStack.end() - 1);
                else
                {
                    selectedStack[0] = nodes->GetNodeAtPosition(mousePos, selectedStack[selectedStack.size() - 1], 1);
                    nodes->PushNodeToTop(selectedStack[0]);
                }
            }
            draggingDistance = 0.0f;
        }
        else
        {
            for (Node* n : selectedStack)
            {
                v2 diff = v2(io.MouseDelta.x * scale.x, io.MouseDelta.y * scale.y);
                n->position += diff;
                draggingDistance += v2(io.MouseDelta.x).length();
            }
        }
    }

    // dragging sliders
    if (draggingSlider)
    {
        if (!isActive)
        {
            draggingSlider = false;
        }
        else
        {
            totalSliderMovement += sliderDelta * io.MouseDelta.x * scale.x * (io.KeyShift ? 0.2f : 1.0f);
            if (sliderIsInt)
                *sliderValue.i = (int)originalSliderValue + (int)totalSliderMovement;
            else
                *sliderValue.f = originalSliderValue + totalSliderMovement;
        }
    }

    // delete things
    if (Input::GetKeyDown(Input::Key::DELETE) && selectedStack.size() > 0)
    {
        for (Node* n : selectedStack)
            nodes->DeleteNode(n);
        selectedStack.clear();

        draggingNodes = false;
        draggingDistance = 0.0f;
    }

    // escape all seelctions
    if (Input::GetKeyDown(Input::Key::ESCAPE) && selectedStack.size() > 0)
        selectedStack.clear();

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
        position -= mouseCanvasPos.scale(prevScale - scale);
    }

    // Context menu (under default mouse threshold)
    ImVec2 drag_delta = ImGui::GetMouseDragDelta(ImGuiMouseButton_Right);
    if (drag_delta.x == 0.0f && drag_delta.y == 0.0f)
        ImGui::OpenPopupOnItemClick("context", ImGuiPopupFlags_MouseButtonRight);
    bool drawSavePopup = false;
    bool drawLoadPopup = false;
    if (ImGui::BeginPopup("context"))
    {
        nodes->DrawContextMenu();

        if (ImGui::MenuItem("Save Network"))
            drawSavePopup = true;
        if (ImGui::MenuItem("Load Network"))
            drawLoadPopup = true;

        ImGui::EndPopup();
    }

    if (drawSavePopup)
        ImGui::OpenPopup("Save Me!");
    if (drawLoadPopup)
        ImGui::OpenPopup("Load Me!");

    if (ImGui::BeginPopupModal("Save Me!"))
    {
        static char filename[64] = {};
        ImGui::InputText("network name", filename, sizeof(char) * 64);

        if (ImGui::Button("Save"))
        {
            nodes->SaveNetworkToFile("networks/" + std::string(filename) + ".nn");
            memset(filename, 0, 64);
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
    if (ImGui::BeginPopupModal("Load Me!"))
    {
        static char filename[64] = {};
        ImGui::InputText("network name", filename, sizeof(char) * 64);

        if (ImGui::Button("Load"))
        {
            if (nodes != nullptr)
                delete nodes;
            nodes = new NodeNetwork("networks/" + std::string(filename) + ".nn");
            if (nodeRenderer != nullptr)
                delete nodeRenderer;
            nodeRenderer = new NodeNetworkRenderer(nodes, this);
            memset(filename, 0, 64);
            ImGui::CloseCurrentPopup();
        }

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }

    // Draw grid + all lines in the canvas
    drawList.dl->PushClipRect((canvasPixelPos + 1.0f).ImGui(), (canvasBottomRight - 1.0f).ImGui(), true);
    const v2 gridStep = scale.reciprocal() * 32.0f;
    const v2 gridStepSmall = scale.reciprocal() * 8.0f;
    for (float x = fmodf(-position.x / scale.x, gridStep.x) - gridStep.x; x < canvasPixelSize.x; x += gridStep.x)
    {
        drawList.Line(v2(canvasPixelPos.x + x, canvasPixelPos.y), v2(canvasPixelPos.x + x, canvasBottomRight.y), DrawColour::Canvas_GridLinesHeavy);
        if (scalingLevel < 21)
            for (int dx = 1; dx < 4; dx++)
                drawList.Line(
                    ImVec2(canvasPixelPos.x + x + dx * gridStepSmall.x, canvasPixelPos.y), 
                    ImVec2(canvasPixelPos.x + x + dx * gridStepSmall.x, canvasBottomRight.y), DrawColour::Canvas_GridLinesLight);
    }
    for (float y = fmodf(-position.y / scale.y, gridStep.y) - gridStep.y; y < canvasPixelSize.y; y += gridStep.y)
    {
        drawList.Line(v2(canvasPixelPos.x, canvasPixelPos.y + y), v2(canvasBottomRight.x, canvasPixelPos.y + y), DrawColour::Canvas_GridLinesHeavy);
        if (scalingLevel < 21)
            for (int dy = 1; dy < 4; dy++)
                drawList.Line(
                    v2(canvasPixelPos.x, canvasPixelPos.y + y + dy * gridStepSmall.y),
                    v2(canvasBottomRight.x, canvasPixelPos.y + y + dy * gridStepSmall.y), DrawColour::Canvas_GridLinesLight);
    }

    ImGui::PushFont(textLODs[scalingLevel]);
    drawList.convertPosition = true;
    nodes->Update();
    nodeRenderer->drawDebugInformation = nodes->doIDrawDebug();
    nodeRenderer->Draw(&drawList, selectedStack, bbox2(stctp(canvasPixelPos), stctp(canvasBottomRight)));

    // draw dragged connection
    if (draggingConnection)
    {
        if (isActive)
        {
            if (connectionReversed)
                nodeRenderer->DrawConnection(
                    connectionOrigin->GetInputPos(connectionOriginName),
                    mousePos,
                    connectionOrigin->GetInputType(connectionOriginName),
                    nullptr,
                    nullptr
                );
            else
                nodeRenderer->DrawConnection(
                    mousePos,
                    connectionOrigin->GetOutputPos(connectionOriginName),
                    connectionOrigin->GetOutputType(connectionOriginName),
                    nullptr,
                    nullptr
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
    
    // draw selection box
    if (selectingArea)
    {
        bbox2 box = bbox2(mousePos, selectionStart);
        drawList.RectFilled(box.a, box.b, DrawColour::Node_SelectionFill);
        drawList.Rect(box.a, box.b, DrawColour::Node_SelectionOutline);
    }

    ImGui::PopFont();
    drawList.dl->PopClipRect();

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
    return position - pos.scale(scale);
}

v2 Canvas::PositionToCanvas(const v2& pos) const // canvas = (offset - position) / scale
{
    return (position - pos).scale(scale.reciprocal());
}

void Canvas::GenerateAllTextLODs()
{
    static ImVector<ImWchar> ranges;
    ImFontGlyphRangesBuilder builder;
    builder.AddText("abcdefghijklmonpqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.<>/?;:'@#~[]{}()-_=+\\|*&^%$�\"!1234567890 ");                        // Add a string (here "Hello world" contains 7 unique characters)
    builder.BuildRanges(&ranges);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    for (int i = 0; i < NUM_SCALING_LEVELS; i++)
        textLODs[i] = io.Fonts->AddFontFromFileTTF("res/fonts/Cousine-Regular.ttf", 12.0f / GetSFFromScalingLevel(i), nullptr, ranges.Data);
}
