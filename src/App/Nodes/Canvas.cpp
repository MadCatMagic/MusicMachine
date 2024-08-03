#include "App/Nodes/Canvas.h"
#include "App/Nodes/NodeNetwork.h"
#include "imgui.h"

#include "Engine/Console.h"
#include "BBox.h"

#include "Engine/Input.h"
#include "Engine/DrawList.h"

#include "App/App.h"

#include <filesystem>

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
void Canvas::CreateWindow(DrawStyle* drawStyle, App* appPointer)
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
    drawList.scaleFactor = scale.x;

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

    static float sliderMin = 0.0f;
    static float sliderMax = 1.0f;
    static bool sliderLockMin = false;
    static bool sliderLockMax = false;

    // handle non-left click node interactions first
    bool handledInteractionAlready = false;
    if (isActive && !selectingArea && !draggingConnection)
    {
        Node* node = nodes->GetNodeAtPosition(mousePos, selectedStack.size() > 0 ? selectedStack[selectedStack.size() - 1] : nullptr, 0);
        if (node == nullptr)
            goto nodeInteractionsEscape;

        // truly horrible code
        NodeClickInfo info;
        // handled later
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
            goto nodeInteractionsEscape;
        else if (ImGui::IsMouseClicked(ImGuiMouseButton_Right))
            info.isRight = true;
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
            info.interactionType = 1;
        else if (ImGui::IsMouseDragging(ImGuiMouseButton_Right))
        {
            info.interactionType = 1;
            info.isRight = true;
        }
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
            info.interactionType = 2;
        else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
        {
            info.interactionType = 2;
            info.isRight = true;
        }
        else
            goto nodeInteractionsEscape;
        info.pos = mousePos - node->position;
        NodeClickResponse r = node->HandleClick(info);
        if (r.handled)
            handledInteractionAlready = true;
    }
nodeInteractionsEscape:

    // Pan
    if (isActive && ImGui::IsMouseDragging(ImGuiMouseButton_Right) && !handledInteractionAlready)
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
            NodeClickInfo info;
            info.pos = mousePos - node->position;
            NodeClickResponse r = node->HandleClick(info);
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
                sliderMin = r.sliderMin;
                sliderMax = r.sliderMax;
                sliderLockMin = r.sliderLockMin;
                sliderLockMax = r.sliderLockMax;
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
                *sliderValue.i = (int)clamp(originalSliderValue + totalSliderMovement, sliderMin, sliderMax, sliderLockMin, sliderLockMax);
            else
                *sliderValue.f = clamp(originalSliderValue + totalSliderMovement, sliderMin, sliderMax, sliderLockMin, sliderLockMax);
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
    bool beginSaveAs = false;
    bool beginLoad = false;
    if (ImGui::BeginPopup("context"))
    {
        nodes->DrawContextMenu();

        // save
        if (ImGui::MenuItem("Save"))
        {
            if (currentFilepath != "")
                SaveState(currentFilepath);
            else
                beginSaveAs = true;
        }
        if (ImGui::MenuItem("Save As"))
            beginSaveAs = true;
        if (ImGui::MenuItem("Load"))
            beginLoad = true;

        ImGui::EndPopup();
    }

    SaveLoadWindows(beginSaveAs, beginLoad, appPointer);

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

void Canvas::SaveLoadWindows(bool beginSaveAs, bool beginLoad, App* appPointer)
{
    ////////////
    // SAVING //
    ////////////
    static char buf[64] = { 0 };
    if (beginSaveAs)
    {
        ImGui::OpenPopup("Save As");
        memset(buf, 0, 64);
    }

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Save As", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("name", buf, 64);

        // disable if filename is empty
        ImGui::BeginDisabled(buf[0] == '\0');
        if (ImGui::Button("Save"))
        {
            // check file does not exist, if so, complain
            std::string filepath = "networks/" + std::string(buf) + ".nn";
            if (std::filesystem::exists(filepath))
                ImGui::OpenPopup("Overwriting File");

            else
            {
                // save file
                SaveState(filepath);
                ImGui::CloseCurrentPopup();
            }
        }
        ImGui::EndDisabled();

        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel")) { ImGui::CloseCurrentPopup(); }

        bool closeBoth = false;
        if (ImGui::BeginPopupModal("Overwriting File", NULL, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::Text("There already exists a file by this name,\nare you sure you want to overwrite it?");
            if (ImGui::Button("Yes!"))
            {
                SaveState("networks/" + std::string(buf) + ".nn");
                ImGui::CloseCurrentPopup();
                closeBoth = true;
            }
            if (ImGui::Button("no..."))
                ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        if (closeBoth)
            ImGui::CloseCurrentPopup();

        ImGui::EndPopup();
    }

    /////////////
    // LOADING //
    /////////////
    static std::vector<std::string> files;
    static int selected = -1;
    if (beginLoad)
    {
        ImGui::OpenPopup("Load");
        files.clear();
        selected = -1;
        for (const auto& entry : std::filesystem::directory_iterator("networks"))
        {
            std::string fp = entry.path().string().substr(9);
            if (fp.size() < 3 || fp.substr(fp.size() - 3, 3) != ".nn")
            {
                Console::LogWarn("Odd filename found in /networks/: " + fp);
                continue;
            }
            files.push_back(fp.substr(0, fp.size() - 3));
        }
    }

    if (ImGui::BeginPopupModal("Load", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::BeginTable("3ways", 1, ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody))
        {
            // The first column will use the default _WidthStretch when ScrollX is Off and _WidthFixed when ScrollX is On
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_NoHide | ImGuiTableColumnFlags_WidthFixed, 300.0f);
            ImGui::TableHeadersRow();

            for (int i = 0; i < files.size(); i++)
            {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_Bullet | ImGuiTreeNodeFlags_NoTreePushOnOpen;
                if (selected == i)
                    flags |= ImGuiTreeNodeFlags_Selected;
                ImGui::TreeNodeEx((void*)(intptr_t)i, flags, files[i].c_str(), i);
                if (ImGui::IsItemClicked())
                {
                    if (selected == i)
                        selected = -1;
                    else
                        selected = i;
                }
            }

            ImGui::EndTable();
        }

        ImGui::BeginDisabled(selected == -1);
        if (ImGui::Button("Load"))
        {
            LoadState("networks/" + files[selected] + ".nn", appPointer);
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndDisabled();

        if (ImGui::Button("Cancel"))
            ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

void Canvas::SaveState(const std::string& filepath)
{
    nodes->SaveNetworkToFile(std::string(filepath));
    currentFilepath = filepath;
}

void Canvas::LoadState(const std::string& filepath, App* appPointer)
{
    if (nodes != nullptr)
        delete nodes;
    nodes = new NodeNetwork(std::string(filepath));
    if (nodeRenderer != nullptr)
        delete nodeRenderer;
    nodeRenderer = new NodeNetworkRenderer(nodes, this);
    appPointer->SetNodes(nodes);
    currentFilepath = filepath;
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
    builder.AddText("abcdefghijklmonpqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ,.<>/?;:'@#~[]{}()-_=+\\|*&^%$£\"!1234567890 ");                        // Add a string (here "Hello world" contains 7 unique characters)
    builder.BuildRanges(&ranges);

    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    for (int i = 0; i < NUM_SCALING_LEVELS; i++)
        textLODs[i] = io.Fonts->AddFontFromFileTTF("res/fonts/Cousine-Regular.ttf", 12.0f / GetSFFromScalingLevel(i), nullptr, ranges.Data);
}
