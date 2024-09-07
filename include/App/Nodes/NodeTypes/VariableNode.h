#pragma once
#include "App/Nodes/Node.h"

struct VariableNode : public Node
{
	friend class Arranger;
	~VariableNode();

	float minV = 0.0f;
	float maxV = 1.0f;

	void FlagForDeletion(int index);
	void DeleteFlaggedPoints();

protected:
	virtual void Init() override;
	virtual void IO() override;

	virtual void Render(const v2& topLeft, DrawList* dl, bool lodOn) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;

private:
	std::vector<v2> points;
	std::vector<int> toDelete;
	float getValue(float xv) const;

	float output = 0.0f;
	std::string id = "new variable";

	static std::vector<VariableNode*> variableNodes;
};