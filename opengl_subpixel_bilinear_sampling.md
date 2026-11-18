When entities and the camera move across a 2D pixel grid, naive rounding causes visual "wiggling" or "shimmering" between objects.

In Handmade Hero, rounding to avoid jitter refers to the technique of keeping all game world positions in high-precision floating-point coordinates while rounding to integer pixel coordinates only at the very final rendering step—or avoiding integer snapping altogether by using sub-pixel bilinear filtering.

As noted in the Handmade Hero study notes for Day 87, integer pixel snapping can never completely eliminate slow-movement jitter on a low-resolution grid because objects moving less than one pixel per frame must eventually jump a whole pixel.

In later episodes, Handmade Hero replaces integer-snapped blitting with sub-pixel rendering:

* Bilinear Filtering. Instead of snapping sprites to integer pixel boundaries, the renderer samples textures at exact floating-point sub-pixel offsets and blends neighboring texels using bilinear interpolation.
* Smooth Motion. Because coverage smoothly transitions across pixel boundaries rather than snapping, both the camera and slow-moving entities glide without any 1-pixel jitter.

Sub-pixel bilinear sampling implementation works by projecting each screen pixel's center onto an arbitrary oriented 2D basis (Origin, XAxis, YAxis), sampling a 2×2 texel neighborhood in linear color space, and blending the result over the framebuffer using premultiplied alpha.


# BUT...

In a 2D platformer, subpixel bilinear sampling produces smooth, fluid motion and clean rotation, but it often blurs crisp pixel art and causes shimmering.

For low-res pixel art, standard bilinear sampling ruins the crisp aesthetic by blurring every pixel into a soft, smudged gradient. Instead of clean squares, sprites look washed out, and fine details like eyes or 1-pixel outlines melt into the background.

What Happens on Screen

* The "Vaseline" Smear: Because a single art pixel covers many screen pixels (e.g., 4×4 or 6×6 screen pixels on a modern display), standard bilinear filtering blends across the entire interior of every art pixel instead of just the edges.
* Shimmering During Movement: As your character walks or jumps at fractional speeds, the blending weights shift continuously. This makes static details appear to pulse, shimmer, or "breathe" as they cross screen pixel boundaries.
* Sprite Sheet Bleeding: If your sprites or tiles sit right next to each other on an atlas without padding, bilinear sampling pulls colors from neighboring sprites, creating flickering lines around your character.

How to Get Smooth Motion Without Blur

Modern pixel-art platformers (like Celeste or Shovel Knight) avoid standard bilinear filtering and use one of two techniques instead:
* Sharp Bilinear (Subpixel Anti-Aliased Upscaling): You render the game at a high resolution (like 1080p) and use a custom shader that keeps the inside of each art pixel 100% sharp while applying bilinear blending only along the 1-screen-pixel border between art pixels. You get razor-sharp pixels with silky-smooth subpixel movement and zero shimmer.
* Nearest-Neighbor with Pixel Snapping: You render the entire game onto a low-resolution canvas (e.g., 320×180), snap all sprite and camera positions to whole art pixels, and scale the final image up by an exact integer (like 6× for 1080p). Every pixel stays completely crisp, though very slow movement can look slightly stepped.

Quick Shader Formula for Sharp Bilinear

If you use a custom fragment shader with hardware bilinear filtering enabled on your texture, you can remap the UV coordinates before sampling:
1. Convert UV coordinates to texel space: texel = uv * textureSize
2. Separate the integer pixel index from the fractional offset: i = floor(texel) and f = fract(texel)
3. Compress the fractional transition zone using the screen-to-texel scale factor (scale): f = clamp((f - 0.5) * scale + 0.5, 0.0, 1.0)
4. Sample the texture at (i + f) / textureSize

In custom OpenGL, you can achieve crisp pixel art with smooth subpixel movement by combining GL_LINEAR texture filtering with a screen-space derivative shader (fwidth). This approach—often called Sharp Bilinear or Anti-Aliased Pixel Art Sampling—keeps the interior of every art pixel completely solid while blending only the 1-screen-pixel boundary between adjacent texels.

1. OpenGL Texture Setup

Set your sprite and tilemap textures to use hardware linear filtering and edge clamping so the GPU handles the final weighted blend automatically:

~~~~ C
glBindTexture(GL_TEXTURE_2D, textureID);

// Use GL_LINEAR so the shader's remapped UVs blend smoothly across 1-pixel edges
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

// Prevent wrapping artifacts at texture borders
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
~~~~


2. GLSL Fragment Shader

Using fwidth() automatically calculates how many texels a single screen pixel spans. This makes the shader work dynamically at any window resolution, camera zoom, or sprite rotation without passing manual scale uniforms.

~~~~ C
#version 330 core

in vec2 v_TexCoord;
out vec4 FragColor;

uniform sampler2D u_Texture;

