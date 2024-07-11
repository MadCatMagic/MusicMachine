#include "App/Nodes/NodeRegistry.h"

#include "App/Nodes/NodeFactory.h"
#include "App/Nodes/NodeTypes.h"

#include "App/Nodes/SequencerNode.h"

void RegisterNodes()
{
    //GetNodeFactory().Register("Node", "Base Node", NodeBuilder<Node>);
    //GetNodeFactory().Register("MathsNode", "Maths Node", NodeBuilder<MathsNode>);
    GetNodeFactory().Register("SawWave", "Saw Wave", NodeBuilder<SawWave>);
    GetNodeFactory().Register("AudioOutputNode", "Audio Output Node", NodeBuilder<AudioOutputNode>);
    GetNodeFactory().Register("AudioTransformer", "Audio Transformer", NodeBuilder<AudioTransformer>);
    GetNodeFactory().Register("SequencerNode", "Sequencer Node", NodeBuilder<SequencerNode>);
    GetNodeFactory().Register("ADSRNode", "ADSR", NodeBuilder<ADSRNode>);
    GetNodeFactory().Register("Distortion", "Distortion", NodeBuilder<Distortion>);
    GetNodeFactory().Register("AudioFilter", "Filter", NodeBuilder<AudioFilter>);
    GetNodeFactory().Register("DelayNode", "Delay", NodeBuilder<DelayNode>);
}