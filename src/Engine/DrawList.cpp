#include "Engine/DrawList.h"
#include "imgui.h"


ImColor DrawStyle::GetCol(DrawColour colour)
{
	return colours[(int)colour].col;
}

#pragma region colours
#define QUOTE(x) #x
#define DrawColourSwitchThing(_name, value) case DrawColour::_name: c.name = QUOTE(_name); c.col = value; break
void DrawStyle::InitColours()
{
	for (int i = 0; i < NUM_DRAW_COLOURS; i++)
	{
		ColourData c;
		switch ((DrawColour)i)
		{
			DrawColourSwitchThing(Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
			DrawColourSwitchThing(TextFaded, ImColor(1.0f, 1.0f, 1.0f, 0.4f));
			DrawColourSwitchThing(TextSuperFaded, ImColor(1.0f, 1.0f, 1.0f, 0.1f));

			DrawColourSwitchThing(Canvas_BG, IM_COL32(50, 50, 50, 255));
			DrawColourSwitchThing(Canvas_Edge, IM_COL32(255, 255, 255, 255));
			DrawColourSwitchThing(Canvas_GridLinesHeavy, IM_COL32(200, 200, 200, 40));
			DrawColourSwitchThing(Canvas_GridLinesLight, IM_COL32(200, 200, 200, 20));

			DrawColourSwitchThing(Node_BGFill, ImGui::GetStyleColorVec4(ImGuiCol_WindowBg));
			DrawColourSwitchThing(Node_BGOutline, ImColor(0.0f, 0.0f, 0.4f, 0.8f));
			DrawColourSwitchThing(Node_BGHeader, ImGui::GetStyleColorVec4(ImGuiCol_Header));

			DrawColourSwitchThing(Node_DragSliderActive, ImGui::GetStyleColorVec4(ImGuiCol_Header));
			DrawColourSwitchThing(Node_DragSliderInactive, IM_COL32(34, 53, 76, 79));

			DrawColourSwitchThing(Node_IOBool, ImColor(1.0f, 0.2f, 0.6f));
			DrawColourSwitchThing(Node_IOFloat, ImColor(0.4f, 0.6f, 0.9f));
			DrawColourSwitchThing(Node_IOInt, ImColor(0.2f, 0.7f, 0.6f));
			DrawColourSwitchThing(Node_IOAudio, ImColor(1.0f, 0.7f, 0.1f));
			DrawColourSwitchThing(Node_IOSequencer, ImColor(0.3f, 1.0f, 0.2f));
			DrawColourSwitchThing(Node_IO, ImColor(0.1f, 0.1f, 0.1f));
			DrawColourSwitchThing(Node_IOSelected, ImColor(0.6f, 0.6f, 0.6f));

			DrawColourSwitchThing(Node_Connector, ImGui::GetStyleColorVec4(ImGuiCol_Text));
			DrawColourSwitchThing(Node_ConnectorInvalid, ImColor(1.0f, 0.0f, 0.0f, 1.0f));

			DrawColourSwitchThing(Node_SelectedOutline, ImColor(0.2f, 0.6f, 1.0f, 0.8f));
			DrawColourSwitchThing(Node_TopSelectedOutline, ImColor(0.5f, 0.8f, 1.0f, 0.8f));
			DrawColourSwitchThing(Node_SelectedFill, ImColor(0.2f, 0.2f, 0.5f, 1.0f));
			DrawColourSwitchThing(Node_SelectionOutline, ImColor(1.0f, 0.8f, 0.2f, 0.8f));
			DrawColourSwitchThing(Node_SelectionFill, ImColor(1.0f, 0.8f, 0.2f, 0.2f));
		}
		colours.push_back(c);
	}
}
#pragma endregion colours

ImVec2 DrawList::convPos(const v2& p)
{
	if (convertPosition)
		return positionCallback(p).ImGui();
	return p.ImGui();
}

void DrawList::Rect(const v2& tl, const v2& br, DrawColour col, float rounding, float thickness)
{
	Rect(tl, br, style->GetCol(col), rounding, thickness);
}

void DrawList::Rect(const v2& tl, const v2& br, const ImColor& col, float rounding, float thickness)
{
	dl->AddRect(
		convPos(tl),
		convPos(br),
		col,
		rounding,
		0,
		thickness
	);
}

void DrawList::RectFilled(const v2& tl, const v2& br, DrawColour col, float rounding, ImDrawFlags flags)
{
	RectFilled(tl, br, style->GetCol(col), rounding, flags);
}

void DrawList::RectFilled(const v2& tl, const v2& br, const ImColor& col, float rounding, ImDrawFlags flags)
{
	dl->AddRectFilled(
		convPos(tl),
		convPos(br),
		col,
		rounding,
		flags
	);
}

void DrawList::Triangle(const v2& a, const v2& b, const v2& c, DrawColour col, float thickness)
{
	Triangle(a, b, c, style->GetCol(col), thickness);
}

void DrawList::Triangle(const v2& a, const v2& b, const v2& c, const ImColor& col, float thickness)
{
	dl->AddTriangle(
		convPos(a),
		convPos(b),
		convPos(c),
		col,
		thickness
	);
}

void DrawList::TriangleFilled(const v2& a, const v2& b, const v2& c, DrawColour col)
{
	TriangleFilled(a, b, c, style->GetCol(col));
}

void DrawList::TriangleFilled(const v2& a, const v2& b, const v2& c, const ImColor& col)
{
	dl->AddTriangleFilled(
		convPos(a),
		convPos(b),
		convPos(c),
		col
	);
}

void DrawList::Circle(const v2& c, float r, DrawColour col, float thickness)
{
	Circle(c, r, style->GetCol(col), thickness);
}

void DrawList::Circle(const v2& c, float r, const ImColor& col, float thickness)
{
	dl->AddCircle(
		convPos(c),
		r,
		col,
		0,
		thickness
	);
}

void DrawList::CircleFilled(const v2& c, float r, DrawColour col)
{
	CircleFilled(c, r, style->GetCol(col));
}

void DrawList::CircleFilled(const v2& c, float r, const ImColor& col)
{
	dl->AddCircleFilled(
		convPos(c),
		r,
		col
	);
}

void DrawList::Text(const v2& p, DrawColour col, const char* text, const char* textEnd)
{
	dl->AddText(
		convPos(p),
		style->GetCol(col),
		text,
		textEnd
	);
}

void DrawList::Line(const v2& a, const v2& b, DrawColour col, float thickness)
{
	Line(a, b, style->GetCol(col), thickness);
}

void DrawList::Line(const v2& a, const v2& b, const ImColor& col, float thickness)
{
	dl->AddLine(
		convPos(a),
		convPos(b),
		col,
		thickness
	);
}

void DrawList::Lines(const std::vector<v2>& points, const ImColor& col, float thickness, ImDrawFlags flags)
{
	std::vector<ImVec2> v;
	for (const v2& p : points)
		v.push_back(convPos(p));
	dl->AddPolyline(&v[0], (int)points.size(), col, flags, thickness);
}

void DrawList::BezierCubic(const v2& a, const v2& b, const v2& c, const v2& d, DrawColour col, float thickness)
{
	dl->AddBezierCubic(
		convPos(a),
		convPos(b),
		convPos(c),
		convPos(d),
		style->GetCol(col),
		thickness
	);
}