void main() {
    // Get texture dimensions in texels
    vec2 texSize = vec2(textureSize(u_Texture, 0));
    
    // Convert normalized UVs to texel space (shift by 0.5 to align with texel centers)
    vec2 texel = v_TexCoord * texSize - 0.5;
    
    // Calculate how fast texel coordinates change per screen pixel
    vec2 texelRate = fwidth(texel);
    
    // Separate integer texel index from fractional position
    vec2 i = floor(texel);
    vec2 f = fract(texel);
    
    // Compress the interpolation zone to exactly 1 screen pixel wide
    f = clamp(f / max(texelRate, vec2(1e-5)), 0.0, 1.0);
    
    // Reconstruct normalized UVs and sample with hardware GL_LINEAR
    vec2 smoothUV = (i + f + 0.5) / texSize;
    FragColor = texture(u_Texture, smoothUV);
}
~~~~


3. Avoiding Atlas Bleeding & Tile Seams

Even with sharp bilinear sampling, packing sprites or tiles together requires two small precautions:
* Add 1-Pixel Padding (Extrusion) on Atlases: Because GL_LINEAR still samples neighboring texels along the 1-screen-pixel border, pad every sprite and tile in your texture atlas by 1 pixel (duplicating its edge colors). Without padding, adjacent sprites or transparent pixels will bleed into your edges.
* Use Premultiplied Alpha: Store textures with premultiplied RGB (rgb * alpha) and set glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA). Standard alpha blending (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA) blends against black (0, 0, 0, 0) transparent texels, creating dark halos around sprite silhouettes.

Why This Works Better Than Low-Res FBO Scaling

* Smooth Subpixel Physics: Characters, moving platforms, and camera pans move at native monitor resolution (e.g., 1080p/1440p) with zero judder, while art pixels remain razor-sharp.
* Clean Rotations & Zooms: Rotating a sword or zooming the camera smoothly anti-aliases diagonal pixel edges without turning the sprite into a blurry smudge.

# Sample OpenGL ES 3.0 

In OpenGL ES 3.0, you can use the exact same fwidth() sharp bilinear approach, provided you declare explicit precision qualifiers (highp) for your UV coordinates. Mobile and embedded GPUs default fragment shaders to mediump (16-bit float), which lacks the precision needed for subpixel texel math and will cause shimmering or blocky artifacts on larger sprite sheets.

1. OpenGL ES 3.0 Shader Setup (#version 300 es)


Vertex Shader

Pass your texture coordinates using highp so subpixel offsets survive interpolation across triangles:

~~~~ C
#version 300 es
precision highp float;

layout(location = 0) in vec2 a_Position;
layout(location = 1) in vec2 a_TexCoord;

uniform mat4 u_MVP;

out highp vec2 v_TexCoord;

void main() {
    gl_Position = u_MVP * vec4(a_Position, 0.0, 1.0);
    v_TexCoord = a_TexCoord;
}
~~~~


Fragment Shader

Use highp for all texel math and fwidth() derivative calculations. In OpenGL ES 3.0, fwidth() and textureSize() are built into the core language without needing extensions.

~~~~ C
#version 300 es
precision highp float;

in highp vec2 v_TexCoord;
out lowp vec4 FragColor;

uniform highp sampler2D u_Texture;

void main() {
    // Get texture dimensions as highp floats
    highp vec2 texSize = vec2(textureSize(u_Texture, 0));
    
    // Shift by 0.5 to align with texel centers
    highp vec2 texel = v_TexCoord * texSize - 0.5;
    
    // Screen-space derivative (requires highp to avoid mobile GPU jitter)
    highp vec2 texelRate = fwidth(texel);
    
    // Separate integer texel grid from fractional subpixel offset
    highp vec2 i = floor(texel);
    highp vec2 f = fract(texel);
    
    // Compress the linear blend zone to 1 screen pixel wide
    f = clamp(f / max(texelRate, vec2(1e-5)), 0.0, 1.0);
    
    // Reconstruct normalized UVs and sample with hardware GL_LINEAR
    highp vec2 smoothUV = (i + f + 0.5) / texSize;
    FragColor = texture(u_Texture, smoothUV);
}
~~~~


2. Mobile GPU Performance & Precision Tips

• Keep highp on UVs: On GPUs like Mali or Adreno, mediump only has 10 bits of mantissa precision (1024 discrete steps). On a 1024×1024 sprite atlas, mediump cannot represent any subpixel positions at all, forcing hard pixel snapping or jitter.
• Avoid Mipmaps (GL_LINEAR only): Make sure GL_TEXTURE_MIN_FILTER is set to GL_LINEAR (not GL_LINEAR_MIPMAP_LINEAR). Mipmaps blur low-res pixel art when zooming out and require extra memory bandwidth on mobile devices.
• Tilemap Seams on Subpixel Cameras: If you draw tilemaps as individual quads at fractional camera coordinates, mobile GPUs can occasionally drop a 1-pixel line between tiles due to floating-point rounding. Either render tilemaps into a single mesh with shared vertices, or extrude tile borders by 1 pixel in your atlas and inset your quad UVs by half a texel (0.5 / texSize) only if you switch back to GL_NEAREST. With the sharp bilinear shader above, do not inset UVs—just extrude tile edges by 1 pixel in the atlas texture.
