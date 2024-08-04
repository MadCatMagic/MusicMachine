#include "App/Nodes/Node.h"
#include "App/Nodes/NodeNetwork.h"
#include "App/Nodes/NodeFactory.h"
#include "Engine/Console.h"

NodeClickResponse Node::HandleClick(const NodeClickInfo& info)
{
	NodeClickResponse r;
	r.handled = true;
	v2 centre = v2(getNormalWidth() - miniTriangleOffset, headerHeight * 0.5f);

	// only care about left clicks for this whole section
	if (info.interactionType == 0 && !info.isRight)
	{
		// handle the minimise button
		if (info.pos.distanceTo(centre) <= 6.0f)
		{
			mini = !mini;
			r.type = NodeClickResponseType::Interact;
			return r;
		}

		// handle outputs and inputs interactions
		v2 worldPos = info.pos + position;
		if (outputs.size() > 0)
		{
			size_t oi = 0;
			float minDist = FLT_MAX;
			for (size_t i = 0; i < outputs.size(); i++)
			{
				float d = GetOutputPos(i).distanceTo(worldPos);
				if (d <= minDist)
				{
					oi = i;
					minDist = d;
				}
			}
			if (minDist <= 8.0f)
			{
				r.type = NodeClickResponseType::BeginConnection;
				r.originName = outputs[oi].name;
				r.origin = this;
				return r;
			}
		}

		if (inputs.size() > 0)
		{
			float minDist = FLT_MAX;
			size_t inpI = 0;
			for (size_t i = 0; i < inputs.size(); i++)
			{
				float d = GetInputPos(i).distanceTo(worldPos);
				if (d <= minDist)
				{
					minDist = d;
					inpI = i;
				}
			}
			if (minDist <= 8.0f)
			{
				NodeInput& inp = inputs[inpI];
				if (inp.source != nullptr)
				{
					r.type = NodeClickResponseType::BeginConnection;
					r.originName = inp.sourceName;
					r.origin = inp.source;
					Disconnect(inpI);
				}
				else
				{
					r.type = NodeClickResponseType::BeginConnectionReversed;
					r.originName = inp.name;
					r.origin = this;
				}
				return r;
			}

			// handle the sliders
			for (size_t i = 0; i < inputs.size(); i++)
			{
				if (inputs[i].target == nullptr || inputs[i].source != nullptr)
					continue;
				v2 p = GetInputPos(i);
				if (inputs[i].type == NodeType::Int || inputs[i].type == NodeType::Float)
				{
					bbox2 bb = bbox2(p - v2(0.0f, 8.0f), p + v2(size.y, 8.0f));
					if (bb.contains(worldPos))
					{
						if (inputs[i].type == NodeType::Float)
						{
							r.type = NodeClickResponseType::InteractWithFloatSlider;
							r.sliderValue.f = (float*)inputs[i].target;
							r.sliderDelta = (inputs[i].fmax - inputs[i].fmin) / size.x;
						}
						else
						{
							r.type = NodeClickResponseType::InteractWithIntSlider;
							r.sliderValue.i = (int*)inputs[i].target;
							r.sliderDelta = (inputs[i].fmax - inputs[i].fmin) / size.x;
						}
						r.sliderLockMin = inputs[i].lockMin;
						r.sliderLockMax = inputs[i].lockMax;
						r.sliderMin = inputs[i].fmin;
						r.sliderMax = inputs[i].fmax;
						//if (inputs[i].lock)
						//	r.sliderDelta = r.sliderDelta < 0.0f ? 0.0f : (r.sliderDelta > 1.0f ? 1.0f : r.sliderDelta);
						return r;
					}
				}
				else if (inputs[i].type == NodeType::Bool)
				{
					v2 centre = p + v2(size.x - 10.0f, 0.0f);
					if ((centre - worldPos).length() > 6.0f)
						continue;
					*(bool*)inputs[i].target = !(*(bool*)inputs[i].target);
					r.type = NodeClickResponseType::InteractWithBool;
					return r;
				}
			}
		}
	}

	// handle node interactions itself
	NodeClickInfo localNodeInfo;
	localNodeInfo.interactionType = info.interactionType;
	localNodeInfo.isRight = info.isRight;
	localNodeInfo.pos = info.pos - spaceOffset();
	if (!mini && (info.pos - spaceOffset()).inBox(v2(), minSpace) && OnClick(localNodeInfo))
	{
		r.handled = true;
		r.type = NodeClickResponseType::Interact;
		return r;
	}

	r.handled = false;
	return r;
}

