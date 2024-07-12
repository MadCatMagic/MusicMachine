#include "Engine/DrawList.h"
#include "App/Nodes/SequencerNode.h"

void SequencerNode::Init()
{
	name = "SequencerNode";
	title = "Sequencer Node";

	EnsureDataSize();
}

void SequencerNode::IO()
{
	SequencerOutput("sequence", &seq);
	FloatInput("bpm", &bpm, 10.0f, 600.0f, true, true);
	IntInput("length", &horizWidth, 4, 32, true, true);
	IntInput("height", &vertWidth, 12, 24, true, true);
}

void SequencerNode::Render(const v2& topLeft, DrawList* dl)
{
	EnsureDataSize();
	// STUFF
	for (int i = 0; i < horizWidth; i++)
		for (int j = 0; j < vertWidth; j++)
			dl->Rect(topLeft + v2(i, j) * cellSize, topLeft + v2(i + 1.0f, j + 1.0f) * cellSize, ImColor(1.0f, (float)i / horizWidth, (float)j / vertWidth, 1.0f));

	for (int i = 0; i < horizWidth; i++)
	{
		if (data[i].first != -1)
			dl->RectFilled(topLeft + v2(i, vertWidth - 1 - data[i].first) * cellSize, topLeft + v2(i, vertWidth - 1 - data[i].first) * cellSize + cellSize, ImColor(0.7f, data[i].second, 0.2f, 1.0f));
	}
	dl->RectFilled(topLeft + v2(currentI * cellSize, 0.0f), topLeft + v2(currentI * cellSize + cellSize, cellSize * vertWidth), ImColor(1.0f, 0.2f, 0.1f, 0.4f));
}

bool SequencerNode::OnClick(const v2& clickPosition)
{
	v2i p = v2i(clickPosition / cellSize);
	if (p.y == vertWidth || p.x == horizWidth)
		return false;
	p.y = vertWidth - 1 - p.y;
	if (data[p.x].first != p.y)
		data[p.x] = std::make_pair(p.y, 0.7f);
	else
		data[p.x] = std::make_pair(-1, 0.0f);
	return true;
}

void SequencerNode::Work()
{
	EnsureDataSize();

	currentI = (int)(AudioChannel::t * (bpm / 60.0f)) % horizWidth;

	float pitchLength = (float)AudioChannel::sampleRate / (bpm / 60.0f);

	seq = PitchSequencer();

	float fakeTime = AudioChannel::t * (bpm / 60.0f);

	int currLength = 0;
	int io = 0;
	while (currLength < AudioChannel::bufferSize)
	{
		float p = GetPitch((currentI + io) % horizWidth);

		// number of samples the sound should play for *THIS* buffer
		float exactI = fakeTime - (float)(int)fakeTime;
		float exactIs = exactI;
		if (io > 0)
			exactI = 0.0f;
		int t = (int)((1.0f - exactI) * pitchLength);
		t = (t + currLength) >= AudioChannel::bufferSize ? AudioChannel::bufferSize - currLength : t;

		// send pitch
		seq.pitch.push_back(p);
		seq.length.push_back(t);
		seq.velocity.push_back(data[(currentI + io) % horizWidth].second);
		// dont think this actually works :)
		seq.cumSamples.push_back((int)(exactIs * AudioChannel::sampleRate));
		currLength += t;

		io++;
	}

}

void SequencerNode::Load(JSONType& data)
{
	horizWidth = (int)data.obj["width"].i;
	vertWidth = (int)data.obj["height"].i;
	// load data
	EnsureDataSize();
	for (int i = 0; i < horizWidth; i++)
	{
		this->data[i] = {
			data.obj["data"].arr[i].obj["p"].i,
			data.obj["data"].arr[i].obj["v"].f
		};
	}

}

JSONType SequencerNode::Save()
{
	std::vector<JSONType> dataVec;
	for (int i = 0; i < horizWidth; i++)
	{
		dataVec.push_back(JSONType({
			{ "p", (long)data[i].first},
			{ "v", (double)data[i].second}
		}));
	}

	return JSONType({
		{ "width", (long)horizWidth },
		{ "height", (long)vertWidth },
		{ "data", dataVec }
	});
}

float SequencerNode::GetPitch(int i)
{
	const float mul = powf(2.0f, 1.0f / 12.0f);
	float b = 220.0f;
	if (data[i].first == -1)
		return 0.0f;

	for (int j = 0; j < data[i].first; j++)
		b *= mul;

	return b;
}

void SequencerNode::EnsureDataSize()
{
	minSpace = v2(horizWidth, vertWidth) * cellSize;
	size_t s = data.size();

	if (s == horizWidth)
		return;

	if (s < horizWidth)
		for (int i = 0; i < horizWidth - s; i++)
			data.push_back({ -1, 0.0f });
	else
		for (int i = 0; i < s - horizWidth; i++)
			data.erase(data.end() - 1);
}