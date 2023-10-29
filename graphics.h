#pragma once

#include <gtk/gtk.h>

namespace foldscape
{
    class Graphics
    {
    public:
        struct ShaderData
        {
            float center[2];
            float zoom;
            float aspectRatio;
        };

    private:
        guint m_vertexBuffer;
        guint m_positionLocation;
        guint m_program;
        
        guint m_shaderDataCenter;
        guint m_shaderDataZoom;
        guint m_shaderDataAspectRatio;

    private:
        GError* InitShaders();
        void InitBuffers();

    public:
        Graphics();
        ~Graphics();

        void Init(GtkGLArea* glArea);
        void Deinit(GtkGLArea* glArea);
        void Render(const ShaderData& shaderData) const;
    };
}