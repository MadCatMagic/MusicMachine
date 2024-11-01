#include "App/Nodes/NodeNetwork.h"
#include "App/Nodes/Canvas.h"
#include <sstream>
#include <iomanip>

void NodeNetworkRenderer::Draw(DrawList* drawList, std::vector<Node*>& selected, const bbox2& screen)
{
	if (network->nodes.size() == 0)
		return;
	drawList->dl->ChannelsSplit(2 * (int)network->nodes.size());
	int currentChannel = -1;
	currentList = drawList;
	for (Node* node : network->nodes)
	{
		currentChannel += 2;
		drawList->dl->ChannelsSetCurrent(currentChannel);

		// make sure to add the correct node connections to the buffer
		node->ResetTouchedStatus();
		node->IO();
		node->CheckTouchedStatus();
		// automatically sets the size
		bool dontCullNode = screen.overlaps(node->renderer.getBounds());
		DrawNode(node, !dontCullNode);

		if (dontCullNode)
		{
			// draw node body
			bbox2 nodeBounds = node->renderer.getBounds();
			v2 topLeft = nodeBounds.a;
			v2 bottomRight = nodeBounds.b;
			float rounding = node->mini ? node->renderer.headerSize() : NODE_ROUNDING;

			bool isSelected = std::find(selected.begin(), selected.end(), node) != selected.end();
			bool isSelectedTop = selected.size() > 0 && selected[selected.size() - 1] == node;

			DrawColour outline = isSelected ? DrawColour::Node_SelectedOutline : DrawColour::Node_BGOutline;
			outline = isSelectedTop ? DrawColour::Node_TopSelectedOutline : outline;
			DrawColour fill = isSelected ? DrawColour::Node_SelectedFill : DrawColour::Node_BGFill;

			// rounded to 4 pixels - a single grid tile.
			currentChannel--;
			drawList->dl->ChannelsSetCurrent(currentChannel);
			drawList->RectFilled(
				nodeBounds.a - 1.0f,
				nodeBounds.b + 1.0f,
				outline,
				rounding / canvas->GetSF().x,
				ImDrawFlags_RoundCornersAll
			);
			drawList->RectFilled(
				topLeft,
				bottomRight,
				fill,
				rounding / canvas->GetSF().x,
				ImDrawFlags_RoundCornersAll
			);
		}
	}
	drawList->dl->ChannelsMerge();

	// draw connections on top of all the nodes
	for (const ConnectionToDraw& connection : connectionsToDraw)
		if (network->nodeDependencyInfoPersistent->problemConnectionExists && (
			connection.from == network->nodeDependencyInfoPersistent->problemConnection.first &&
			connection.to == network->nodeDependencyInfoPersistent->problemConnection.second ||
			connection.to == network->nodeDependencyInfoPersistent->problemConnection.first &&
			connection.from == network->nodeDependencyInfoPersistent->problemConnection.second))
			drawList->BezierCubic(connection.a, connection.b, connection.c, connection.d, DrawColour::Node_ConnectorInvalid, connection.thickness);
		else
			drawList->BezierCubic(connection.a, connection.b, connection.c, connection.d, connection.col, connection.thickness);
	connectionsToDraw.clear();

	// draw debugging stuff
	if (drawDebugInformation)
	{
		const float k_r = 0.2f;
		const float k_a = 0.02f;

		std::vector<NodeNetwork::AbstractNode*>& absNodes = network->nodeDependencyInfoPersistent->nodes;
		srand(10);
		std::vector<v2> positions = std::vector<v2>(absNodes.size());
		for (size_t i = 0; i < absNodes.size(); i++)
			positions[i] = v2((float)rand() / (float)RAND_MAX, (float)rand() / (float)RAND_MAX) * 50.0f - 25.0f;
		// pretty slow
		for (size_t iter = 0; iter < 20; iter++)
		{
			// repulsive force between nodes
			for (size_t i = 0; i < absNodes.size(); i++)
				for (size_t j = i + 1; j < absNodes.size(); j++)
				{
					const v2 diff = positions[j] - positions[i];
					const float dist = std::max(0.1f, diff.length());
					const v2 force = diff * (k_r / dist);
					positions[i] -= force;
					positions[j] += force;
				}

			// attractive force along edges
			// only consider node inputs so not to repeat anything
			for (size_t i = 0; i < absNodes.size(); i++)
				for (size_t j : absNodes[i]->inputs)
				{
					const v2 diff = positions[j] - positions[i];
					const float dist = std::max(0.1f, diff.length());
					const v2 force = diff * (k_a / dist);
					positions[i] -= force;
					positions[j] += force;
				}
		}

		// actually draw the damn thing
		// again only care about inputs
		drawList->convertPosition = false;
		for (size_t i = 0; i < absNodes.size(); i++)
		{
			v2 origin = positions[i] + canvas->ScreenToCanvas(-100.0f);
			for (size_t j : absNodes[i]->inputs)
			{
				v2 endpoint = positions[j] + canvas->ScreenToCanvas(-100.0f);
				ImColor col = ImColor(1.0f, 0.0f, 1.0f);
				if (std::find(absNodes[j]->inputs.begin(), absNodes[j]->inputs.end(), i) != absNodes[j]->inputs.end())
					col = ImColor(0.0f, 1.0f, 1.0f);
				if (
					network->nodeDependencyInfoPersistent->problemConnectionExists &&
					(j == network->nodeDependencyInfoPersistent->problemConnection.first &&
						i == network->nodeDependencyInfoPersistent->problemConnection.second ||
						i == network->nodeDependencyInfoPersistent->problemConnection.first &&
						j == network->nodeDependencyInfoPersistent->problemConnection.second)
					)
					col = ImColor(1.0f, 1.0f, 0.0f);
				drawList->Line(origin, endpoint, col, 2.0f);
			}
		}
		for (size_t i = 0; i < absNodes.size(); i++)
		{
			v2 origin = positions[i] + canvas->ScreenToCanvas(-100.0f);
			DrawColour col = absNodes[i]->isEndpoint ? DrawColour::Node_IOBool : DrawColour::Text;
			drawList->CircleFilled(origin, 4.0f, col);
		}
		drawList->convertPosition = true;
	}
}

