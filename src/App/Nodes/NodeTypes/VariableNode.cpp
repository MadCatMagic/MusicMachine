#include "App/Nodes/NodeTypes/VariableNode.h"
#include "Engine/DrawList.h"

VariableNode::~VariableNode()
{
    auto r = std::find(variableNodes.begin(), variableNodes.end(), this);
    if (r != variableNodes.end())
        variableNodes.erase(r);
}

void VariableNode::Init()
{
	name = "VariableNode";
	title = "Variable";
	minSpace = v2(100.0f, 15.0f);
	variableNodes.push_back(this);

    points.push_back(v2(0.5f, 0.8f));
    points.push_back(v2(3.4f, 0.4f));
    points.push_back(v2(5.2f, 0.7f));
    points.push_back(v2(5.3f, 0.1f));
}

void VariableNode::IO()
{
	FloatOutput("value", &output);
}

void VariableNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	dl->Text(topLeft, DrawColour::Text, id.c_str());
}

float VariableNode::getValue(float xv) const
{
    if (points.size() == 0)
        return 0.0f;

    if (xv <= points[0].x)
        return points[0].y;
    if (xv >= points[points.size() - 1].x)
        return points[points.size() - 1].y;

    for (size_t i = 0; i < points.size() - 1; i++)
    {
        if (xv >= points[i].x && xv < points[i + 1].x)
        {
            float lerpv = (xv - points[i].x) / (points[i + 1].x - points[i].x);
            return lerpv * points[i].y + (1.0f - lerpv) * points[i + 1].y;
        }
    }
    return 0.0f;
}

std::vector<VariableNode*> VariableNode::variableNodes = {};