bool Node::Connect(size_t inputIndex, Node* origin, size_t originIndex)
{
	if (inputs.size() <= inputIndex || origin->outputs.size() <= originIndex)
		return false;
	if (inputs[inputIndex].source != nullptr)
		Disconnect(inputIndex);
	inputs[inputIndex].source = origin;
	inputs[inputIndex].sourceName = origin->outputs[originIndex].name;
	inputs[inputIndex].target = origin->outputs[originIndex].data;
	origin->outputs[originIndex].connections++;
	parent->RecalculateDependencies();
	return true;
}

void Node::Disconnect(size_t inputIndex)
{
	if (inputs.size() <= inputIndex)
		return;
	NodeInput& i = inputs[inputIndex];
	for (size_t k = 0; k < i.source->outputs.size(); k++)
		if (i.source->outputs[k].name == i.sourceName)
		{
			i.source->outputs[k].connections--;
			break;
		}
	i.source = nullptr;
	i.sourceName = "";
	i.target = nullptr;
	parent->RecalculateDependencies();
}

void Node::IO()
{
	BoolInput("A boolean input", nullptr);
	FloatInput("floating input", nullptr);
	BoolOutput("A bool out", nullptr);
	FloatOutput("and out we go", nullptr);
}

#pragma region IOTypes
void Node::BoolInput(const std::string& name, bool* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Bool;
	TransferInput(o);
}

void Node::BoolOutput(const std::string& name, bool* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Bool;
	TransferOutput(o);
}

void Node::FloatInput(const std::string& name, float* target, float min, float max, bool lockMinToRange, bool lockMaxToRange, FloatDisplayType displayType)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Float;
	o.fmin = min;
	o.fmax = max;
	o.lockMin = lockMinToRange;
	o.lockMax = lockMaxToRange;
	o.displayType = displayType;
	TransferInput(o);
}

void Node::FloatOutput(const std::string& name, float* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Float;
	TransferOutput(o);
}

void Node::IntInput(const std::string& name, int* target, int min, int max, bool lockMinToRange, bool lockMaxToRange)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Int;
	o.fmin = (float)min;
	o.fmax = (float)max;
	o.lockMin = lockMinToRange;
	o.lockMax = lockMaxToRange;
	TransferInput(o);
}

void Node::IntOutput(const std::string& name, int* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Int;
	TransferOutput(o);
}

void Node::AudioInput(const std::string& name, AudioChannel* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Audio;
	TransferInput(o);
}

void Node::AudioOutput(const std::string& name, AudioChannel* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Audio;
	TransferOutput(o);
}

void Node::SequencerInput(const std::string& name, PitchSequencer* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Sequencer;
	TransferInput(o);
}

void Node::SequencerOutput(const std::string& name, PitchSequencer* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Sequencer;
	TransferOutput(o);
}


#pragma endregion IOTypes

#include <sstream>
#include <iomanip>
std::string Node::id_s()
{
	std::stringstream stream;
	stream
		<< std::setfill('0') << std::setw(sizeof(uint64_t) * 2)
		<< std::hex << id;
	return stream.str() + "_" + name;
}

void Node::Execute()
{
	hasBeenExecuted = true;
	for (NodeInput& input : inputs)
	{
		if (input.source == nullptr)
			continue;
		if (!input.source->hasBeenExecuted)
			input.source->Execute();
		size_t index = input.source->GetOutputIndex(input.sourceName);
		// need to transfer data
		if (input.target != nullptr && input.source->outputs[index].data != nullptr)
		{
			if (input.type == NodeType::Audio)
			{
				auto& testingData = ((AudioChannel*)input.source->outputs[index].data)->data;
				//for (v2& pair : testingData)
				//	assert(!isnan(pair.x) && !isnan(pair.y));
				((AudioChannel*)input.target)->data = testingData;
			}
			else if (input.type == NodeType::Sequencer)
			{
				((PitchSequencer*)input.source->outputs[index].data)->CopyTo((PitchSequencer*)input.target);
			}
			else
			{
				memcpy(input.target, input.source->outputs[index].data, DataSize(input.type));
				if (input.type == NodeType::Float)
				{
					if (input.lockMin)
						(*(float*)input.target) = std::max((*(float*)input.target), input.fmin);
					if (input.lockMax)
						(*(float*)input.target) = std::min((*(float*)input.target), input.fmax);
				}
			}
		}
	}

	Work();
}

float Node::headerSize() const
{
	return std::max(headerHeight, 16.0f * (std::max(inputs.size(), outputs.size())) / (PI * 0.6f));
}

