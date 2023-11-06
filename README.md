# MusicMachine

This is a project for my NEA for computer science. 
It will be a unique DAW designed for procedural music generation.
The concept is that instead of a conventional midi->synth->mixer approach, there will be a complex graph view that allows audio streams to pass in through various modifying tools and plugins, and allowing splitting and recombining to create all sorts of experimental effects. This will allow (hopefully) a whole different style of audio production to the more conventional DAW.

### Planned features:
- Node Graph for directing flows of audio through various plugins and tools
- Basic midi editor and tools to modify midi signals
- Saving and loading of project files and a way of exporting them as audio
- *Possibly* an arrangement tool, like a track list with ability to automate and add midi clips/audio clips (if not there will be a much simpler way of organising a track)
- *Possibly* support for VSTs

### Dependencies:
- imgui (including docking)
- glew
- GLFW
- stb headers
- libstk (synthesis toolkit)

These are all integrated with vcpkg and so will be installed automatically if you have vcpkg installed.
