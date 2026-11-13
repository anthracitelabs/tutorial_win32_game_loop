#include "game_math.h"

#include <stddef.h>


m4x4 myMVP;
// try giving MVP matrix to our shader program
// Get a handle for our "MVP" uniform
// Only during the initialisation
GLuint MatrixID;


 typedef struct {
    GLuint shader_program;
    GLuint vao;
    GLuint vbo;
} GLRenderer;

typedef struct {
    float x, y;
    float r, g, b, a;
} Vertex;

static const char* vs_src =
    "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "layout (location = 1) in vec4 aColor;\n"
	"uniform mat4 MVP;\n"
    "out vec4 vColor;\n"
    "void main() {\n"
    "    vColor = aColor;\n"
    "    gl_Position = MVP * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const char* fs_src =
    "#version 330 core\n"
    "in vec4 vColor;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = vColor;\n"
    "}\n";

GLRenderer r;

void gl_renderer_init() {
	
	// set up matrix
	v3 camera = {8.0f, 12.0f, 30.0f};

	// Camera matrix
	/*glm::mat4 View = glm::lookAt(
			glm::vec3(camera.x , camera.y, camera.z), // Camera in World Space, arranged for 60 degrees triangle vertices touching sides
			glm::vec3(8.0f, 12.0f, 0.0f), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
			);

	// Model matrix: an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);*/
	// Our ModelViewProjection: multiplication of our 3 matrices
	//mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// custom version
	/***************************/

	// Projection matrix: 45° Field of View, 4:3 ratio, display range: 0.1 unit <-> 100 units
	m4x4 P = Perspective(60.0f, (float) 800.0f / (float)600.0f, 0.1f, 100.0f);

	// Or, for an ortho camera:
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	v3 eye = {camera.x , camera.y, camera.z};	// Camera is at same as above in World Space
	v3 center = {camera.x , camera.y, 0.0f};	// and looks at the origin
	v3 up = {0.0f, 1.0f, 0.0f};		// Head is up (set to 0,-1,0 to look upside-down)

	// Camera matrix
	m4x4 V = LookAt(eye, center, up);

	// Model matrix: an identity matrix (model will be at the origin)
	m4x4 M = Identity();

	// Our ModelViewProjection: multiplication of our 3 matrices
	//mvp = P * V * M; // Remember, matrix multiplication is the other way around
	myMVP = P * V * M;
	
	/***************************/
	
    // 1. Compile Shaders
    GLuint vs = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vs, 1, &vs_src, NULL);
    glCompileShader(vs);

    GLuint fs = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fs, 1, &fs_src, NULL);
    glCompileShader(fs);

    r.shader_program = glCreateProgram();
    glAttachShader(r.shader_program, vs);
    glAttachShader(r.shader_program, fs);
    glLinkProgram(r.shader_program);

    glDeleteShader(vs);
    glDeleteShader(fs);

	// try giving MVP matrix to our shader program
	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	MatrixID = glGetUniformLocation(r.shader_program, "MVP");
	
    // 2. Setup Dynamic VAO & VBO for 6 vertices (1 quad)
    glGenVertexArrays(1, &r.vao);
    glGenBuffers(1, &r.vbo);

    glBindVertexArray(r.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * 6, NULL, GL_DYNAMIC_DRAW);

	// offsetof is defined as a macro int stddef header: #define offsetof(type, member) ((size_t)&(((type *)0)->member))
	
    // Position attribute (location = 0)
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, x));
    glEnableVertexAttribArray(0);

    // Color attribute (location = 1)
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, r));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void gl_renderer_execute(const CommandBuffer* cb) {
    glUseProgram(r.shader_program);
    glBindVertexArray(r.vao);
    glBindBuffer(GL_ARRAY_BUFFER, r.vbo);

    const uint8_t* ptr = cb->buffer;
    const uint8_t* end = cb->buffer + cb->offset;

    while (ptr < end) {
        uintptr_t raw_addr = (uintptr_t)ptr;
        ptr = (const uint8_t*)((raw_addr + 7) & ~((uintptr_t)7));
        if (ptr >= end) break;

        const CmdHeader* header = (const CmdHeader*)ptr;
        const void* payload = (const void*)(header + 1);

        switch (header->type) {
            case CMD_CLEAR: {
                const CmdClear* cmd = (const CmdClear*)payload;
                glClearColor(cmd->r, cmd->g, cmd->b, cmd->a);
                glClear(GL_COLOR_BUFFER_BIT);
                break;
            }
            case CMD_DRAW_RECTANGLE: {
                const CmdDrawRectangle* cmd = (const CmdDrawRectangle*)payload;
                float x0 = cmd->x;
                float y0 = cmd->y;
                float x1 = cmd->x + cmd->w;
                float y1 = cmd->y + cmd->h;
				
				// Send our transformation to the currently bound shader, in the "MVP" uniform
				// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
				glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &myMVP.Elements[0]);
	
                // Build 2 triangles (6 vertices)
                Vertex verts[6] = {
                    { x0, y0, cmd->r, cmd->g, cmd->b, cmd->a },
                    { x1, y0, cmd->r, cmd->g, cmd->b, cmd->a },
                    { x1, y1, cmd->r, cmd->g, cmd->b, cmd->a },
                    { x0, y0, cmd->r, cmd->g, cmd->b, cmd->a },
                    { x1, y1, cmd->r, cmd->g, cmd->b, cmd->a },
                    { x0, y1, cmd->r, cmd->g, cmd->b, cmd->a }
                };

                // Upload vertices and draw
                glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                break;
            }
        }

        ptr += header->size;
    }

    glBindVertexArray(0);
}


