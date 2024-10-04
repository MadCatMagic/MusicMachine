#include "App/Nodes/Node.h"

float NodeRenderer::headerSize() const
{
	return std::max(headerHeight, 16.0f * (std::max(node->inputs.size(), node->outputs.size())) / (PI * 0.6f));
}

float NodeRenderer::getNormalWidth() const
{
	float maxXOff = 0.0f;
	for (const Node::NodeInput& input : node->inputs)
	{
		size_t additionalWidth = (input.type == Node::NodeType::Int) ? 5 : 0;
		additionalWidth = (input.type == Node::NodeType::Float) ? 6 : additionalWidth;
		additionalWidth = (input.displayType != Node::FloatDisplayType::None) ? 8 : additionalWidth;
		maxXOff = std::max(IOWidth(input.name, additionalWidth), maxXOff);
	}
	for (const Node::NodeOutput& output : node->outputs)
		maxXOff = std::max(IOWidth(output.name), maxXOff);
	maxXOff = std::max(node->minSpace.x + 4.0f, maxXOff);
	maxXOff = std::max(IOWidth(node->title) + 8.0f, maxXOff);
	return maxXOff;
}

bbox2 NodeRenderer::getBounds() const
{
	if (!node->mini)
		return bbox2(node->position, node->position + size);
	v2 offset = node->position - v2(0.0f, headerSize() * 0.5f - headerHeight * 0.5f);
	return bbox2(offset, offset + size);
}

v2 NodeRenderer::GetInputPos(size_t index) const
{
	if (!node->mini)
	{
		return node->position + v2(0.0f, headerHeight + 4.0f + 4.0f + 16.0f * node->outputs.size() + node->minSpace.y + 16.0f * index + 8.0f);
	}
	if (node->inputs.size() > 1)
	{
		// fuck this shit
		// ahhh they should take up the same spacing as the most populous node type (input/output)
		// fuck
		float amountOfCircle = 0.6f;
		if (node->outputs.size() > 1)
			amountOfCircle *= std::min(1.0f, (float)(node->inputs.size() - 1) / (float)(node->outputs.size() - 1));
		if (node->inputs.size() == 2 && node->outputs.size() <= 2)
			amountOfCircle *= 2.0f / 3.0f;
		const float hh = headerSize() * 0.5f;
		const float angle = PI * (0.5f - amountOfCircle * 0.5f + amountOfCircle * index / (float)(node->inputs.size() - 1));
		const v2 offset = v2(sinf(-angle), -cosf(angle));
		const v2 constOffset = v2(hh, headerHeight * 0.5f);
		return node->position + constOffset + offset * hh;
	}
	return node->position + v2(0.0f, headerHeight * 0.5f);
}

v2 NodeRenderer::GetOutputPos(size_t index) const
{
	if (!node->mini)
	{
		return node->position + v2(size.x, headerHeight + 4.0f + 16.0f * index + 8.0f);
	}
	if (node->outputs.size() > 1)
	{
		// same as for inputs but with one or two changes
		float amountOfCircle = 0.6f;
		if (node->inputs.size() > 1)
			amountOfCircle *= std::min(1.0f, (float)(node->outputs.size() - 1) / (float)(node->inputs.size() - 1));
		if (node->outputs.size() == 2 && node->inputs.size() <= 2)
			amountOfCircle *= 2.0f / 3.0f;
		const float hh = headerSize() * 0.5f;
		const float angle = PI * (0.5f - amountOfCircle * 0.5f + amountOfCircle * index / (float)(node->outputs.size() - 1));
		const v2 offset = v2(sinf(angle), -cosf(angle));
		const v2 constOffset = v2(size.x - hh, headerHeight * 0.5f);
		return node->position + constOffset + offset * hh;
	}
	return node->position + v2(size.x, headerHeight * 0.5f);
}

v2 NodeRenderer::spaceOffset() const
{
	return v2((size.x - node->minSpace.x) * 0.5f, headerHeight + 4.0f + 2.0f + 16.0f * node->outputs.size());
}

void NodeRenderer::UpdateDimensions()
{
	float maxXOff = getNormalWidth();
	if (node->mini)
		size = v2(
			std::max(maxXOff, headerSize() + 2.0f),
			headerSize()
		);
	else
		size = v2(
			maxXOff,
			headerHeight + 8.0f + (node->minSpace.y + 4.0f) + node->inputs.size() * 16.0f + node->outputs.size() * 16.0f
		);
}

float NodeRenderer::IOWidth(const std::string& text, size_t additionalWidth) const
{
	// return length
	// text space + 8px padding either side
	return 6.0f * (float)(text.size() + additionalWidth + 1) + 16.0f;
}