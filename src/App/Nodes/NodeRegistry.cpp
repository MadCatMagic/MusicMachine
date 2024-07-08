#include "App/Nodes/NodeRegistry.h"

#include "App/Nodes/NodeFactory.h"
#include "App/Nodes/NodeTypes.h"

void RegisterNodes()
{
    GetNodeFactory().Register("Node", "Base Node", NodeBuilder<Node>);
    GetNodeFactory().Register("MathsNode", "Maths Node", NodeBuilder<MathsNode>);
    GetNodeFactory().Register("SawWave", "Saw Wave", NodeBuilder<SawWave>);
    GetNodeFactory().Register("AudioOutputNode", "Audio Output Node", NodeBuilder<AudioOutputNode>);
    GetNodeFactory().Register("AudioAdder", "Audio Adder", NodeBuilder<AudioAdder>);
    GetNodeFactory().Register("SequencerNode", "Sequencer Node", NodeBuilder<SequencerNode>);
}