#pragma once
#include "App/Nodes/Node.h"

struct VariableNode : public Node
{
	friend class Arranger;
	~VariableNode();

protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

private:
	std::vector<v2> points;
	float getValue(float xv) const;

	float output = 0.0f;
	std::string id = "new variable";

	static std::vector<VariableNode*> variableNodes;
};