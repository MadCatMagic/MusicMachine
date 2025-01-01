#include "Engine/DrawList.h"
#include "imgui.h"

v4 DrawStyle::GetCol(DrawColour colour)
{
	return colours[(int)colour].col;
}

v4 ImToVec(const ImVec4& v)
{
	return v4(v.x, v.y, v.z, v.w);
}

ImColor Cv4(const v4& v)
{
	return ImColor(v.x, v.y, v.z, v.w);
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
			DrawColourSwitchThing(Text, ImToVec(ImGui::GetStyleColorVec4(ImGuiCol_Text)));
			DrawColourSwitchThing(TextFaded, v4(1.0f, 1.0f, 1.0f, 0.4f));
			DrawColourSwitchThing(TextSuperFaded, v4(1.0f, 1.0f, 1.0f, 0.1f));

			DrawColourSwitchThing(Canvas_BG, v4(50, 50, 50, 255) / 255.0f);
			DrawColourSwitchThing(Canvas_Edge, v4(255, 255, 255, 255) / 255.0f);
			DrawColourSwitchThing(Canvas_GridLinesHeavy, v4(200, 200, 200, 40) / 255.0f);
			DrawColourSwitchThing(Canvas_GridLinesLight, v4(200, 200, 200, 20) / 255.0f);

			DrawColourSwitchThing(Node_BGFill, ImToVec(ImGui::GetStyleColorVec4(ImGuiCol_WindowBg)));
			DrawColourSwitchThing(Node_BGOutline, v4(0.0f, 0.0f, 0.4f, 0.8f));
			DrawColourSwitchThing(Node_BGHeader, ImToVec(ImGui::GetStyleColorVec4(ImGuiCol_Header)));

			DrawColourSwitchThing(Node_DragSliderActive, ImToVec(ImGui::GetStyleColorVec4(ImGuiCol_Header)));
			DrawColourSwitchThing(Node_DragSliderInactive, v4(34, 53, 76, 79) / 255.0f);

			DrawColourSwitchThing(Node_IOBool, v4(1.0f, 0.2f, 0.6f));
			DrawColourSwitchThing(Node_IOFloat, v4(0.4f, 0.6f, 0.9f));
			DrawColourSwitchThing(Node_IOInt, v4(0.2f, 0.7f, 0.6f));
			DrawColourSwitchThing(Node_IOAudio, v4(1.0f, 0.7f, 0.1f));
			DrawColourSwitchThing(Node_IOSequencer, v4(0.3f, 1.0f, 0.2f));
			DrawColourSwitchThing(Node_IO, v4(0.1f, 0.1f, 0.1f));
			DrawColourSwitchThing(Node_IOSelected, v4(0.6f, 0.6f, 0.6f));

			DrawColourSwitchThing(Node_Connector, ImToVec(ImGui::GetStyleColorVec4(ImGuiCol_Text)));
			DrawColourSwitchThing(Node_ConnectorInvalid, v4(1.0f, 0.0f, 0.0f, 1.0f));

			DrawColourSwitchThing(Node_SelectedOutline, v4(0.2f, 0.6f, 1.0f, 0.8f));
			DrawColourSwitchThing(Node_TopSelectedOutline, v4(0.5f, 0.8f, 1.0f, 0.8f));
			DrawColourSwitchThing(Node_SelectedFill, v4(0.2f, 0.2f, 0.5f, 1.0f));
			DrawColourSwitchThing(Node_SelectionOutline, v4(1.0f, 0.8f, 0.2f, 0.8f));
			DrawColourSwitchThing(Node_SelectionFill, v4(1.0f, 0.8f, 0.2f, 0.2f));
		}
		colours.push_back(c);
	}
}
#pragma endregion colours

v2 DrawList::convPos(const v2& p)
{
	if (convertPosition)
		return positionCallback(p).ImGui();
	return p.ImGui();
}

void DrawList::Rect(const v2& tl, const v2& br, DrawColour col, float rounding, float thickness)
{
	Rect(tl, br, style->GetCol(col), rounding, thickness);
}

void DrawList::Rect(const v2& tl, const v2& br, const v4& col, float rounding, float thickness)
{
	dl->AddRect(
		convPos(tl).ImGui(),
		convPos(br).ImGui(),
		Cv4(col),
		rounding,
		0,
		thickness
	);
}

void DrawList::RectFilled(const v2& tl, const v2& br, DrawColour col, float rounding, int flags)
{
	RectFilled(tl, br, style->GetCol(col), rounding, flags);
}

void DrawList::RectFilled(const v2& tl, const v2& br, const v4& col, float rounding, int flags)
{
	dl->AddRectFilled(
		convPos(tl).ImGui(),
		convPos(br).ImGui(),
		Cv4(col),
		rounding,
		flags
	);
}

void DrawList::Triangle(const v2& a, const v2& b, const v2& c, DrawColour col, float thickness)
{
	Triangle(a, b, c, style->GetCol(col), thickness);
}

void DrawList::Triangle(const v2& a, const v2& b, const v2& c, const v4& col, float thickness)
{
	dl->AddTriangle(
		convPos(a).ImGui(),
		convPos(b).ImGui(),
		convPos(c).ImGui(),
		Cv4(col),
		thickness
	);
}

void DrawList::TriangleFilled(const v2& a, const v2& b, const v2& c, DrawColour col)
{
	TriangleFilled(a, b, c, style->GetCol(col));
}

void DrawList::TriangleFilled(const v2& a, const v2& b, const v2& c, const v4& col)
{
	dl->AddTriangleFilled(
		convPos(a).ImGui(),
		convPos(b).ImGui(),
		convPos(c).ImGui(),
		Cv4(col)
	);
}

void DrawList::Circle(const v2& c, float r, DrawColour col, float thickness)
{
	Circle(c, r, style->GetCol(col), thickness);
}

void DrawList::Circle(const v2& c, float r, const v4& col, float thickness)
{
	dl->AddCircle(
		convPos(c).ImGui(),
		r,
		Cv4(col),
		0,
		thickness
	);
}

void DrawList::CircleFilled(const v2& c, float r, DrawColour col)
{
	CircleFilled(c, r, style->GetCol(col));
}

void DrawList::CircleFilled(const v2& c, float r, const v4& col)
{
	dl->AddCircleFilled(
		convPos(c).ImGui(),
		r,
		Cv4(col)
	);
}

void DrawList::Text(const v2& p, DrawColour col, const char* text, const char* textEnd)
{
	dl->AddText(
		convPos(p).ImGui(),
		Cv4(style->GetCol(col)),
		text,
		textEnd
	);
}

void DrawList::Line(const v2& a, const v2& b, DrawColour col, float thickness)
{
	Line(a, b, style->GetCol(col), thickness);
}

void DrawList::Line(const v2& a, const v2& b, const v4& col, float thickness)
{
	dl->AddLine(
		convPos(a).ImGui(),
		convPos(b).ImGui(),
		Cv4(col),
		thickness
	);
}

void DrawList::Lines(const std::vector<v2>& points, const v4& col, float thickness, int flags)
{
	std::vector<ImVec2> v;
	for (const v2& p : points)
		v.push_back(convPos(p).ImGui());
	dl->AddPolyline(&v[0], (int)points.size(), Cv4(col), flags, thickness);
}

void DrawList::BezierCubic(const v2& a, const v2& b, const v2& c, const v2& d, DrawColour col, float thickness)
{
	dl->AddBezierCubic(
		convPos(a).ImGui(),
		convPos(b).ImGui(),
		convPos(c).ImGui(),
		convPos(d).ImGui(),
		Cv4(style->GetCol(col)),
		thickness
	);
}
