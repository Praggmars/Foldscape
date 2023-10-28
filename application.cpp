#include "application.h"
#include <epoxy/gl.h>
#include <iostream>

G_BEGIN_DECLS

#define GLAREA_ERROR (glarea_error_quark ())

typedef enum {
    GLAREA_ERROR_SHADER_COMPILATION,
    GLAREA_ERROR_SHADER_LINK
} GlareaError;

GQuark glarea_error_quark (void);

G_END_DECLS

G_DEFINE_QUARK (glarea-error, glarea_error)

namespace foldscape
{
    namespace shadercode
    {
        const char g_VsCode[] = R"(#version 130
in vec3 position;
smooth out vec2 coord;
void main()
{
    gl_Position = vec4(position, 1.0f);
    coord = vec2(position.x, position.y);
}
)";

        const char g_FsCode[] = R"(#version 130
smooth in vec2 coord;
out vec4 outputColor;
void main()
{
    outputColor = vec4(coord.x * 0.5f + 0.5f, coord.y * 0.5f + 0.5f, 0.0f, 1.0f);
}
)";
    }

    static GError* OpenGlCreateShader(int shaderType, const char* code, guint& shader)
    {
        GError* err = nullptr;

        shader = glCreateShader(shaderType);
        glShaderSource(shader, 1, &code, nullptr);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        if (GL_FALSE == status)
        {return err;
            GLint logLen;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLen);
            char* buffer = new char[logLen + 1];
            memset(buffer, 0, logLen + 1);
            glGetShaderInfoLog(shader, logLen, nullptr, buffer);
            g_set_error(&err, GLAREA_ERROR, GLAREA_ERROR_SHADER_COMPILATION, "Compilation failure in %s shader: %s", shaderType == GL_VERTEX_SHADER ? "vertex" : "fragment", buffer);
            delete[] buffer;
            glDeleteShader(shader);
            shader = 0;
        }
        return err;
    }

    Application::Application()
        : m_gtkApp{nullptr}
        , m_mainWindow{nullptr}
        , m_glArea{nullptr}
        , m_vertexBuffer{0}
        , m_positionLocation{0}
        , m_program{0}
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
            app->OpenGlInit();
        };
        g_signal_connect(m_glArea, "realize", G_CALLBACK(glInit), this);

        void(*glDeinit)(GtkWidget*, Application*) = [](GtkWidget* self, Application* app){
            app->OpenGlDeinit();
        };
        g_signal_connect(m_glArea, "unrealize", G_CALLBACK(glDeinit), this);

        //void(*glResize)(GtkGLArea*, gint, gint, Application*) = [](GtkGLArea* self, gint width, gint height, Application* app){
        //    app->OpenGlResize(width, height);
        //};
        //g_signal_connect(m_glArea, "resize", G_CALLBACK(glResize), this);

        gboolean(*glRender)(GtkGLArea*, GdkGLContext*, Application*) = [](GtkGLArea* self, GdkGLContext* context, Application* app){
            app->GlRender();
            return static_cast<gboolean>(false);
        };
        g_signal_connect(m_glArea, "render", G_CALLBACK(glRender), this);

        gtk_widget_set_hexpand(m_glArea, true);
        gtk_widget_set_vexpand(m_glArea, true);
        gtk_widget_set_visible(m_glArea, true);
    }

    void Application::OpenGlInit()
    {
        gtk_gl_area_make_current(GTK_GL_AREA(m_glArea));

        if (GError* err = gtk_gl_area_get_error(GTK_GL_AREA(m_glArea)))
        {
            std::cerr << err->message << std::endl;
            return;
        }

        if (GError* err = OpenGlInitShaders())
        {
            std::cerr << err->message << std::endl;
            gtk_gl_area_set_error(GTK_GL_AREA(m_glArea), err);
            g_error_free(err);
            return;
        }

        OpenGlInitBuffers();

        if (const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER)))
            std::cout << renderer << std::endl;
        else
            std::cout << "Unknown renderer" << std::endl;
    }

    GError* Application::OpenGlInitShaders()
    {
        GError* err = nullptr;
        guint vertexShader, fragmentShader;

        err = OpenGlCreateShader(GL_VERTEX_SHADER, shadercode::g_VsCode, vertexShader);
        if (err) return err;

        err = OpenGlCreateShader(GL_FRAGMENT_SHADER, shadercode::g_FsCode, fragmentShader);
        if (err) return err;

        m_program = glCreateProgram();
        glAttachShader(m_program, vertexShader);
        glAttachShader(m_program, fragmentShader);
        glLinkProgram(m_program);

        GLint status = 0;
        glGetProgramiv(m_program, GL_LINK_STATUS, &status);
        if (GL_FALSE == status)
        {
            GLint logLen = 0;
            glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &logLen);
            char* buffer = new char[logLen + 1];
            memset(buffer, 0, logLen + 1);
            glGetProgramInfoLog(m_program, logLen, nullptr, buffer);
            g_set_error(&err, GLAREA_ERROR, GLAREA_ERROR_SHADER_LINK, "Linking failure in program: %s", buffer);
            delete[] buffer;
            glDeleteProgram(m_program);
            m_program = 0;
            return err;
        }

        m_positionLocation = glGetAttribLocation(m_program, "position");
        glDetachShader(m_program, vertexShader);
        glDeleteShader(vertexShader);
        glDetachShader(m_program, fragmentShader);
        glDeleteShader(fragmentShader);

        return nullptr;
    }

    void Application::OpenGlInitBuffers()
    {
        const float vertices[] = {
            -1.0f, -1.0f, 0.0f,
            -1.0f,  1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
             1.0f,  1.0f, 0.0f,
             1.0f, -1.0f, 0.0f,
            -1.0f, -1.0f, 0.0f
        };

        glGenVertexArrays(1, &m_vertexBuffer);
        glBindVertexArray(m_vertexBuffer);

        guint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(m_positionLocation);
        glVertexAttribPointer(m_positionLocation, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
        glDeleteBuffers(1, &buffer);
    }

    void Application::OpenGlDeinit()
    {
        gtk_gl_area_make_current(GTK_GL_AREA(m_glArea));

        if (GError* err = gtk_gl_area_get_error(GTK_GL_AREA(m_glArea)))
        {
            std::cerr << err->message << std::endl;
            return;
        }

        if (m_vertexBuffer)
        {
            glDeleteVertexArrays(1, &m_vertexBuffer);
            m_vertexBuffer = 0;
        }
        if (m_program)
        {
            glDeleteProgram(m_program);
            m_program = 0;
        }
    }

    void Application::OpenGlResize(int width, int height)
    {
    }

    void Application::GlRender() const
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        glUseProgram(m_program);
        glBindVertexArray(m_vertexBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glBindVertexArray(0);
        glUseProgram(0);

        glFlush();
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