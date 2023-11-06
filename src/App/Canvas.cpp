#include "App/Canvas.h"

Canvas::Canvas(const v2& screensize)
    : screenSize(screensize)
{
}

v2 Canvas::CanvasToScreen(const v2& pos) const
{
    // first translate it to 0,0 and then scale to fit the screen
    v2 translated = pos - position;
    v2 scaled = v2::Scale(translated, scale);
    return v2(); // todo
}

v2 Canvas::ScreenToCanvas(const v2& pos) const
{
    return v2();
}
