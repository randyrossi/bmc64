/**
 * \file opengl_renderer.c
 * \brief   OpenGL-based renderer for the GTK3 backend.
 *
 * \author Michael C. Martin <mcmartin@gmail.com>
 */

/* This file is part of VICE, the Versatile Commodore Emulator.
 * See README for copyright notice.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 *  02111-1307  USA.
 *
 */

#include "opengl_renderer.h"

#ifdef HAVE_GTK3_OPENGL

#include <string.h>

#ifdef MACOSX_SUPPORT
#include <OpenGL/gl3.h>
#else
#ifdef HAVE_GTK3_GLEW
#include <GL/glew.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif
#endif

#include "lib.h"
#include "resources.h"
#include "ui.h"
#include "video.h"

/** \brief The screen has not changed, so the texture may be used
 *         unchanged */
#define RENDER_MODE_STATIC      0
/** \brief The texture must be completely recreated from scratch */
#define RENDER_MODE_NEW_TEXTURE 1
/** \brief The texture needs some or all of its pixels updated */
#define RENDER_MODE_DIRTY_RECT  2

/** \brief Rendering context for the OpenGL backend.
 *  \sa video_canvas_s::renderer_context */
typedef struct vice_opengl_renderer_context_s {
    /** \brief The OpenGL program that comprises our vertex and
     *         fragment shaders. */
    GLuint program;
    /** \brief The index of the "position" parameter in the shader
     *         program. */
    GLuint position_index;
    /** \brief The index of the "texCoord" parameter in the shader
     *         program. */
    GLuint tex_coord_index;
    /** \brief The vertex buffer object that holds our vertex data. */
    GLuint vbo;
    /** \brief The vertex array object that gives structure to our
     *         vertex data. */
    GLuint vao;
    /** \brief The texture identifier for the GPU's copy of our
     *         machine display. */
    GLuint texture;
    /** \brief Width of the texture, in pixels. */
    unsigned int width;
    /** \brief Height of the texture, in pixels. */
    unsigned int height;
    /** \brief The raw pixel data that is the CPU's copy of our
     * machine display. */
    unsigned char *backbuffer;
    /** \brief Fraction of the window width the scaled machine display
     *         takes up (1.0f=entire width). */
    float scale_x;
    /** \brief Fraction of the window height the scaled machine display
     *         takes up (1.0f=entire height). */
    float scale_y;
    /** \brief X coordinate of leftmost pixel that needs to be updated
     * in the texture. */
    unsigned int dirty_x;
    /** \brief Y coordinate of topmost pixel that needs to be updated
     * in the texture. */
    unsigned int dirty_y;
    /** \brief Width of the rectangle that needs to be updated in the
     * texture. */
    unsigned int dirty_w;
    /** \brief Height of the rectangle that needs to be updated in the
     * texture. */
    unsigned int dirty_h;
    /** \brief What preprocessing the texture will need pre-rendering.
     *
     * Must be one of RENDER_MODE_STATIC, RENDER_MODE_NEW_TEXTURE, or
     * RENDER_MODE_DIRTY_RECT.
     */
    unsigned int render_mode;
} context_t;

/** \brief Raw geometry for the machine screen.
 *
 * The first sixteen elements describe a rectangle the size of the
 * entire display area, and the last eight assign texture coordinates
 * to each corner.
 */
static float vertexData[] = {
        -1.0f,    -1.0f, 0.0f, 1.0f,
         1.0f,    -1.0f, 0.0f, 1.0f,
        -1.0f,     1.0f, 0.0f, 1.0f,
         1.0f,     1.0f, 0.0f, 1.0f,
         0.0f,     1.0f,
         1.0f,     1.0f,
         0.0f,     0.0f,
         1.0f,     0.0f
};

/** \brief Our renderer's vertex shader. 
 *
 * This simply scales the geometry it is provided and provides
 * smoothly interpolated texture coordinates between each vertex. The
 * world coordinates remain [-1, 1] in all dimensions. */
static const char *vertexShader = "#version 150\n"
    "uniform vec4 scale;\n"
    "in vec4 position;\n"
    "in vec2 tex;\n"
    "smooth out vec2 texCoord;\n"
    "void main() {\n"
    "  gl_Position = position * scale;\n"
    "  texCoord = tex;\n"
    "}\n";