void NodeNetworkRenderer::DrawNode(Node* node, bool cullBody)
{
	v2 cursor = node->position;
	NodeRenderer& r = node->renderer;
	r.UpdateDimensions();
	if (!cullBody)
	{
		if (!node->mini)
		{
			// leave spaces
			cursor.y += r.headerHeight + 4.0f;
			cursor.y += 16.0f * node->outputs.size();

			// draw node and stuff
			// 2px padding round the minspace
			// centered horizontally too

			node->Render(cursor + v2((r.size.x - node->minSpace.x) * 0.5f, 2.0f), currentList, canvas->GetSF().x > 2.4f);

			cursor.y += node->minSpace.y + 4.0f;

			// draw inputs
			for (const Node::NodeInput& input : node->inputs)
			{
				cursor.y += 8.0f;
				DrawInput(cursor, input, r.size.x);
				cursor.y += 8.0f;
			}

			cursor = node->position;
			// draw node header
			DrawHeader(cursor, node->title, r.size.x, r.headerHeight, node->mini, r.getNormalWidth() - r.miniTriangleOffset);
			cursor.y += r.headerHeight + 4.0f;

			// draw outputs
			for (const Node::NodeOutput& output : node->outputs)
			{
				cursor.y += 8.0f;
				DrawOutput(cursor, r.size.x, output);
				cursor.y += 8.0f;
			}
		}
		else
		{
			DrawHeader(
				cursor - v2(0.0f, 
				(r.headerSize() - r.headerHeight) * 0.5f),
				node->title,
				r.size.x,
				r.headerSize(),
				node->mini,
				r.getNormalWidth() - r.miniTriangleOffset
			);
			for (size_t i = 0; i < node->inputs.size(); i++)
				DrawConnectionEndpoint(r.GetInputPos(i), GetCol(node->inputs[i].type), true, node->inputs[i].target == nullptr);
			for (size_t i = 0; i < node->outputs.size(); i++)
				DrawConnectionEndpoint(r.GetOutputPos(i), GetCol(node->outputs[i].type), true, node->outputs[i].data == nullptr);
		}
	}

	// draw connections
	for (const Node::NodeInput& inp : node->inputs)
	{
		if (inp.source != nullptr)
		{
			DrawConnection(node->GetInputPos(inp.name), inp.source->GetOutputPos(inp.sourceName), inp.type, node, inp.source);
		}
	}
}

