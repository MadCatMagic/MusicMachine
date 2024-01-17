#include "App/Node.h"
#include "App/NodeNetwork.h"

NodeClickResponse Node::HandleClick(const v2& nodePos)
{
	NodeClickResponse r;
	v2 centre = v2(getNormalWidth() - miniTriangleOffset, headerHeight * 0.5f);
	if (v2::Distance(nodePos, centre) <= 6.0f)
	{
		mini = !mini;
		r.handled = true;
		r.type = NodeClickResponseType::Minimise;
		return r;
	}
	
	v2 worldPos = nodePos + position;
	if (outputs.size() > 0)
	{
		size_t oi = 0;
		float minDist = FLT_MAX;
		for (size_t i = 0; i < outputs.size(); i++)
		{
			float d = v2::Distance(GetOutputPos(i), worldPos);
			if (d <= minDist)
			{
				oi = i;
				minDist = d;
			}
		}
		if (minDist <= 8.0f)
		{
			r.handled = true;
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
			float d = v2::Distance(GetInputPos(i), worldPos);
			if (d <= minDist)
			{
				minDist = d;
				inpI = i;
			}
		}
		if (minDist <= 8.0f)
		{
			NodeInput& inp = inputs[inpI];
			r.handled = true;
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
	}
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

void Node::Init()
{
}

void Node::IO()
{
	BoolInput("A boolean input", nullptr);
	FloatInput("floating input", nullptr);
	BoolOutput("A bool out", nullptr);
	FloatOutput("and out we go", nullptr);
}

#pragma region IOTypes
bool Node::BoolInput(const std::string& name, bool* target)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Bool;
	TransferInput(o);
	return false;
}

bool Node::BoolOutput(const std::string& name, bool* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Bool;
	TransferOutput(o);
	return false;
}

bool Node::FloatInput(const std::string& name, float* target, float min, float max)
{
	NodeInput o;
	o.name = name;
	o.target = target;
	o.type = NodeType::Float;
	o.fmin = min;
	o.fmax = max;
	TransferInput(o);
	return false;
}

bool Node::FloatOutput(const std::string& name, float* target)
{
	NodeOutput o;
	o.name = name;
	o.data = target;
	o.type = NodeType::Float;
	TransferOutput(o);
	return false;
}
#pragma endregion IOTypes

void Node::Execute()
{
	for (const NodeInput& input : inputs)
	{
		if (input.source == nullptr)
			continue;
		if (!input.source->hasBeenExecuted)
			input.source->Execute();
		size_t index = input.source->GetOutputIndex(input.sourceName);
		// need to transfer data
		if (input.target != nullptr && input.source->outputs[index].data != nullptr)
			memcpy(input.target, input.source->outputs[index].data, DataSize(input.type));
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
		maxXOff = std::max(IOWidth(input.name), maxXOff);
	for (const NodeOutput& output : outputs)
		maxXOff = std::max(IOWidth(output.name), maxXOff);
	maxXOff = std::max(minSpace.x, maxXOff);
	maxXOff = std::max(IOWidth(name) + 8.0f, maxXOff);
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

bool Node::TryConnect(Node* origin, const std::string& originName, const v2& pos, bool connectionReversed)
{
	if (connectionReversed)
	{
		v2 firstOutput = GetOutputPos(0);
		for (size_t i = 0; i < outputs.size(); i++)
			if (v2::Distance(firstOutput + v2(0.0f, 16.0f * i), pos) <= 6.0f && origin->GetInputType(originName) == outputs[i].type)
			{
				origin->Connect(origin->GetInputIndex(originName), this, i);
				return true;
			}
	}
	else
	{
		v2 firstInput = GetInputPos(0);
		for (size_t i = 0; i < inputs.size(); i++)
			if (v2::Distance(firstInput + v2(0.0f, 16.0f * i), pos) <= 6.0f && origin->GetOutputType(originName) == inputs[i].type)
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
		return position + v2(0.0f, headerHeight + 4.0f + 16.0f * outputs.size() + minSpace.y + 16.0f * index + 8.0f);
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

void Node::Draw(NodeNetwork* network, bool cullBody)
{
	v2 cursor = position;
	UpdateDimensions();
	if (!cullBody)
	{
		if (!mini)
		{
			// leave spaces
			cursor.y += headerHeight + 4.0f;
			cursor.y += 16.0f * outputs.size();
			cursor.y += minSpace.y;

			// draw inputs
			for (const NodeInput& input : inputs)
			{
				cursor.y += 8.0f;
				network->DrawInput(cursor, input, size.x);
				cursor.y += 8.0f;
			}

			cursor = position;
			// draw node header
			network->DrawHeader(cursor, name, size.x, headerHeight, mini, getNormalWidth() - miniTriangleOffset);
			cursor.y += headerHeight + 4.0f;

			// draw outputs
			for (const NodeOutput& output : outputs)
			{
				cursor.y += 8.0f;
				network->DrawOutput(cursor, size.x, output);
				cursor.y += 8.0f;
			}
		}
		else
		{
			network->DrawHeader(cursor - v2(0.0f, (headerSize() - headerHeight) * 0.5f), name, size.x, headerSize(), mini, getNormalWidth() - miniTriangleOffset);
			for (size_t i = 0; i < inputs.size(); i++)
				network->DrawConnectionEndpoint(GetInputPos(i), network->GetCol(inputs[i].type), true, inputs[i].target == nullptr);
			for (size_t i = 0; i < outputs.size(); i++)
				network->DrawConnectionEndpoint(GetOutputPos(i), network->GetCol(outputs[i].type), true, outputs[i].data == nullptr);
		}
	}

	// draw connections
	for (const NodeInput& inp : inputs)
	{
		if (inp.source != nullptr)
		{
			network->DrawConnection(GetInputPos(inp.name), inp.source->GetOutputPos(inp.sourceName), inp.type, this, inp.source);
		}
	}
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
			headerHeight + 8.0f + minSpace.y + inputs.size() * 16.0f + outputs.size() * 16.0f
		);
}

float Node::IOWidth(const std::string& text) const
{
	// return length
	// text space + 8px padding either side
	return 6.0f * (float)(text.size() + 1) + 16.0f;
}

size_t Node::DataSize(NodeType type)
{
	switch (type)
	{
	case NodeType::Bool:
		return sizeof(bool);
	case NodeType::Float:
		return sizeof(float);
	}
	return 0;
}