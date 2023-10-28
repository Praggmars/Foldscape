#pragma once

#include <gtk/gtk.h>

namespace foldscape
{
    class Application
    {
        GtkApplication* m_gtkApp;
        GtkWidget* m_mainWindow;
        GtkWidget* m_glArea;

    private:
        void Init(const char* title, int width, int height);

    public:
        Application();
        ~Application();

        int Start(int argc, char* argv[]);
    };
}