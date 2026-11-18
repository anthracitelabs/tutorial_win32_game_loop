When entities and the camera move across a 2D pixel grid, naive rounding causes visual "wiggling" or "shimmering" between objects.

In Handmade Hero, rounding to avoid jitter refers to the technique of keeping all game world positions in high-precision floating-point coordinates while rounding to integer pixel coordinates only at the very final rendering step—or avoiding integer snapping altogether by using sub-pixel bilinear filtering.

As noted in the Handmade Hero study notes for Day 87, integer pixel snapping can never completely eliminate slow-movement jitter on a low-resolution grid because objects moving less than one pixel per frame must eventually jump a whole pixel.

In later episodes, Handmade Hero replaces integer-snapped blitting with sub-pixel rendering:

* Bilinear Filtering. Instead of snapping sprites to integer pixel boundaries, the renderer samples textures at exact floating-point sub-pixel offsets and blends neighboring texels using bilinear interpolation.
* Smooth Motion. Because coverage smoothly transitions across pixel boundaries rather than snapping, both the camera and slow-moving entities glide without any 1-pixel jitter.

Sub-pixel bilinear sampling implementation works by projecting each screen pixel's center onto an arbitrary oriented 2D basis (Origin, XAxis, YAxis), sampling a 2×2 texel neighborhood in linear color space, and blending the result over the framebuffer using premultiplied alpha.


CPU packs all quad vertices for the entire frame into a single memory buffer, uploads it once per frame to a Vertex Buffer Object (VBO), and lets a GLSL vertex and fragment shader pair execute the transform, bilinear texture lookup, and color modulation in parallel on the GPU.


1. Vertex Layout & Single-Upload VBO (CPU → GPU)

First, define a compact vertex struct that holds the floating-point position, (U, V) texture coordinate, and premultiplied tint color:

~~~~ C
struct textured_vertex
{
    v3 P;       // Floating-point world/screen position
    v2 UV;      // Sub-pixel aligned texture coordinates
    uint32 Color; // Packed RGBA8 tint color
};
~~~~

Each frame, as the CPU iterates through the render command buffer, it appends 4 vertices per quad into a contiguous CPU-side VertexArray (and 6 indices 0, 1, 2, 0, 2, 3 into an IndexArray), then streams both arrays to the GPU in a single upload:

~~~~ C
// Bind VAO and upload all frame vertices in one batch
glBindVertexArray(OpenGL.VertexArray);
glBindBuffer(GL_ARRAY_BUFFER, OpenGL.VertexBuffer);
glBufferData(GL_ARRAY_BUFFER, VertexCount * sizeof(textured_vertex),
             Vertices, GL_STREAM_DRAW);

glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OpenGL.IndexBuffer);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, IndexCount * sizeof(uint32),
             Indices, GL_STREAM_DRAW);

// Describe vertex attributes to the vertex shader
glEnableVertexAttribArray(0); // P
glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(textured_vertex),
                      (void *)offsetof(textured_vertex, P));

glEnableVertexAttribArray(1); // UV
glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(textured_vertex),
                      (void *)offsetof(textured_vertex, UV));

glEnableVertexAttribArray(2); // Color (normalized 0..255 -> 0.0..1.0)
glVertexAttribPointer(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(textured_vertex),
                      (void *)offsetof(textured_vertex, Color));
~~~~


2. GLSL Vertex Shader

The vertex shader replaces the fixed-function GL_PROJECTION matrix multiply. It transforms each floating-point vertex position into clip space and passes the interpolated UV and Color forward to the fragment shader:

~~~~ C
#version 330 core

layout(location = 0) in vec3 VertP;
layout(location = 1) in vec2 VertUV;
layout(location = 2) in vec4 VertColor;

uniform mat4 Transform;

out vec2 FragUV;
out vec4 FragColor;

void main()
{
    gl_Position = Transform * vec4(VertP, 1.0);
    FragUV = VertUV;
    FragColor = VertColor;
}
~~~~


3. GLSL Fragment Shader (Hardware Bilinear Sampling)

The fragment shader runs once for every covered pixel center (X + 0.5, Y + 0.5). Calling texture(TextureSampler, FragUV) invokes the GPU's dedicated texture filtering hardware—performing the exact 2×2 sub-pixel bilinear interpolation and sRGB-to-linear conversion automatically:

~~~~ C
#version 330 core

in vec2 FragUV;
in vec4 FragColor;

uniform sampler2D TextureSampler;

out vec4 OutColor;

void main()
{
    // Hardware performs sRGB -> Linear conversion and 2x2 bilinear filtering
    vec4 Texel = texture(TextureSampler, FragUV);

    // Modulate by vertex tint color (equivalent to Hadamard(Texel, Color))
    OutColor = Texel * FragColor;
}
~~~~


4. Batched Draw Calls

With the VBO bound and shader active, the CPU only issues a new glDrawElements call whenever the active texture handle changes—instead of starting and ending a draw call for every individual sprite:


~~~~ C
glUseProgram(OpenGL.Program);
glUniformMatrix4fv(OpenGL.TransformLoc, 1, GL_FALSE, Proj);

// Draw a contiguous run of quads sharing the same texture in one call
glBindTexture(GL_TEXTURE_2D, CurrentTextureHandle);
glDrawElements(GL_TRIANGLES, RunIndexCount, GL_UNSIGNED_INT,
               (void *)(StartIndex * sizeof(uint32)));
~~~~


Why This Preserves Exact Sub-Pixel Behavior

* Full Floating-Point Precision. VertP remains vec3 (float32), so sub-pixel positions never snap to an integer grid before rasterization.
* Perspective-Correct Smooth UV Interpolation. The rasterizer interpolates FragUV smoothly across every sub-pixel center inside the quad, feeding continuous fractional coordinates directly into texture().
* Hardware Blend & Gamma Pipeline. Because GL_FRAMEBUFFER_SRGB and glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA) remain enabled on the framebuffer, OutColor is still blended in linear space with premultiplied alpha just like DrawRectangleSlowly.
