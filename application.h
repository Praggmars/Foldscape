#pragma once

#include <gtk/gtk.h>

namespace foldscape
{
    class Application
    {
        GtkApplication* m_gtkApp;
        GtkWidget* m_mainWindow;
        GtkWidget* m_glArea;
        guint m_vertexBuffer;
        guint m_positionLocation;
        guint m_program;
        
        guint m_shaderDataCenter;
        guint m_shaderDataZoom;
        guint m_shaderDataAspectRatio;

        float m_center[2];
        float m_zoom;

    private:
        void Init(const char* title, int width, int height);
        void CreateGlArea();

        void OpenGlInit();
        GError* OpenGlInitShaders();
        void OpenGlInitBuffers();
        void OpenGlDeinit();
        void OpenGlResize(int width, int height);
        void GlRender() const;

    public:
        Application();
        ~Application();

        int Start(int argc, char* argv[]);
    };
}