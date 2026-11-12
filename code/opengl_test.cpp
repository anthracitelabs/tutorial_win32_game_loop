#include "game_math.h"

// 1. Hardcoded Shaders (GLSL 330 Core)
const char* VertexShaderSource = 
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
	"// Values that stay constant for the whole mesh.\n"
	"uniform mat4 MVP;\n"
	"\n"
    "void main() {\n"
	"// Output position of the vertex, in clip space: MVP * position\n"
    "   gl_Position = MVP * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n\0";

const char* FragmentShaderSource = 
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n" // Orange color
    "}\n\0";

// 2. Setup Code (Run Once After Context Initialization)
GLuint ShaderProgram = 0;
GLuint VAO = 0;
GLuint VBO = 0;

m4x4 myMVP;
// try giving MVP matrix to our shader program
// Get a handle for our "MVP" uniform
// Only during the initialisation
GLuint MatrixID;

void InitRenderingTest() 
{
	// matrix math test ****************************
	/*	screen resolution : 800x600
	 *	World position : (2.0, 1.0, -5.0, 1.0) homogenous 4D coordinates
	 *	w = 1.0 means this is a position not a direction
	 */
	v4 Pworld = {10.0f, 10.0f, 10.0f, 1.0f};
	m4x4 T  = { 1.0f, 0.0f, 0.0f, 0.0f,		//| 0 - 3 | First column (typically X-axis basis vector)
		0.0f, 1.0f, 0.0f, 0.0f,		//| 4 – 7	| Second column (typically Y-axis basis vector)
		0.0f, 0.0f, 1.0f, 0.0f,		//| 8 – 11 | Third column (typically Z-axis basis vector)
		10.0f, 0.0f, 0.0f, 1.0f};	//| 12 – 15 | Fourth column (Translation components X, Y, Z)

	// translate vector 10 units
	v4 Pnew = Multiply(T, Pworld);
	/* and we get a (20,10,10,1) homogeneous vector! Remember, the 1 means that it is a position, not a direction. So our transformation didn’t change the fact that we were dealing with a position, which is good.
	 */

	// Let’s now see what happens to a vector that represents a direction towards the -z axis: (0,0,-1,0)
	v4 Pdir = {0.0f, 0.0f, -1.0f, 0.0f};
	v4 Pdir_translated = Multiply(T, Pdir);
	// result is our original (0,0,-1,0) direction, which is great because, as I said ealier, moving a direction does not make sense.


	//GLM example to reproduce-----
	// Projection matrix: 45° Field of View, 4:3 ratio, display range: 0.1 unit <-> 100 units
	//Projection = glm::perspective(glm::radians(60.0f), (float) 640.0f / (float)480.0f, 0.1f, 100.0f);

	// Or, for an ortho camera:
	//glm::mat4 Projection = glm::ortho(-10.0f,10.0f,-10.0f,10.0f,0.0f,100.0f); // In world coordinates

	v3 vertex1 = {0.0f, 0.0f, 0.0f};	// left
	v3 vertex2 = {16.0f, 0.0f, 0.0f};	// right
	v3 vertex3 = {8.0f, 24.0f, 0.0f};	// top
	float tri_height = vertex3.y - vertex1.y;
	float tri_base = vertex2.x - vertex1.x;
	//float d =  SquareRoot(tri_height * tri_height - (tri_height / 2.0) * (tri_height / 2.0));

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

	float Vertices[] = {
		vertex1.x, vertex1.y, vertex1.z,  // Left  
		vertex2.x, vertex2.y, vertex2.z,  // Right  
		vertex3.x, vertex3.y, vertex3.z,  // Top  
	};

	// Dynamically load missing state setup functions if not done already
	glGenVertexArrays_type* glGenVertexArrays = (glGenVertexArrays_type*)wglGetProcAddress("glGenVertexArrays");
	glBindVertexArray_type* glBindVertexArray = (glBindVertexArray_type*)wglGetProcAddress("glBindVertexArray");
	glVertexAttribPointer_type* glVertexAttribPointer = (glVertexAttribPointer_type*)wglGetProcAddress("glVertexAttribPointer");
	glEnableVertexAttribArray_type* glEnableVertexAttribArray = (glEnableVertexAttribArray_type*)wglGetProcAddress("glEnableVertexAttribArray");

	// Compile Vertex Shader
	GLuint VertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(VertexShader, 1, &VertexShaderSource, NULL);
	glCompileShader(VertexShader);

	// Compile Fragment Shader
	GLuint FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &FragmentShaderSource, NULL);
	glCompileShader(FragmentShader);

	// Link Shaders into Program
	ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
	glLinkProgram(ShaderProgram);

	// Clean up individual shaders as they are now linked
	// (In a real engine like Handmade Hero, you'd check compilation/link flags here)
	// glDeleteShader(VertexShader);
	// glDeleteShader(FragmentShader);


	// try giving MVP matrix to our shader program
	// Get a handle for our "MVP" uniform
	// Only during the initialisation
	MatrixID = glGetUniformLocation(ShaderProgram, "MVP");

	// Generate and Bind Vertex Array Object (VAO) and Vertex Buffer Object (VBO)
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

	// Map vertex attributes to location 0 in the vertex shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Unbind buffer and array to prevent accidental modifications
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

// 3. Execution Code (Run Every Frame Inside the Main Loop)
void RenderTestFrame(game_state* state, float alpha) {
	
	float x = Lerp(state->PlayerOldPosition.x * state->PixelsPerMeter, alpha, state->PlayerPosition.x * state->PixelsPerMeter);
	float y = Lerp(state->PlayerOldPosition.y * state->PixelsPerMeter, alpha, state->PlayerPosition.y * state->PixelsPerMeter);
	
	v3 camera = {state->CameraPosition.x, state->CameraPosition.y, 30.0f};

	// Camera matrix
	/*glm::mat4 View = glm::lookAt(
			glm::vec3(camera.x , camera.y, camera.z), // Camera in World Space, arranged for 60 degrees triangle vertices touching sides
			glm::vec3(camera.x , camera.y, 0.0f), // and looks at the origin
			glm::vec3(0, 1, 0)  // Head is up (set to 0,-1,0 to look upside-down)
			);

	// Model matrix: an identity matrix (model will be at the origin)
	glm::mat4 Model = glm::mat4(1.0f);
	// Our ModelViewProjection: multiplication of our 3 matrices
	mvp = Projection * View * Model; // Remember, matrix multiplication is the other way around
*/
	// Clear screen to a dark grey color
	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Use our compiled shader program and bind geometry
	glUseProgram(ShaderProgram);

	// Send our transformation to the currently bound shader, in the "MVP" uniform
	// This is done in the main loop since each model will have a different MVP matrix (At least for the M part)
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &myMVP.Elements[0]);
	//glUniformMatrix4fv(MatrixID, 1, GL_FALSE, glm::value_ptr(mvp));

	// Use the function loaded dynamically in step 2
	glBindVertexArray_type* glBindVertexArray = (glBindVertexArray_type*)wglGetProcAddress("glBindVertexArray");
	glBindVertexArray(VAO);

	// Draw the 3 vertices as a triangle
	glDrawArrays(GL_TRIANGLES, 0, 3);

	// Swap buffers to display the rendered image (Standard Win32 API call)
	//SwapBuffers(WindowDC);
}
