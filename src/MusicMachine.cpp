#include "Engine.h"
#include <stdio.h>

// main function called when the program executes.
int main(int, char**)
{
    Engine engine = Engine();

    if (!engine.CreateWindow(v2i(1280, 720), "Music Machine"))
    {
        std::printf("error: something has gone terribly wrong\n");
        return 0;
    }

    engine.Mainloop();
    return 0;
}