float Node::getNormalWidth() const
{
	float maxXOff = 0.0f;
	for (const NodeInput& input : inputs)
	{
		size_t additionalWidth = (input.type == NodeType::Int) ? 5 : 0;
		additionalWidth = (input.type == NodeType::Float) ? 6 : additionalWidth;
		additionalWidth = (input.displayType != FloatDisplayType::None) ? 8 : additionalWidth;
		maxXOff = std::max(IOWidth(input.name, additionalWidth), maxXOff);
	}
	for (const NodeOutput& output : outputs)
		maxXOff = std::max(IOWidth(output.name), maxXOff);
	maxXOff = std::max(minSpace.x + 4.0f, maxXOff);
	maxXOff = std::max(IOWidth(title) + 8.0f, maxXOff);
	return maxXOff;
}

bbox2 Node::getBounds() const
{
	if (!mini)
		return bbox2(position, position + size);
	v2 offset = position - v2(0.0f, headerSize() * 0.5f - headerHeight * 0.5f);
	return bbox2(offset, offset + size);
}

// O(n^2) (over a whole frame)
void Node::TransferInput(const NodeInput& i)
{
	for (size_t j = 0; j < inputs.size(); j++)
		if (inputs[j].name == i.name && inputs[j].type == i.type)
		{
			Node* source = inputs[j].source;
			std::string sourceName = inputs[j].sourceName;
			inputs[j] = i;
			inputs[j].source = source;
			inputs[j].sourceName = sourceName;
			inputs[j].touchedThisFrame = true;
			return;
		}
	inputs.push_back(i);
	inputs[inputs.size() - 1].touchedThisFrame = true;
}

void Node::TransferOutput(const NodeOutput& i)
{
	for (size_t j = 0; j < outputs.size(); j++)
		if (outputs[j].name == i.name && outputs[j].type == i.type)
		{
			size_t connections = outputs[j].connections;
			outputs[j] = i;
			outputs[j].connections = connections;
			outputs[j].touchedThisFrame = true;
			return;
		}
	outputs.push_back(i);
	outputs[outputs.size() - 1].touchedThisFrame = true;
}

void Node::ResetTouchedStatus()
{
	for (size_t i = 0; i < inputs.size(); i++)
		inputs[i].touchedThisFrame = false;
	for (size_t i = 0; i < outputs.size(); i++)
		outputs[i].touchedThisFrame = false;
}

void Node::CheckTouchedStatus()
{
	for (size_t i = 0; i < inputs.size(); i++)
		if (inputs[i].touchedThisFrame == false)
			inputs.erase(inputs.begin() + i);
	for (size_t i = 0; i < outputs.size(); i++)
		if (outputs[i].touchedThisFrame == false)
			outputs.erase(outputs.begin() + i);
}

void Node::LoadData(JSONType& data)
{
	position = v2(
		(float)data.obj["pos"].arr[0].f,
		(float)data.obj["pos"].arr[1].f
	);
	mini = data.obj["mini"].b;
	for (JSONType& jinp : data.obj["inputs"].arr)
	{
		for (size_t i = 0; i < inputs.size(); i++)
			if (inputs[i].name == jinp.obj["name"].s)
			{
				std::string src = jinp.obj["source"].s;
				if (src != "")
				{
					Node* source = parent->GetNodeFromID(src);
					if (source == nullptr)
					{
						Console::LogErr("Failed to find node from id '" + src + "'.");
						continue;
					}
					Connect(i, source, source->GetOutputIndex(jinp.obj["sourceName"].s));
				}
			}
	}

	Load(data.obj["node"]);
}

JSONType Node::SaveData()
{
	// only need to store input data as it is all reconstructed afterwards
	JSONType inputData{ JSONType::Array };

	for (const NodeInput& i : inputs)
		inputData.arr.push_back(JSONType({
			{ "name", i.name },
			{ "source", i.source == nullptr ? "" : i.source->id_s() },
			{ "sourceName", i.sourceName }
		}));

	return JSONType({
		{ "id", id_s() },
		{ "pos", std::vector<JSONType>{ (double)position.x, (double)position.y }},
		{ "mini", mini },
		{ "inputs", inputData },
		{ "name", name },
		{ "node", Save() }
	});
}

bool Node::TryConnect(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed)
{
	if (connectionReversed)
	{
		v2 firstOutput = GetOutputPos(0);
		for (size_t i = 0; i < outputs.size(); i++)
			if (pos.distanceTo(firstOutput + v2(0.0f, 16.0f * i)) <= 6.0f && origin->GetInputType(originName) == outputs[i].type)
			{
				origin->Connect(origin->GetInputIndex(originName), this, i);
				return true;
			}
	}
	else
	{
		v2 firstInput = GetInputPos(0);
		for (size_t i = 0; i < inputs.size(); i++)
			if (pos.distanceTo(firstInput + v2(0.0f, 16.0f * i)) <= 6.0f && origin->GetOutputType(originName) == inputs[i].type)
			{
				Connect(i, origin, origin->GetOutputIndex(originName));
				return true;
			}
	}

	return false;
}

