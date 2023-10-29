#pragma once

#include "graphics.h"

namespace foldscape
{
    class Application
    {
        GtkApplication* m_gtkApp;
        GtkWidget* m_mainWindow;
        GtkWidget* m_glArea;

        Graphics m_graphics;

        float m_center[2];
        float m_zoom;

        float m_dragTimeCenter[2];
        float m_cursor[2];

    private:
        void Init(const char* title, int width, int height);
        void CreateGlArea();
        void InitControl();

        void DragBegin(double x, double y);
        void DragUpdate(double x, double y);
        void DragEnd(double x, double y);
        void Scroll(double x, double y);
        void PointerMotion(double x, double y);

        void Render() const;

        void ScreenToCoord(const float screen[2], float coord[2]) const;
        void CoordToScreen(const float coord[2], float screen[2]) const;

    public:
        Application();
        ~Application();

        int Start(int argc, char* argv[]);
    };
}