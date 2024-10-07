#pragma once
#include "App/Nodes/Node.h"
#include <variant>

struct NodeNetworkNode : public Node
{
public:
	void AssignNetwork(std::pair<NodeNetwork*, int> nid);

protected:
	typedef std::variant<bool, float, int, AudioChannel, PitchSequencer> TypeUnion;
	TypeUnion GetDefault(NodeType t) const;
	void EnsureDataCorrect();

	~NodeNetworkNode();

	virtual void Init() override;
	virtual void IO() override;

	virtual void Work(int id) override;

	virtual void Load(JSONType& data) override;
	virtual JSONType Save() override;
	
	std::vector<TypeUnion> idata;
	std::vector<TypeUnion> odata;

	class NodeNetwork* network = nullptr;
	int ownerID = 0;
};