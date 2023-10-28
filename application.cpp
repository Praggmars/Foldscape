#include "application.h"

namespace foldscape
{
    Application::Application()
        : m_gtkApp{nullptr}
        , m_mainWindow{nullptr}
        , m_glArea{nullptr}
    {
    }

    Application::~Application()
    {
    }

    void Application::Init(const char* title, int width, int height)
    {
        m_mainWindow = gtk_application_window_new(m_gtkApp);
        gtk_window_set_title(GTK_WINDOW(m_mainWindow), title);
        gtk_window_set_default_size(GTK_WINDOW(m_mainWindow), width, height);
        gtk_window_present(GTK_WINDOW(m_mainWindow));
    }

    int Application::Start(int argc, char* argv[])
    {
        void(*app_activate)(GApplication* gtkApp, Application* application) = [](GApplication* gtkApp, Application* application){
            application->m_gtkApp = GTK_APPLICATION(gtkApp);
            application->Init("Foldscape", 1000, 700);
            };

        GtkApplication* gtkApp = gtk_application_new("com.github.praggmars", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(gtkApp, "activate", G_CALLBACK(app_activate), this);
        int stat = g_application_run(G_APPLICATION(gtkApp), argc, argv);
        g_object_unref(gtkApp);
        return stat;
    }
}