/** \brief Our renderer's fragment shader.
 *
 * This does nothing but texture lookups based on the values fed to it
 * by the vertex shader. */
static const char *fragmentShader = "#version 150\n"
    "uniform sampler2D sampler;\n"
    "smooth in vec2 texCoord;\n"
    "out vec4 outputColor;\n"
    "void main() { outputColor = texture2D(sampler, texCoord); }\n";

/** \brief Compile a shader.
 *
 *  If the shader cannot be compiled, error messages from OpenGL will
 *  be dumped to stdout.
 *
 *  \param shader_type The kind of shader being compiled. Must be
 *                     either GL_VERTEX_SHADER or GL_FRAGMENT_SHADER.
 *  \param text        The shader source.
 *  \return The identifier of the shader.
 */
static GLuint create_shader(GLenum shader_type, const char *text)
{
    GLuint shader = glCreateShader(shader_type);
    GLint status = 0;
    glShaderSource(shader, 1, &text, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE) {
        GLint info_log_length;
        GLchar *info_log;
        const char *shader_type_name = NULL;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_log_length);
        info_log = lib_malloc(sizeof(GLchar) * (info_log_length + 1));
        glGetShaderInfoLog(shader, info_log_length, NULL, info_log);

        switch(shader_type)
        {
        case GL_VERTEX_SHADER: shader_type_name = "vertex"; break;
        case GL_FRAGMENT_SHADER: shader_type_name = "fragment"; break;
        default: shader_type_name = "unknown"; break;
        }

        fprintf(stderr, "Compile failure in %s shader:\n%s\n", shader_type_name, info_log);
        lib_free(info_log);
    }

    return shader;
}

/** \brief Compile and link the renderer's shaders.
 *
 *  If successful, the vice_opengl_renderer_context_s::program,
 *  vice_opengl_renderer_context_s::position_index, and
 *  vice_opengl_renderer_context_s::tex_coord_index fields will be
 *  filled in with values for future use.
 *
 *  \param ctx The renderer context that will receive the results.
 */
static void create_shader_program(context_t *ctx)
{
    GLuint program = glCreateProgram();
    GLuint vert = create_shader(GL_VERTEX_SHADER, vertexShader);
    GLuint frag = create_shader(GL_FRAGMENT_SHADER, fragmentShader);
    GLint status;

    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);
    glGetProgramiv (program, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint info_log_length;
        GLchar *info_log;

        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_log_length);
        info_log = lib_malloc(sizeof(GLchar) * (info_log_length + 1));
        glGetProgramInfoLog(program, info_log_length, NULL, info_log);
        fprintf(stderr, "Linker failure: %s\n", info_log);
        lib_free(info_log);
    }

    glDeleteShader(vert);
    glDeleteShader(frag);
    ctx->position_index = glGetAttribLocation(program, "position");
    ctx->tex_coord_index = glGetAttribLocation(program, "tex");
    ctx->program = program;
}

/** \brief GTK3 callback when setting up the OpenGL context.
 *
 * This is also where OpenGL compatibility is checked. If the system
 * is not OpenGL 3.2-compatible, or if the area otherwise fails to
 * initialize, errors will be logged to stderr.
 *
 * \param area      The widget being initialized.
 * \param user_data The video_canvas_s associated with this widget.
 *
 * \warning If initialization fails, the display will completely fail
 *          to render. However, some experimentation has shown that
 *          some displays will successfully render as a 3.2 context
 *          even when the driver only purports to support up to
 *          2.1. It is not clear exactly what the requirements truly
 *          are. 
 */