void NodeNetworkRenderer::DrawInput(const v2& cursor, const Node::NodeInput& inp, float width)
{
	float sf = canvas->GetSF().x;
	if (inp.target != nullptr)
	{
		// interaction for floats and integers
		if ((inp.type == Node::NodeType::Float || inp.type == Node::NodeType::Int))
		{
			float value;
			if (inp.type == Node::NodeType::Float)
				value = *(float*)inp.target;
			else
				value = (float)(*(int*)inp.target);
			// drawing the box representing the value
			v2 tl = cursor + v2(0.0f, -8.0f);
			float proportion = (value - inp.fmin) / (inp.fmax - inp.fmin);
			if (proportion > 0.0f)
			{
				v2 br = cursor + v2(
					std::min(std::max(proportion, 0.0f), 1.0f) * width,
					8.0f
				);
				currentList->RectFilled(tl, br, (inp.source == nullptr) ? DrawColour::Node_DragSliderActive : DrawColour::Node_DragSliderInactive, 4.0f / canvas->GetSF().x);
			}
			float renderedValue = value;
			if (inp.displayType == Node::FloatDisplayType::Db)
				renderedValue = log10f(value) * 20.0f;
			else if (inp.displayType == Node::FloatDisplayType::Hz)
				renderedValue = value * 7214.4f; // experimentally obtained results

			// drawing the text displaying the value
			std::ostringstream ss;
			if (inp.displayType == Node::FloatDisplayType::Hz)
				ss << std::fixed << std::setprecision(1);
			else
				ss.precision(3);
			if (inp.type == Node::NodeType::Float)
				ss << renderedValue;
			else
				ss << *(int*)inp.target;
			if (inp.displayType == Node::FloatDisplayType::Db)
				ss << "dB";
			else if (inp.displayType == Node::FloatDisplayType::Hz)
				ss << "Hz";
			std::string convertedString = ss.str();

			// now do something completely different iff this is the case
			const std::vector<std::string> tempoSyncMap = {
				"1/64 beat",
				"1/32 beat",
				"1/16 beat",
				"1/8 beat",
				"1/4 beat",
				"1/2 beat",
				"1 beat",
				"2 beats",
				"4 beats",
				"8 beats",
				"16 beats",
				"32 beats",
				"64 beats"
			};
			if (inp.displayType == Node::FloatDisplayType::TempoSync)
				convertedString = tempoSyncMap[*(int*)inp.target + 6];

			v2 ftpos = cursor + v2(width - (convertedString.size() + 1) * 6.0f, -6.0f);
			currentList->Text(
				ftpos,
				(inp.source == nullptr) ? DrawColour::TextFaded : DrawColour::TextSuperFaded,
				convertedString.c_str()
			);
		}
		// interaction for bool inputs
		else if (inp.type == Node::NodeType::Bool)
		{
			v2 centre = cursor + v2(width - 10.0f, 0.0f);
			currentList->Circle(
				centre, 
				4.0f / sf, 
				(inp.source == nullptr) ? DrawColour::TextFaded : DrawColour::TextSuperFaded, 
				1.0f / sf
			);
			if (*(bool*)inp.target)
				currentList->CircleFilled(
					centre, 
					2.0f / sf, 
					(inp.source == nullptr) ? DrawColour::TextFaded : DrawColour::TextSuperFaded
				);
		}
	}
	v2 pos = cursor + v2(8.0f, -6.0f);
	currentList->Text(pos, DrawColour::Text, inp.name.c_str());
	// input circle thingy
	pos = cursor;
	DrawConnectionEndpoint(pos, GetCol(inp.type), true, inp.target == nullptr);
}

