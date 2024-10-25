#include "App/Nodes/NodeRegistry.h"

#include "App/Nodes/NodeFactory.h"

#include "App/Nodes/NodeTypes/ADSRNode.h"
#include "App/Nodes/NodeTypes/AnalysisNode.h"
#include "App/Nodes/NodeTypes/ApproxLFO.h"
#include "App/Nodes/NodeTypes/AudioFilter.h"
#include "App/Nodes/NodeTypes/AudioOutputNode.h"
#include "App/Nodes/NodeTypes/AudioTransformer.h"
#include "App/Nodes/NodeTypes/DelayNode.h"
#include "App/Nodes/NodeTypes/Distortion.h"
#include "App/Nodes/NodeTypes/MathsNode.h"
#include "App/Nodes/NodeTypes/NoiseNode.h"
#include "App/Nodes/NodeTypes/Panner.h"
#include "App/Nodes/NodeTypes/PitchShifter.h"
#include "App/Nodes/NodeTypes/RoundNode.h"
#include "App/Nodes/NodeTypes/Sampler.h"
#include "App/Nodes/NodeTypes/SequencerNode.h"
#include "App/Nodes/NodeTypes/VariableNode.h"
#include "App/Nodes/NodeTypes/WaveformGenerator.h"

void RegisterNodes()
{
    // categories:
    // - Synthesis
    // - Maths
    // - Sequencing
    // - <none>
    GetNodeFactory().Register("ADSRNode", "Synthesis", "ADSR", NodeBuilder<ADSRNode>);
    GetNodeFactory().Register("AnalysisNode", "", "Analysis Node", NodeBuilder<AnalysisNode>);
    GetNodeFactory().Register("ApproxLFO", "Maths", "Approx LFO", NodeBuilder<ApproxLFO>);
    GetNodeFactory().Register("AudioFilter", "Synthesis", "Filter", NodeBuilder<AudioFilter>);
    GetNodeFactory().Register("AudioOutputNode", "", "Audio Output Node", NodeBuilder<AudioOutputNode>);
    GetNodeFactory().Register("MixNode", "Maths", "Mix", NodeBuilder<MixNode>);
    GetNodeFactory().Register("DelayNode", "Synthesis", "Delay", NodeBuilder<DelayNode>);
    GetNodeFactory().Register("Distortion", "Synthesis", "Distortion", NodeBuilder<Distortion>);
    GetNodeFactory().Register("MathsNode", "Maths", "Maths", NodeBuilder<MathsNode>);
    GetNodeFactory().Register("NoiseNode", "Synthesis", "Noise", NodeBuilder<NoiseNode>);
    GetNodeFactory().Register("Panner", "Synthesis", "Panner", NodeBuilder<Panner>);
    GetNodeFactory().Register("PitchShifter", "Sequencing", "Pitch Shifter", NodeBuilder<PitchShifter>);
    GetNodeFactory().Register("RoundNode", "Maths", "Rounder", NodeBuilder<RoundNode>);
    GetNodeFactory().Register("Sampler", "Synthesis", "Sampler", NodeBuilder<Sampler>);
    GetNodeFactory().Register("SequencerNode", "Sequencing", "Sequencer Node", NodeBuilder<SequencerNode>);
    GetNodeFactory().Register("VariableNode", "", "Variable", NodeBuilder<VariableNode>);
    GetNodeFactory().Register("WaveformGenerator", "Synthesis", "Waveform Generator",   NodeBuilder<WaveformGenerator>);
}