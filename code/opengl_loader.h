#if !defined(OPENGL_LOADER_H)
#define OPENGL_LOADER_H

// 1. Central list of OpenGL functions to load: X(ReturnType/PFN_Type, FunctionName)
#define GL_FUNCTIONS_LIST \
    /* Shaders & Programs */ \
    X(PFNGLCREATESHADERPROC,            glCreateShader) \
    X(PFNGLSHADERSOURCEPROC,            glShaderSource) \
    X(PFNGLCOMPILESHADERPROC,           glCompileShader) \
    X(PFNGLGETSHADERIVPROC,             glGetShaderiv) \
    X(PFNGLGETSHADERINFOLOGPROC,        glGetShaderInfoLog) \
    X(PFNGLDELETESHADERPROC,            glDeleteShader) \
    X(PFNGLCREATEPROGRAMPROC,           glCreateProgram) \
    X(PFNGLATTACHSHADERPROC,            glAttachShader) \
    X(PFNGLLINKPROGRAMPROC,             glLinkProgram) \
    X(PFNGLGETPROGRAMIVPROC,            glGetProgramiv) \
    X(PFNGLGETPROGRAMINFOLOGPROC,       glGetProgramInfoLog) \
    X(PFNGLUSEPROGRAMPROC,              glUseProgram) \
    X(PFNGLDELETEPROGRAMPROC,           glDeleteProgram) \
    /* Buffers & Vertex Arrays (VBO / VAO / EBO) */ \
    X(PFNGLGENVERTEXARRAYSPROC,         glGenVertexArrays) \
    X(PFNGLBINDVERTEXARRAYPROC,         glBindVertexArray) \
    X(PFNGLDELETEVERTEXARRAYSPROC,      glDeleteVertexArrays) \
    X(PFNGLGENBUFFERSPROC,              glGenBuffers) \
    X(PFNGLBINDBUFFERPROC,              glBindBuffer) \
    X(PFNGLBUFFERDATAPROC,              glBufferData) \
    X(PFNGLBUFFERSUBDATAPROC,           glBufferSubData) \
    X(PFNGLDELETEBUFFERSPROC,           glDeleteBuffers) \
    X(PFNGLVERTEXATTRIBPOINTERPROC,     glVertexAttribPointer) \
    X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
    /* Uniforms & Textures */ \
    X(PFNGLGETUNIFORMLOCATIONPROC,      glGetUniformLocation) \
    X(PFNGLUNIFORM1IPROC,               glUniform1i) \
    X(PFNGLUNIFORM1FPROC,               glUniform1f) \
    X(PFNGLUNIFORM3FPROC,               glUniform3f) \
    X(PFNGLUNIFORM4FPROC,               glUniform4f) \
    X(PFNGLUNIFORMMATRIX4FVPROC,        glUniformMatrix4fv) \
    X(PFNGLACTIVETEXTUREPROC,           glActiveTexture) \
    X(PFNGLGENERATEMIPMAPPROC,          glGenerateMipmap)

// 2. Automatically declare all global function pointers initialized to NULL
#define X(type, name) type pfn_##name = NULL;
GL_FUNCTIONS_LIST
#undef X

// 2. Map standard gl* names to your loaded pointers
#define glCreateShader            pfn_glCreateShader
#define glShaderSource            pfn_glShaderSource
#define glCompileShader           pfn_glCompileShader
#define	glGetShaderiv				pfn_glGetShaderiv
#define	glGetShaderInfoLog          pfn_glGetShaderInfoLog
#define	glDeleteShader              pfn_glDeleteShader
#define	glCreateProgram             pfn_glCreateProgram
#define	glAttachShader              pfn_glAttachShader
#define	glLinkProgram               pfn_glLinkProgram
#define	glGetProgramiv              pfn_glGetProgramiv
#define	glGetProgramInfoLog         pfn_glGetProgramInfoLog
#define	glUseProgram                pfn_glUseProgram
#define	glDeleteProgram             pfn_glDeleteProgram
#define	glGenVertexArrays           pfn_glGenVertexArrays
#define	glBindVertexArray           pfn_glBindVertexArray
#define	glDeleteVertexArrays        pfn_glDeleteVertexArrays
#define	glGenBuffers                pfn_glGenBuffers
#define	glBindBuffer                pfn_glBindBuffer
#define	glBufferData                pfn_glBufferData
#define	glBufferSubData             pfn_glBufferSubData
#define	glDeleteBuffers             pfn_glDeleteBuffers
#define	glVertexAttribPointer       pfn_glVertexAttribPointer
#define	glEnableVertexAttribArray   pfn_glEnableVertexAttribArray
#define	glGetUniformLocation        pfn_glGetUniformLocation
#define	glUniform1i                 pfn_glUniform1i
#define	glUniform1f                 pfn_glUniform1f
#define	glUniform3f                 pfn_glUniform3f
#define	glUniform4f                 pfn_glUniform4f
#define	glUniformMatrix4fv          pfn_glUniformMatrix4fv
#define	glActiveTexture             pfn_glActiveTexture
#define	glGenerateMipmap            pfn_glGenerateMipmap

// 3. Load into the prefixed pointers
bool load_gl_functions(void) {
    bool all_loaded = true;

    #define X(type, name) \
        pfn_##name = (type)SDL_GL_GetProcAddress(#name); \
        if (!pfn_##name) { \
            fprintf(stderr, "Failed to load OpenGL function: %s\n", #name); \
            all_loaded = false; \
        }
    GL_FUNCTIONS_LIST
    #undef X

    return all_loaded;
}

#endif