void NodeNetworkRenderer::DrawOutput(const v2& cursor, float xOffset, const Node::NodeOutput& out)
{
	v2 pos = cursor + v2(xOffset, 0.0f);
	float sf = canvas->GetSF().x;
	// draw on right side of node
	DrawConnectionEndpoint(pos, GetCol(out.type), true, out.data == nullptr);
	// text
	pos = cursor + v2(xOffset - (out.name.size() + 1) * 6.0f, 0.0f) + v2(-8.0f, -6.0f);
	currentList->Text(pos, DrawColour::Text, out.name.c_str());
}

void NodeNetworkRenderer::DrawConnectionEndpoint(const v2& centre, DrawColour col, bool convertPosition, bool isNull)
{
	// draw it
	float sf = canvas->GetSF().x;
	currentList->convertPosition = convertPosition;
	currentList->CircleFilled(centre, 4.0f / sf, DrawColour::Node_IO);
	currentList->CircleFilled(centre, 3.0f / sf, col);
	if (isNull)
		currentList->Circle(centre, 2.0f / sf, DrawColour::Node_IO, 1.0f / sf);
	currentList->convertPosition = true;
}

void NodeNetworkRenderer::DrawHeader(const v2& cursor, const std::string& name, float width, float height, bool mini, float miniTriOffset)
{
	v2 topLeft = cursor + 1.0f;
	v2 bottomRight = cursor + v2(width, height) - 1.0f;
	v2 textPos = v2(cursor.x + 8.0f, cursor.y + height * 0.5f - 6.0f);
	ImDrawFlags flags = ImDrawFlags_RoundCornersAll;
	float rounding = mini ? height * 0.5f : NODE_ROUNDING;
	currentList->RectFilled(topLeft, bottomRight, DrawColour::Node_BGHeader, rounding / canvas->GetSF().x, flags);
	currentList->Text(textPos, DrawColour::Text, name.c_str());

	// minimized triangle thing
	// position should be constant regardless of header size
	v2 triCentre = cursor + v2(miniTriOffset, height * 0.5f);
	if (mini)
		currentList->TriangleFilled(
			triCentre + v2(-3.0f, 3.0f),
			triCentre + v2(-3.0f, -3.0f),
			triCentre + v2(3.0f, 0.0f),
			DrawColour::Text
		);
	else
		currentList->TriangleFilled(
			triCentre + v2(0.0f, 3.0f),
			triCentre + v2(-3.0f, -3.0f),
			triCentre + v2(3.0f, -3.0f),
			DrawColour::Text
		);
}

void NodeNetworkRenderer::DrawConnection(const v2& target, const v2& origin, Node::NodeType type, Node* from, Node* to)
{
	size_t f = -1;
	size_t t = -1;
	if (from != nullptr && to != nullptr)
	{
		for (size_t i = 0; i < network->nodes.size(); i++)
		{
			if (network->nodes[i] == from)
				f = i;
			if (network->nodes[i] == to)
				t = i;
		}
	}
	float width = 12.0f + fabsf(target.x - origin.x) * 0.3f + fabsf(target.y - origin.y) * 0.1f;
	ConnectionToDraw c{
		origin,
		origin + v2(width, 0.0f),
		target - v2(width, 0.0f),
		target,
		GetCol(type),
		1.5f / canvas->GetSF().x,
		f,
		t
	};
	connectionsToDraw.push_back(c);
}

DrawColour NodeNetworkRenderer::GetCol(Node::NodeType type)
{
	if (type == Node::NodeType::Bool)
		return DrawColour::Node_IOBool;
	else if (type == Node::NodeType::Float)
		return DrawColour::Node_IOFloat;
	else if (type == Node::NodeType::Int)
		return DrawColour::Node_IOInt;
	else if (type == Node::NodeType::Audio)
		return DrawColour::Node_IOAudio;
	else if (type == Node::NodeType::Sequencer)
		return DrawColour::Node_IOSequencer;
	return DrawColour::Canvas_BG;
}