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

    private:
        void Init(const char* title, int width, int height);
        void CreateGlArea();
        void Render() const;

    public:
        Application();
        ~Application();

        int Start(int argc, char* argv[]);
    };
}