v2 Node::GetInputPos(size_t index) const
{
	if (!mini)
	{
		return position + v2(0.0f, headerHeight + 4.0f + 4.0f + 16.0f * outputs.size() + minSpace.y + 16.0f * index + 8.0f);
	}
	if (inputs.size() > 1)
	{
		// fuck this shit
		// ahhh they should take up the same spacing as the most populous node type (input/output)
		// fuck
		float amountOfCircle = 0.6f;
		if (outputs.size() > 1)
			amountOfCircle *= std::min(1.0f, (float)(inputs.size() - 1) / (float)(outputs.size() - 1));
		if (inputs.size() == 2 && outputs.size() <= 2)
			amountOfCircle *= 2.0f / 3.0f;
		const float hh = headerSize() * 0.5f;
		const float angle = PI * (0.5f - amountOfCircle * 0.5f + amountOfCircle * index / (float)(inputs.size() - 1));
		const v2 offset = v2(sinf(-angle), -cosf(angle));
		const v2 constOffset = v2(hh, headerHeight * 0.5f);
		return position + constOffset + offset * hh;
	}
	return position + v2(0.0f, headerHeight * 0.5f);
}

v2 Node::GetInputPos(const std::string& name) const
{
	size_t n = GetInputIndex(name);
	if (n == -1)
		return v2::zero;
	return GetInputPos(n);
}

v2 Node::GetOutputPos(size_t index) const
{
	if (!mini)
	{
		return position + v2(size.x, headerHeight + 4.0f + 16.0f * index + 8.0f);
	}
	if (outputs.size() > 1)
	{
		// same as for inputs but with one or two changes
		float amountOfCircle = 0.6f;
		if (inputs.size() > 1)
			amountOfCircle *= std::min(1.0f, (float)(outputs.size() - 1) / (float)(inputs.size() - 1));
		if (outputs.size() == 2 && inputs.size() <= 2)
			amountOfCircle *= 2.0f / 3.0f;
		const float hh = headerSize() * 0.5f;
		const float angle = PI * (0.5f - amountOfCircle * 0.5f + amountOfCircle * index / (float)(outputs.size() - 1));
		const v2 offset = v2(sinf(angle), -cosf(angle));
		const v2 constOffset = v2(size.x - hh, headerHeight * 0.5f);
		return position + constOffset + offset * hh;
	}
	return position + v2(size.x, headerHeight * 0.5f);
}

v2 Node::spaceOffset() const
{
	return v2((size.x - minSpace.x) * 0.5f, headerHeight + 4.0f + 2.0f + 16.0f * outputs.size());
}

size_t Node::GetInputIndex(const std::string& name) const
{
	for (size_t i = 0; i < inputs.size(); i++)
		if (inputs[i].name == name)
			return i;
	return -1;
}

size_t Node::GetOutputIndex(const std::string& name) const
{
	for (size_t i = 0; i < outputs.size(); i++)
		if (outputs[i].name == name)
			return i;
	return -1;
}

v2 Node::GetOutputPos(const std::string& name) const
{
	size_t n = GetOutputIndex(name);
	if (n == -1)
		return v2::zero;
	return GetOutputPos(n);
}

Node::NodeType Node::GetInputType(const std::string& name) const
{
	size_t n = GetInputIndex(name);
	if (n != -1)
		return inputs[n].type;
	return NodeType::Bool;
}

Node::NodeType Node::GetOutputType(const std::string& name) const
{
	size_t n = GetOutputIndex(name);
	if (n != -1)
		return outputs[n].type;
	return NodeType::Bool;
}

void Node::UpdateDimensions()
{
	float maxXOff = getNormalWidth();
	if (mini)
		size = v2(
			std::max(maxXOff, headerSize() + 2.0f), 
			headerSize()
		);
	else
		size = v2(
			maxXOff, 
			headerHeight + 8.0f + (minSpace.y + 4.0f) + inputs.size() * 16.0f + outputs.size() * 16.0f
		);
}

float Node::IOWidth(const std::string& text, size_t additionalWidth) const
{
	// return length
	// text space + 8px padding either side
	return 6.0f * (float)(text.size() + additionalWidth + 1) + 16.0f;
}

size_t Node::DataSize(NodeType type)
{
	switch (type)
	{
	case NodeType::Bool:
		return sizeof(bool);
	case NodeType::Float:
		return sizeof(float);
	case NodeType::Int:
		return sizeof(int);
	}
	return 0;
}