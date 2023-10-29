#include "application.h"
#include <iostream>

namespace foldscape
{
    Application::Application()
        : m_gtkApp{nullptr}
        , m_mainWindow{nullptr}
        , m_glArea{nullptr}
        , m_center{-0.5f, 0.0f}
        , m_zoom{1.25f}
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

        CreateGlArea();
        gtk_window_set_child(GTK_WINDOW(m_mainWindow), m_glArea);

        gtk_window_present(GTK_WINDOW(m_mainWindow));
    }

    void Application::CreateGlArea()
    {
        m_glArea = gtk_gl_area_new();

        void(*glInit)(GtkWidget*, Application*) = [](GtkWidget* self, Application* app){
            app->m_graphics.Init(GTK_GL_AREA(app->m_glArea));
        };
        g_signal_connect(m_glArea, "realize", G_CALLBACK(glInit), this);

        void(*glDeinit)(GtkWidget*, Application*) = [](GtkWidget* self, Application* app){
            app->m_graphics.Deinit(GTK_GL_AREA(app->m_glArea));
        };
        g_signal_connect(m_glArea, "unrealize", G_CALLBACK(glDeinit), this);

        gboolean(*glRender)(GtkGLArea*, GdkGLContext*, const Application*) = [](GtkGLArea* self, GdkGLContext* context, const Application* app){
            app->Render();
            return static_cast<gboolean>(false);
        };
        g_signal_connect(m_glArea, "render", G_CALLBACK(glRender), this);

        gtk_widget_set_hexpand(m_glArea, true);
        gtk_widget_set_vexpand(m_glArea, true);
        gtk_widget_set_visible(m_glArea, true);
    }

    void Application::Render() const
    {
        m_graphics.Render(
            Graphics::ShaderData{
                { m_center[0], m_center[1] },
                m_zoom,
                static_cast<float>(gtk_widget_get_width(m_glArea)) / static_cast<float>(gtk_widget_get_height(m_glArea))
        });        
    }

    int Application::Start(int argc, char* argv[])
    {
        void(*app_activate)(GApplication* gtkApp, Application* application) = [](GApplication* gtkApp, Application* application){
            application->m_gtkApp = GTK_APPLICATION(gtkApp);
            application->Init("Foldscape", 1000, 700);
            };

        GtkApplication* gtkApp = gtk_application_new("com.praggmars.foldscape", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(gtkApp, "activate", G_CALLBACK(app_activate), this);
        int stat = g_application_run(G_APPLICATION(gtkApp), argc, argv);
        g_object_unref(gtkApp);
        return stat;
    }
}