static void realize_opengl_cb (GtkGLArea *area, gpointer user_data)
{
    video_canvas_t *canvas = (video_canvas_t *)user_data;
    context_t *ctx = NULL;
    GError *err = NULL;
    GLenum glErr;

    gtk_gl_area_make_current(area);
    err = gtk_gl_area_get_error(area);
    if (err != NULL) {
        fprintf(stderr, "CRITICAL: Could not realize GL context: %s\n", err->message);
        return;
    }
    if (canvas->renderer_context) {
        fprintf(stderr, "WARNING: Re-realizing the GtkGL area! This will leak.\n");
    }
    ctx = lib_malloc(sizeof(context_t));
    memset(ctx, 0, sizeof(context_t));
    canvas->renderer_context = ctx;
#ifdef HAVE_GTK3_GLEW
    glewExperimental = GL_TRUE;
    glErr = glewInit();
    if (glErr != GLEW_OK) {
        fprintf(stderr, "GTKGL: Could not initialize GLEW\n");
    }
    if (!GLEW_VERSION_3_2) {
        fprintf(stderr, "GTKGL: OpenGL version 3.2 not supported in this context\n");
    }
#endif

    create_shader_program(ctx);
    glGenBuffers(1, &ctx->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glGenVertexArrays(1, &ctx->vao);
    glGenTextures(1, &ctx->texture);
}

/** \brief OpenGL render callback.
 *  \param area   The widget being rendered.
 *  \param unused The GDK context that wraps OpenGL.
 *  \param data   The video_canvas_s associated with this widget.
 *  \return TRUE if no further processing is needed on this event.
 *  \todo It should be possible to select GL_NEAREST or GL_LINEAR when
 *        deciding how to scale textures.
 */
static gboolean render_opengl_cb (GtkGLArea *area, GdkGLContext *unused, gpointer data)
{
    video_canvas_t *canvas = data;
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;

    if (ui_is_fullscreen()) {
        glClearColor (0.0f,0.0f,0.0f,1.0f);
    } else {
        glClearColor (0.5f,0.5f,0.5f,1.0f);
    }
    glClear (GL_COLOR_BUFFER_BIT);

    if (!ctx) {
        /* Nothing else to do */
        return TRUE;
    }
    glActiveTexture(GL_TEXTURE0);
    if (ctx->render_mode == RENDER_MODE_NEW_TEXTURE) {
        if (ctx->texture == 0) {
            fprintf(stderr, "GTKGL CRITICAL: No texture generated!\n");
        }
        glBindTexture(GL_TEXTURE_2D, ctx->texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, ctx->width);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ctx->width, ctx->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, ctx->backbuffer);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        /* These should be selectable as GL_LINEAR or GL_NEAREST */
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);
    } else if (ctx->render_mode == RENDER_MODE_DIRTY_RECT) {
        glBindTexture(GL_TEXTURE_2D, ctx->texture);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, ctx->width);
        glTexSubImage2D(GL_TEXTURE_2D, 0, ctx->dirty_x, ctx->dirty_y, ctx->dirty_w, ctx->dirty_h, GL_RGBA, GL_UNSIGNED_BYTE, ctx->backbuffer + 4 * (ctx->width * ctx->dirty_y + ctx->dirty_x));
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    ctx->render_mode = RENDER_MODE_STATIC;
    if (ctx->program) {
        GLuint scale_uniform, sampler_uniform;

        glUseProgram(ctx->program);

        glBindVertexArray(ctx->vao);
        glBindBuffer(GL_ARRAY_BUFFER, ctx->vbo);
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(ctx->position_index, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glVertexAttribPointer(ctx->tex_coord_index, 2, GL_FLOAT, GL_FALSE, 0, (void*)64);

        scale_uniform = glGetUniformLocation(ctx->program, "scale");
        glUniform4f(scale_uniform, ctx->scale_x, ctx->scale_y, 1.0f, 1.0f);
        sampler_uniform = glGetUniformLocation(ctx->program, "sampler");
        glUniform1i(sampler_uniform, 0);

        glBindTexture(GL_TEXTURE_2D, ctx->texture);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glBindTexture(GL_TEXTURE_2D, 0);

        glDisableVertexAttribArray(ctx->position_index);
        glDisableVertexAttribArray(ctx->tex_coord_index);
        glUseProgram(0);
    }

    return TRUE;
}

/** \brief OpenGL viewport resize callback, called when the user or OS
 *         has resized the window but the machine screen remains
 *         intact.
 *  \param area      The widget being resized.
 *  \param width     The new viewport width.
 *  \param height    The new viewport height.
 *  \param user_data The video_canvas_s associated with this widget.
 */
