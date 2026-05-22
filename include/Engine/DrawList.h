#pragma once
#include "Vector.h"
#include <functional>
//#include "imgui.h"

const size_t NUM_DRAW_COLOURS = 26;
enum DrawColour {
	Text, TextFaded, TextSuperFaded,

	Canvas_BG, Canvas_Edge,
	Canvas_GridLinesHeavy, Canvas_GridLinesLight,

	Node_BGFill, Node_BGOutline, Node_BGHeader,
	Node_DragSliderActive, Node_DragSliderInactive,
	Node_IOBool, Node_IOFloat, Node_IOInt, Node_IOAudio, Node_IOSequencer,
	Node_IO, Node_IOSelected,
	Node_Connector, Node_ConnectorInvalid,
	Node_SelectedOutline, Node_TopSelectedOutline, Node_SelectedFill,
	Node_SelectionOutline, Node_SelectionFill
};

struct DrawStyle
{
	struct ColourData
	{
		std::string name;
		v4 col;
	};

	std::vector<ColourData> colours;
	void InitColours();

	v4 GetCol(DrawColour colour);
};

typedef int DLDrawFlags;
enum DLDrawFlags_
{
	DLDrawFlags_None = 0,
	DLDrawFlags_Closed = 1 << 0, // PathStroke(), AddPolyline(): specify that shape should be closed (Important: this is always == 1 for legacy reason)
	DLDrawFlags_RoundCornersTopLeft = 1 << 4, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-left corner only (when rounding > 0.0f, we default to all corners). Was 0x01.
	DLDrawFlags_RoundCornersTopRight = 1 << 5, // AddRect(), AddRectFilled(), PathRect(): enable rounding top-right corner only (when rounding > 0.0f, we default to all corners). Was 0x02.
	DLDrawFlags_RoundCornersBottomLeft = 1 << 6, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-left corner only (when rounding > 0.0f, we default to all corners). Was 0x04.
	DLDrawFlags_RoundCornersBottomRight = 1 << 7, // AddRect(), AddRectFilled(), PathRect(): enable rounding bottom-right corner only (when rounding > 0.0f, we default to all corners). Wax 0x08.
	DLDrawFlags_RoundCornersNone = 1 << 8, // AddRect(), AddRectFilled(), PathRect(): disable rounding on all corners (when rounding > 0.0f). This is NOT zero, NOT an implicit flag!
	DLDrawFlags_RoundCornersTop = DLDrawFlags_RoundCornersTopLeft | DLDrawFlags_RoundCornersTopRight,
	DLDrawFlags_RoundCornersBottom = DLDrawFlags_RoundCornersBottomLeft | DLDrawFlags_RoundCornersBottomRight,
	DLDrawFlags_RoundCornersLeft = DLDrawFlags_RoundCornersBottomLeft | DLDrawFlags_RoundCornersTopLeft,
	DLDrawFlags_RoundCornersRight = DLDrawFlags_RoundCornersBottomRight | DLDrawFlags_RoundCornersTopRight,
	DLDrawFlags_RoundCornersAll = DLDrawFlags_RoundCornersTopLeft | DLDrawFlags_RoundCornersTopRight | DLDrawFlags_RoundCornersBottomLeft | DLDrawFlags_RoundCornersBottomRight,
	DLDrawFlags_RoundCornersDefault_ = DLDrawFlags_RoundCornersAll, // Default to ALL corners if none of the _RoundCornersXX flags are specified.
	DLDrawFlags_RoundCornersMask_ = DLDrawFlags_RoundCornersAll | DLDrawFlags_RoundCornersNone,
};

// wrapper around ImGui drawlist to provide easier functionality
class DrawList
{
public:
	friend class Canvas;

	void Rect(const v2& tl, const v2& br, DrawColour col, float rounding = 1.0f, float thickness = 1.0f);
	void Rect(const v2& tl, const v2& br, const v4& col, float rounding = 1.0f, float thickness = 1.0f);
	void RectFilled(const v2& tl, const v2& br, DrawColour col, float rounding = 1.0f, DLDrawFlags flags = 0);
	void RectFilled(const v2& tl, const v2& br, const v4& col, float rounding = 1.0f, DLDrawFlags flags = 0);

	void Triangle(const v2& a, const v2& b, const v2& c, DrawColour col, float thickness = 1.0f);
	void Triangle(const v2& a, const v2& b, const v2& c, const v4& col, float thickness = 1.0f);
	void TriangleFilled(const v2& a, const v2& b, const v2& c, DrawColour col);
	void TriangleFilled(const v2& a, const v2& b, const v2& c, const v4& col);

	void Circle(const v2& c, float r, DrawColour col, float thickness = 1.0f);
	void Circle(const v2& c, float r, const v4& col, float thickness = 1.0f);
	void CircleFilled(const v2& c, float r, DrawColour col);
	void CircleFilled(const v2& c, float r, const v4& col);

	void Text(const v2& p, DrawColour col, const char* text, const char* textEnd = 0);

	void Line(const v2& a, const v2& b, DrawColour col, float thickness = 1.0f);
	void Line(const v2& a, const v2& b, const v4& col, float thickness = 1.0f);

	void Lines(const std::vector<v2>& points, const v4& col, float thickness = 1.0f, DLDrawFlags flags = 0);

	void BezierCubic(const v2& a, const v2& b, const v2& c, const v2& d, DrawColour col, float thickness = 1.0f);

	struct ImDrawList* dl = nullptr;
	DrawStyle* style = nullptr;
	bool convertPosition = true;

	float scaleFactor = 1.0f;

	inline void SetConversionCallback(std::function<v2(const v2&)> f) { positionCallback = f; }

private:

	v2 convPos(const v2& p);

	std::function<v2(const v2&)> positionCallback;
}; 