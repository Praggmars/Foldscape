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
        , m_dragTimeCenter{0.0f, 0.0f}
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

        InitControl();

        gtk_window_present(GTK_WINDOW(m_mainWindow));
    }

    void Application::CreateGlArea()
    {
        m_glArea = gtk_gl_area_new();

        g_signal_connect(m_glArea, "realize", G_CALLBACK(
            static_cast<void(*)(GtkWidget*, Application*)>(
                [](GtkWidget* self, Application* app){
                    app->m_graphics.Init(GTK_GL_AREA(app->m_glArea));
        })), this);

        g_signal_connect(m_glArea, "unrealize", G_CALLBACK(
            static_cast<void(*)(GtkWidget*, Application*)>(
                [](GtkWidget* self, Application* app){
                    app->m_graphics.Deinit(GTK_GL_AREA(app->m_glArea));
        })), this);

        g_signal_connect(m_glArea, "render", G_CALLBACK(
            static_cast<gboolean(*)(GtkGLArea*, GdkGLContext*, const Application*)>(
                [](GtkGLArea* self, GdkGLContext* context, const Application* app){
                    app->Render();
                    return static_cast<gboolean>(false);
        })), this);

        gtk_widget_set_hexpand(m_glArea, true);
        gtk_widget_set_vexpand(m_glArea, true);
        gtk_widget_set_visible(m_glArea, true);
    }

    void Application::InitControl()
    {
        GtkGesture* dragControl = gtk_gesture_drag_new();        
        g_signal_connect(dragControl, "drag-begin", G_CALLBACK(
            static_cast<void(*)(GtkGestureDrag*, double, double, Application*)>(
                [](GtkGestureDrag* gesture, double x, double y, Application* app){
                    app->DragBegin(x, y);
            })), this);        
        g_signal_connect(dragControl, "drag-update", G_CALLBACK(
            static_cast<void(*)(GtkGestureDrag*, double, double, Application*)>(
                [](GtkGestureDrag* gesture, double x, double y, Application* app){
                    app->DragUpdate(x, y);
            })), this);        
        g_signal_connect(dragControl, "drag-end", G_CALLBACK(
            static_cast<void(*)(GtkGestureDrag*, double, double, Application*)>(
                [](GtkGestureDrag* gesture, double x, double y, Application* app){
                    app->DragEnd(x, y);
            })), this);
        gtk_widget_add_controller(m_glArea, GTK_EVENT_CONTROLLER(dragControl));

        GtkEventController* scrollControl = gtk_event_controller_scroll_new(GTK_EVENT_CONTROLLER_SCROLL_VERTICAL);
        g_signal_connect(scrollControl, "scroll", G_CALLBACK(
            static_cast<void(*)(GtkGestureDrag*, double, double, Application*)>(
                [](GtkGestureDrag* gesture, double x, double y, Application* app){
                    app->Scroll(x, y);
            })), this);
        gtk_widget_add_controller(m_glArea, GTK_EVENT_CONTROLLER(scrollControl));

        GtkEventController* motionControl = gtk_event_controller_motion_new();
        g_signal_connect(motionControl, "motion", G_CALLBACK(
            static_cast<void(*)(GtkEventControllerMotion*, double, double, Application*)>(
                [](GtkEventControllerMotion* motionControl, double x, double y, Application* app){
                    app->PointerMotion(x, y);
            })), this);
        gtk_widget_add_controller(m_glArea, motionControl);
    }

    void Application::DragBegin(double x, double y)
    {
        m_dragTimeCenter[0] = m_center[0];
        m_dragTimeCenter[1] = m_center[1];
    }

    void Application::DragUpdate(double x, double y)
    {
        const float multiplier = 2.0f * m_zoom / (static_cast<float>(gtk_widget_get_height(m_glArea)));
        m_center[0] = m_dragTimeCenter[0] - static_cast<float>(x) * multiplier;
        m_center[1] = m_dragTimeCenter[1] + static_cast<float>(y) * multiplier;
        gtk_gl_area_queue_render(GTK_GL_AREA(m_glArea));
    }

    void Application::DragEnd(double x, double y)
    {
    }

    void Application::Scroll(double x, double y)
    {
        float coordBefore[2]{};
        ScreenToCoord(m_cursor, coordBefore);

        m_zoom *= std::exp(static_cast<float>(y) * 0.2f);

        float coordAfter[2]{};
        ScreenToCoord(m_cursor, coordAfter);

        m_center[0] += coordBefore[0] - coordAfter[0];
        m_center[1] += coordBefore[1] - coordAfter[1];

        gtk_gl_area_queue_render(GTK_GL_AREA(m_glArea));
    }

    void Application::PointerMotion(double x, double y)
    {
        m_cursor[0] = static_cast<float>(x);
        m_cursor[1] = static_cast<float>(y);
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

    void Application::ScreenToCoord(const float screen[2], float coord[2]) const
    {
        const float r[] = {
            static_cast<float>(gtk_widget_get_width(m_glArea)),
            static_cast<float>(gtk_widget_get_height(m_glArea))
        };
        coord[0] = (screen[0] - (r[0] - 1.0f) * 0.5f) / ((r[0] - 1.0f) * r[1]) * 2.0f * r[0] * m_zoom + m_center[0];
        coord[1] = (screen[1] - (r[1] - 1.0f) * 0.5f) / (1.0f - r[1]) * 2.0f * m_zoom + m_center[1];
    }

    void Application::CoordToScreen(const float coord[2], float screen[2]) const
    {
        const float r[] = {
            static_cast<float>(gtk_widget_get_width(m_glArea)),
            static_cast<float>(gtk_widget_get_height(m_glArea))
        };
        screen[0] = (coord[0] - m_center[0]) * (r[0] - 1.0f) * r[1] / (2.0f * r[0] * m_zoom) + (r[0] - 1.0f) * 0.5f;
        screen[1] = (coord[1] - m_center[1]) * (1.0f - r[1]) / (2.0f * m_zoom) + (r[1] - 1.0f) * 0.5f;
    }

    int Application::Start(int argc, char* argv[])
    {
        GtkApplication* gtkApp = gtk_application_new("com.praggmars.foldscape", G_APPLICATION_DEFAULT_FLAGS);
        g_signal_connect(gtkApp, "activate", G_CALLBACK(
            static_cast<void(*)(GApplication* gtkApp, Application* application)>(
                [](GApplication* gtkApp, Application* application){
                    application->m_gtkApp = GTK_APPLICATION(gtkApp);
                    application->Init("Foldscape", 1000, 700);
        })), this);
        int stat = g_application_run(G_APPLICATION(gtkApp), argc, argv);
        g_object_unref(gtkApp);
        return stat;
    }
}