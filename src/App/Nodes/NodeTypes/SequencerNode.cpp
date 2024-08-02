#include "Engine/DrawList.h"
#include "App/Nodes/NodeTypes/SequencerNode.h"

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
	IntInput("length", &width, 4, 32, true, true);
	IntInput("height", &height, 12, 24, true, true);
	IntInput("octave", &octaveShift, -2, 2);
}

void SequencerNode::Render(const v2& topLeft, DrawList* dl, bool lodOn)
{
	EnsureDataSize();

	// begins on A
	const bool keyboard[] = { false, true, false, false, true, false, true, false, false, true, false, true };

	// render squares
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
		{
			dl->Rect(
				topLeft + v2(i, j) * cellSize, 
				topLeft + v2(i + 1.0f, j + 1.0f) * cellSize, 
				keyboard[(height - 1 - j) % 12] ? ImColor(1.0f, (float)i / width, (float)j / height, 1.0f) : ImColor(0.0f, (float)i / width, (float)j / height, 1.0f)
			);
		}
	
	// render filled in squares
	for (int i = 0; i < width; i++)
	{
		if (data[i].first != -1)
			dl->RectFilled(topLeft + v2(i, height - 1 - data[i].first) * cellSize, topLeft + v2(i, height - 1 - data[i].first) * cellSize + cellSize, ImColor(0.7f, data[i].second, 0.2f, 1.0f));
	}

	// render velocities
	for (int i = 0; i < width; i++)
	{
		if (data[i].first != -1)
			dl->RectFilled(
				topLeft + v2(i, height) * cellSize + v2(cellSize * 0.2f, 20.0f * (1.0f - data[i].second)),
				topLeft + v2(i + 1, height) * cellSize + v2(-cellSize * 0.2f, 20.0f),
				ImColor(0.7f, data[i].second, 0.2f, 1.0f)
			);
	}

	// render play cursor
	dl->RectFilled(topLeft + v2(currentI * cellSize, 0.0f), topLeft + v2(currentI * cellSize + cellSize, cellSize * height), ImColor(1.0f, 0.2f, 0.1f, 0.4f));
}

bool SequencerNode::OnClick(const NodeClickInfo& info)
{
	if (info.interactionType == 2 || info.isRight)
		return false;

	v2i p = v2i(info.pos / cellSize);
	if (info.pos.y >= cellSize * height)
	{
		float value = 1.0f - (info.pos.y - cellSize * height) / 20.0f;
		if (data[p.x].first != -1)
			data[p.x].second = value;

		// convenience feature
		return true;
	}
	if (info.interactionType == 1)
		return false;

	p.y = height - 1 - p.y;
	if (data[p.x].first != p.y)
		data[p.x] = std::make_pair(p.y, 0.7f);
	else
		data[p.x] = std::make_pair(-1, 0.0f);
	return true;
}

void SequencerNode::Work()
{
	EnsureDataSize();

	currentI = (int)(AudioChannel::t * (bpm / 60.0f)) % width;

	float pitchLength = (float)AudioChannel::sampleRate / (bpm / 60.0f);

	seq = PitchSequencer();

	float fakeTime = AudioChannel::t * (bpm / 60.0f);

	int currLength = 0;
	int io = 0;
	while (currLength < AudioChannel::bufferSize)
	{
		float p = GetPitch((currentI + io) % width);

		// number of samples the sound should play for *THIS* buffer
		float exactI = fakeTime - (float)(int)fakeTime;
		if (io > 0)
			exactI = 0.0f;
		int t = (int)((1.0f - exactI) * pitchLength);
		int ts = t;
		t = (t + currLength) >= AudioChannel::bufferSize ? AudioChannel::bufferSize - currLength : t;

		// send pitch
		seq.pitch.push_back(p);
		seq.length.push_back(t);
		seq.velocity.push_back(data[(currentI + io) % width].second);
		// dont think this actually works :)
		seq.cumSamples.push_back(pitchLength - ts);
		currLength += t;

		io++;
	}

}

void SequencerNode::Load(JSONType& data)
{
	width = (int)data.obj["width"].i;
	height = (int)data.obj["height"].i;
	// load data
	EnsureDataSize();
	for (int i = 0; i < width; i++)
	{
		this->data[i] = {
			(int)data.obj["data"].arr[i].obj["p"].i,
			(float)data.obj["data"].arr[i].obj["v"].f
		};
	}

}

JSONType SequencerNode::Save()
{
	std::vector<JSONType> dataVec;
	for (int i = 0; i < width; i++)
	{
		dataVec.push_back(JSONType({
			{ "p", (long)data[i].first},
			{ "v", (double)data[i].second}
		}));
	}

	return JSONType({
		{ "width", (long)width },
		{ "height", (long)height },
		{ "data", dataVec }
	});
}

float SequencerNode::GetPitch(int i)
{
	const float mul = powf(2.0f, 1.0f / 12.0f);
	float b = 220.0f * powf(2.0f, (float)octaveShift);
	if (data[i].first == -1)
		return 0.0f;

	for (int j = 0; j < data[i].first; j++)
		b *= mul;

	return b;
}

void SequencerNode::EnsureDataSize()
{
	minSpace = v2(width, height) * cellSize + v2(0.0f, 20.0f);
	size_t s = data.size();

	if (s == width)
		return;

	if (s < width)
		for (int i = 0; i < width - s; i++)
			data.push_back({ -1, 0.0f });
	else
		for (int i = 0; i < s - width; i++)
			data.erase(data.end() - 1);
}