#pragma once
#include "Vector.h"
#include <functional>

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

// wrapper around ImGui drawlist to provide easier functionality
class DrawList
{
public:
	friend class Canvas;

	void Rect(const v2& tl, const v2& br, DrawColour col, float rounding = 1.0f, float thickness = 1.0f);
	void Rect(const v2& tl, const v2& br, const v4& col, float rounding = 1.0f, float thickness = 1.0f);
	void RectFilled(const v2& tl, const v2& br, DrawColour col, float rounding = 1.0f, int flags = 0);
	void RectFilled(const v2& tl, const v2& br, const v4& col, float rounding = 1.0f, int flags = 0);

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

	void Lines(const std::vector<v2>& points, const v4& col, float thickness = 1.0f, int flags = 0);

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