static void
resize_opengl_cb (GtkGLArea *area, gint width, gint height, gpointer user_data)
{
    video_canvas_t *canvas = (video_canvas_t *)user_data;
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;
    int keepaspect = 1, trueaspect = 0;
    if (!ctx || ctx->width == 0 || ctx->height == 0) {
        return;
    }

    if (width <= 0) {
        width = 1;
    }
    if (height <= 0) {
        height = 1;
    }

    resources_get_int("KeepAspectRatio", &keepaspect);
    resources_get_int("TrueAspectRatio", &trueaspect);

    if (keepaspect) {
        float canvas_aspect, viewport_aspect;

        viewport_aspect = (float)width / (float)height;
        canvas_aspect = (float)ctx->width / (float)ctx->height;
        if (trueaspect) {
            canvas_aspect *= canvas->geometry->pixel_aspect_ratio;
        }
        if (canvas_aspect < viewport_aspect) {
            ctx->scale_x = canvas_aspect / viewport_aspect;
            ctx->scale_y = 1.0f;
        } else {
            ctx->scale_x = 1.0f;
            ctx->scale_y = viewport_aspect / canvas_aspect;
        }
    } else {
        ctx->scale_x = 1.0f;
        ctx->scale_y = 1.0f;
    }
    canvas->screen_display_w = (double)width * ctx->scale_x;
    canvas->screen_display_h = (double)height * ctx->scale_y;
    canvas->screen_origin_x = ((double)width - canvas->screen_display_w) / 2.0;
    canvas->screen_origin_y = ((double)height - canvas->screen_display_h) / 2.0;
}

/** \brief OpenGL implementation of create_widget.
 *
 *  \param canvas The canvas to create the widget for.
 *  \return The newly created canvas.
 *  \sa vice_renderer_backend_s::create_widget
 */
static GtkWidget *vice_opengl_create_widget(video_canvas_t *canvas)
{
    GtkWidget *widget = gtk_gl_area_new();
    gtk_widget_set_hexpand(widget, TRUE);
    gtk_widget_set_vexpand(widget, TRUE);
    canvas->drawing_area = widget;
    canvas->renderer_context = NULL;
    g_signal_connect (widget, "realize", G_CALLBACK (realize_opengl_cb), canvas);
    g_signal_connect (widget, "render", G_CALLBACK (render_opengl_cb), canvas);
    g_signal_connect (widget, "resize", G_CALLBACK (resize_opengl_cb), canvas);
    return widget;
}

/** \brief OpenGL implementation of destroy_context.
 * 
 *  \param canvas The canvas whose renderer_context is to be
 *                deleted
 *  \sa vice_renderer_backend_s::destroy_context
 */
static void vice_opengl_destroy_context(video_canvas_t *canvas)
{
    if (canvas) {
        context_t *ctx = (context_t *)canvas->renderer_context;
        if (ctx == NULL) {
            return;
        }
        /* TODO: delete textures, shaders, backbuffers, etc */
        if (ctx->backbuffer != NULL) {
            lib_free(ctx->backbuffer);
        }
        canvas->renderer_context = NULL;
        lib_free(ctx);
    }
}

/** \brief OpenGL implementation of update_context.
 * \param canvas The canvas being resized or initially created.
 * \param width The new width for the machine's screen.
 * \param height The new height for the machine's screen.
 * \sa vice_renderer_backend_s::update_context
 */
static void vice_opengl_update_context(video_canvas_t *canvas, unsigned int width, unsigned int height)
{
    context_t *ctx = canvas ? (context_t *)canvas->renderer_context : NULL;
    if (ctx) {
        double aspect = 1.0;
        int keepaspect = 1, trueaspect = 0;
        gint widget_width, widget_height;
        if (ctx->width == width && ctx->height == height) {
            return;
        }
        if (ctx->backbuffer) {
            lib_free(ctx->backbuffer);
        }
        ctx->width = width;
        ctx->height = height;
        ctx->backbuffer = lib_malloc(width * height * 4);
        ctx->render_mode = RENDER_MODE_NEW_TEXTURE;

        resources_get_int("KeepAspectRatio", &keepaspect);
        resources_get_int("TrueAspectRatio", &trueaspect);
        if (keepaspect && trueaspect) {
            aspect = canvas->geometry->pixel_aspect_ratio;
        }

        /* Configure the matrix to fit it in the widget as it exists */
        widget_width = gtk_widget_get_allocated_width(canvas->drawing_area);
        widget_height = gtk_widget_get_allocated_height(canvas->drawing_area);
        resize_opengl_cb(GTK_GL_AREA(canvas->drawing_area), widget_width, widget_height, canvas);

        /* Fix the widget's size request */
        gtk_widget_set_size_request(canvas->drawing_area, width * aspect, height);
    }
}

