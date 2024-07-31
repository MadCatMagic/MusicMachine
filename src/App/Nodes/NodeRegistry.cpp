#include "App/Nodes/NodeRegistry.h"

#include "App/Nodes/NodeFactory.h"

#include "App/Nodes/NodeTypes/ADSRNode.h"
#include "App/Nodes/NodeTypes/ApproxLFO.h"
#include "App/Nodes/NodeTypes/AudioFilter.h"
#include "App/Nodes/NodeTypes/AudioOutputNode.h"
#include "App/Nodes/NodeTypes/AudioTransformer.h"
#include "App/Nodes/NodeTypes/DelayNode.h"
#include "App/Nodes/NodeTypes/Distortion.h"
#include "App/Nodes/NodeTypes/MathsNode.h"
#include "App/Nodes/NodeTypes/NoiseNode.h"
#include "App/Nodes/NodeTypes/SequencerNode.h"
#include "App/Nodes/NodeTypes/WaveformGenerator.h"

void RegisterNodes()
{
    GetNodeFactory().Register("ADSRNode", "ADSR", NodeBuilder<ADSRNode>);
    GetNodeFactory().Register("ApproxLFO", "Approx LFO", NodeBuilder<ApproxLFO>);
    GetNodeFactory().Register("AudioFilter", "Filter", NodeBuilder<AudioFilter>);
    GetNodeFactory().Register("AudioOutputNode", "Audio Output Node", NodeBuilder<AudioOutputNode>);
    GetNodeFactory().Register("AudioTransformer", "Audio Transformer", NodeBuilder<AudioTransformer>);
    GetNodeFactory().Register("DelayNode", "Delay", NodeBuilder<DelayNode>);
    GetNodeFactory().Register("Distortion", "Distortion", NodeBuilder<Distortion>);
    GetNodeFactory().Register("MathsNode", "Maths", NodeBuilder<MathsNode>);
    GetNodeFactory().Register("NoiseNode", "Noise", NodeBuilder<NoiseNode>);
    GetNodeFactory().Register("SequencerNode", "Sequencer Node", NodeBuilder<SequencerNode>);
    GetNodeFactory().Register("WaveformGenerator",  "Waveform Generator",   NodeBuilder<WaveformGenerator>);
}