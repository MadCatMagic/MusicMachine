#include "App/Nodes/NodeTypes/VariableNode.h"
#include "App/Arranger.h"
#include "Engine/DrawList.h"

VariableNode::~VariableNode()
{
    auto r = std::find(variableNodes.begin(), variableNodes.end(), this);
    if (r != variableNodes.end())
        variableNodes.erase(r);
}

void VariableNode::FlagForDeletion(int index)
{
    // linear insertion
    // slow but whatever fast enough for now
    if (toDelete.size() == 0)
    {
        toDelete.push_back(index);
        return;
    }

    for (size_t i = 0; i < toDelete.size(); i++)
        if (toDelete[i] < index)
        {
            toDelete.insert(toDelete.begin() + i, index);
            return;
        }
}

void VariableNode::DeleteFlaggedPoints()
{
    // to delete should be sorted reverse-alphabetically
    for (int index : toDelete)
        points.erase(points.begin() + index);
    toDelete.clear();
}

void VariableNode::Init()
{
	name = "VariableNode";
	title = "Variable";
	minSpace = v2(100.0f, 30.0f);
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
    dl->Text(topLeft + v2(0.0f, 15.0f), DrawColour::Text, std::to_string(getValue(Arranger::instance->getTime()) * (maxV - minV) + minV).c_str());
}

void VariableNode::Load(JSONType& data)
{
    points.clear();
    for (auto& p : data.obj["points"].arr)
        points.push_back(v2((float)p.arr[0].f, (float)p.arr[1].f));
    id = data.obj["id"].s;
    minV = (float)data.obj["min"].f;
    maxV = (float)data.obj["max"].f;
}

JSONType VariableNode::Save()
{
    std::vector<JSONType> pointsVec;
    for (v2& p : points)
        pointsVec.push_back(JSONType(std::vector({ JSONType((double)p.x), JSONType((double)p.y) })));

    return JSONType({
        { "points", pointsVec },
        { "id", id },
        { "min", minV },
        { "max", minV }
    });
}

float VariableNode::getValue(float xv) const
{
    if (points.size() == 0)
        return 0.0f;

    if (xv <= points[0].x)
        return 1.0f - points[0].y;
    if (xv >= points[points.size() - 1].x)
        return 1.0f - points[points.size() - 1].y;

    for (size_t i = 0; i < points.size() - 1; i++)
    {
        if (xv >= points[i].x && xv < points[i + 1].x)
        {
            float lerpv = (xv - points[i].x) / (points[i + 1].x - points[i].x);
            return 1.0f - ((1.0f - lerpv) * points[i].y + lerpv * points[i + 1].y);
        }
    }
    return 0.0f;
}

std::vector<VariableNode*> VariableNode::variableNodes = {};