/** \brief OpenGL implementation of refresh_rect.
 * \param canvas The canvas being rendered to
 * \param xs     A parameter to forward to video_canvas_render()
 * \param ys     A parameter to forward to video_canvas_render()
 * \param xi     X coordinate of the leftmost pixel to update
 * \param yi     Y coordinate of the topmost pixel to update
 * \param w      Width of the rectangle to update
 * \param h      Height of the rectangle to update
 * \sa vice_renderer_backend_s::refresh_rect */
static void vice_opengl_refresh_rect(video_canvas_t *canvas,
                                     unsigned int xs, unsigned int ys,
                                     unsigned int xi, unsigned int yi,
                                     unsigned int w, unsigned int h)
{
    context_t *ctx = (context_t *)canvas->renderer_context;
    if (!ctx || !ctx->backbuffer) {
        return;
    }

    if (((xi + w) > ctx->width) || ((yi + h) > ctx->height)) {
        /* Trying to draw outside canvas? */
        fprintf(stderr, "Attempt to draw outside canvas!\nXI%u YI%u W%u H%u CW%u CH%u\n", xi, yi, w, h, ctx->width, ctx->height);
        return;
    }

    video_canvas_render(canvas, ctx->backbuffer, w, h, xs, ys, xi, yi, ctx->width * 4, 32);

    if (ctx->render_mode == RENDER_MODE_STATIC) {
        ctx->render_mode = RENDER_MODE_DIRTY_RECT;
        ctx->dirty_x = xi;
        ctx->dirty_y = yi;
        ctx->dirty_w = w;
        ctx->dirty_h = h;
    } else if (ctx->render_mode == RENDER_MODE_DIRTY_RECT) {
        unsigned int x1 = ctx->dirty_x;
        unsigned int y1 = ctx->dirty_y;
        unsigned int x2 = ctx->dirty_x + ctx->dirty_w;
        unsigned int y2 = ctx->dirty_y + ctx->dirty_h;
        if (x1 > xi) {
            x1 = xi;
        }
        if (y1 > yi) {
            y1 = yi;
        }
        if (x2 < xi + w) {
            x2 = xi + w;
        }
        if (y2 < yi + h) {
            y2 = yi + h;
        }
        ctx->dirty_x = x1;
        ctx->dirty_y = y1;
        ctx->dirty_w = x2-x1;
        ctx->dirty_h = y2-y1;
        /* Render mode stays DIRTY_RECT */
    }
    /* Render mode NEW_TEXTURE has no effect; it stays just as new */

    gtk_widget_queue_draw(canvas->drawing_area);
}

/** \brief OpenGL implementation of set_palette.
 * \param canvas The canvas being initialized
 * \sa vice_renderer_backend_s::set_palette */
static void vice_opengl_set_palette(video_canvas_t *canvas)
{
    int i;
    struct palette_s *palette = canvas ? canvas->palette : NULL;
    if (!palette) {
        return;
    }
    /* If we get this far we know canvas is also non-NULL */

    for (i = 0; i < palette->num_entries; i++) {
        palette_entry_t color = palette->entries[i];
        uint32_t color_code = color.red | (color.green << 8) | (color.blue << 16) | (0xff << 24);
        video_render_setphysicalcolor(canvas->videoconfig, i, color_code, 32);
    }

    for (i = 0; i < 256; i++) {
        video_render_setrawrgb(i, i, i << 8, i << 16);
    }
    video_render_setrawalpha(255 << 24);
    video_render_initraw(canvas->videoconfig);
}

vice_renderer_backend_t vice_opengl_backend = {
    vice_opengl_create_widget,
    vice_opengl_update_context,
    vice_opengl_destroy_context,
    vice_opengl_refresh_rect,
    vice_opengl_set_palette
};

#endif
