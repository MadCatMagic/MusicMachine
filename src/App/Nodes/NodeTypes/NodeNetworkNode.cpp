#include "App/Nodes/NodeTypes/NodeNetworkNode.h"
#include "App/Nodes/NodeTypes/NodeNetworkVariable.h"
#include "App/Nodes/NodeNetwork.h"

#include "App/App.h"

void NodeNetworkNode::AssignNetwork(std::pair<NodeNetwork*, int> nid)
{
	if (this->network != nullptr)
		this->network->usedInNetworkNode.set(ownerID, false);
	this->network = nid.first;
	ownerID = nid.second;
	this->network->usedInNetworkNode.set(ownerID, true);
	title = network->name;
	network->RecalculateDependencies();
}

NodeNetworkNode::TypeUnion NodeNetworkNode::GetDefault(NodeType t) const
{
	if (t == NodeType::Bool)
		return false;
	else if (t == NodeType::Float)
		return 0.0f;
	else if (t == NodeType::Int)
		return 0;
	else if (t == NodeType::Audio)
		return AudioChannel();
	else
		return PitchSequencer();
}

void NodeNetworkNode::EnsureDataCorrect()
{
	size_t numInputs = 0;
	size_t numOutputs = 0;
	for (NodeNetworkVariable* v : network->ioVariables)
	{
		if (v->isOutput)
			numOutputs++;
		else
			numInputs++;
	}

	// intelligently resize and check that the types are correct
	idata.resize(numInputs);
	odata.resize(numOutputs);

	size_t ii = 0;
	size_t oi = 0;
	for (NodeNetworkVariable* v : network->ioVariables)
	{
		if (v->isOutput)
		{
			if (odata[oi].valueless_by_exception() || odata[oi].index() != (size_t)v->nodeType)
				odata[oi] = GetDefault(v->nodeType);
			oi++;
		}
		else
		{
			if (idata[ii].valueless_by_exception() || idata[ii].index() != (size_t)v->nodeType)
				idata[ii] = GetDefault(v->nodeType);
			ii++;
		}
	}
}

NodeNetworkNode::~NodeNetworkNode()
{
	if (network != nullptr)
		network->usedInNetworkNode.set(ownerID, false);
}

void NodeNetworkNode::Init()
{
	name = "NodeNetworkNode";
	title = "Network";
	minSpace = v2(40.0f, 20.0f);
}

void NodeNetworkNode::IO()
{
	if (network == nullptr)
		return;

	title = network->name;

	EnsureDataCorrect();

	size_t ii = 0;
	size_t oi = 0;
	for (size_t i = 0; i < network->ioVariables.size(); i++)
	{
		NodeNetworkVariable* v = network->ioVariables[i];
		if (v->isOutput)
		{
			DefaultOutput(
				v->id.c_str(), 
				std::get_if<bool>(&odata[oi]), 
				std::get_if<int>(&odata[oi]), 
				std::get_if<float>(&odata[oi]), 
				std::get_if<AudioChannel>(&odata[oi]), 
				std::get_if<PitchSequencer>(&odata[oi]), 
				v->nodeType
			);
			oi++;
		}
		else
		{
			DefaultInput(
				v->id.c_str(), 
				std::get_if<bool>(&idata[ii]),
				std::get_if<int>(&idata[ii]),
				std::get_if<float>(&idata[ii]),
				std::get_if<AudioChannel>(&idata[ii]),
				std::get_if<PitchSequencer>(&idata[ii]),
				v->nodeType
			);
			ii++;
		}
	}
}

void NodeNetworkNode::Work(int id)
{
	EnsureDataCorrect();

	size_t ii = 0;
	for (NodeNetworkVariable* v : network->ioVariables)
		if (!v->isOutput)
		{
			if (v->nodeType == NodeType::Audio)
				v->c.data = std::get<AudioChannel>(idata[ii]).data;
			else if (v->nodeType == NodeType::Sequencer)
				std::get<PitchSequencer>(idata[ii]).CopyTo(&v->s);
			else if (v->nodeType == NodeType::Float)
				v->f = std::get<float>(idata[ii]);
			else if (v->nodeType == NodeType::Int)
				v->i = std::get<int>(idata[ii]);
			else if (v->nodeType == NodeType::Bool)
				v->b = std::get<bool>(idata[ii]);
			ii++;
		}
	network->Execute(false, ownerID);

	size_t oi = 0;
	for (NodeNetworkVariable* v : network->ioVariables)
		if (v->isOutput)
		{
			if (v->nodeType == NodeType::Audio)
				std::get<AudioChannel>(odata[oi]).data = v->c.data;
			else if (v->nodeType == NodeType::Sequencer)
				v->s.CopyTo(&std::get<PitchSequencer>(odata[oi]));
			else if (v->nodeType == NodeType::Float)
				odata[oi] = v->f;
			else if (v->nodeType == NodeType::Int)
				odata[oi] = v->i;
			else if (v->nodeType == NodeType::Bool)
				odata[oi] = v->b;
			oi++;
		}
}

void NodeNetworkNode::Load(JSONType& data)
{
	// only for migration
	if (data.s != "")
	{
		AssignNetwork(App::instance->GetNetwork(data.s));
		return;
	}

	AssignNetwork(App::instance->GetNetwork(data.obj["title"].s));
	
	EnsureDataCorrect();

	int pi = -1;
	int ii = 0;
	for (NodeNetworkVariable* v : network->ioVariables)
		if (!v->isOutput)
		{
			pi++;
			if (v->nodeType == NodeType::Float)
				idata[ii] = (float)data.obj["inputs"].arr[pi].f;
			else if (v->nodeType == NodeType::Int)
				idata[ii] = (int)data.obj["inputs"].arr[pi].i;
			else if (v->nodeType == NodeType::Bool)
				idata[ii] = data.obj["inputs"].arr[pi].b;
			else
				pi--;
			ii++;
		}
}

JSONType NodeNetworkNode::Save()
{
	std::vector<JSONType> inputValues;
	int ii = 0;
	for (NodeNetworkVariable* v : network->ioVariables)
		if (!v->isOutput)
		{
			if (v->nodeType == NodeType::Float)
				inputValues.push_back((double)std::get<float>(idata[ii]));
			else if (v->nodeType == NodeType::Int)
				inputValues.push_back((long)std::get<int>(idata[ii]));
			else if (v->nodeType == NodeType::Bool)
				inputValues.push_back(std::get<bool>(idata[ii]));
			ii++;
		}

	return JSONType({
		{ "title", title },
		{ "inputs", inputValues }
	});
}
