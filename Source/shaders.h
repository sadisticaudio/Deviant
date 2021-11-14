static const char* waveVertexShader = R"(
#version 330 core
layout (location = 0) in vec4 position;

void main() {
    gl_Position = position;
})";

static const char* waveGeometryShader = R"(
#version 330 core
#define ALPHA 0.8
layout (triangles) in;
layout (triangle_strip, max_vertices = 24) out;
uniform float blend;
uniform vec3 colour;
float lineHeight, dispersionHeight = 0.03;
out vec4 fColour;

void drawStrip(vec2 topLeft, vec2 topRight) {
    if (topLeft.y != 0.0 || topRight.y != 0.0) {
        fColour = vec4(colour, blend * ALPHA);
        gl_Position = vec4(topLeft, 0.0, 1.0);
        EmitVertex();
        gl_Position = vec4(topLeft.x, topLeft.y, 0.0, 1.0);
        EmitVertex();
        fColour = vec4(colour, blend * ALPHA);
        gl_Position = vec4(topRight, 0.0, 1.0);
        EmitVertex();
        gl_Position = vec4(topRight.x, topRight.y, 0.0, 1.0);
        EmitVertex();
        EndPrimitive();
        
        gl_Position = vec4(topLeft.x, topLeft.y, 0.0, 1.0);
        EmitVertex();
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        gl_Position = vec4(topLeft.x, 0.0, 0.0, 1.0);
        EmitVertex();
        fColour = vec4(colour, blend * ALPHA);
        gl_Position = vec4(topRight.x, topRight.y, 0.0, 1.0);
        EmitVertex();
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        gl_Position = vec4(topRight.x, 0.0, 0.0, 1.0);
        EmitVertex();
        EndPrimitive();
    }
}

void drawLine(vec2 left, vec2 right) {
    if (left.y != 0.0 || right.y != 0.0) {
        fColour = vec4(colour, blend);
        gl_Position = vec4(left.x, left.y + lineHeight, 0.0, 1.0);
        EmitVertex();
        gl_Position = vec4(left.x, left.y - lineHeight, 0.0, 1.0);
        EmitVertex();
        gl_Position = vec4(right.x, right.y + lineHeight, 0.0, 1.0);
        EmitVertex();
        gl_Position = vec4(right.x, right.y - lineHeight, 0.0, 1.0);
        EmitVertex();
        EndPrimitive();
        
        vec4 whiterColour = vec4(max(colour.r + 0.3, 1.0), max(colour.g + 0.3, 1.0), max(colour.b + 0.3, 1.0), blend);
        
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        gl_Position = vec4(left.x, left.y + lineHeight + dispersionHeight, 0.0, 1.0);
        EmitVertex();
        fColour = whiterColour;
        gl_Position = vec4(left.x, left.y + lineHeight, 0.0, 1.0);
        EmitVertex();
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        gl_Position = vec4(right.x, right.y + lineHeight + dispersionHeight, 0.0, 1.0);
        EmitVertex();
        fColour = whiterColour;
        gl_Position = vec4(right.x, right.y + lineHeight, 0.0, 1.0);
        EmitVertex();
        EndPrimitive();
        
        fColour = whiterColour;
        gl_Position = vec4(left.x, left.y - lineHeight, 0.0, 1.0);
        EmitVertex();
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        gl_Position = vec4(left.x, (left.y - lineHeight) - dispersionHeight, 0.0, 1.0);
        EmitVertex();
        fColour = whiterColour;
        gl_Position = vec4(right.x, right.y - lineHeight, 0.0, 1.0);
        EmitVertex();
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        gl_Position = vec4(right.x, (right.y - lineHeight) - dispersionHeight, 0.0, 1.0);
        EmitVertex();
        EndPrimitive();
    }
}

void main() {
    lineHeight = 0.01;
    float leftX = gl_in[0].gl_Position.x;
    float rightX = gl_in[2].gl_Position.x;
    float leftY = gl_in[0].gl_Position.y;
    float rightY = gl_in[2].gl_Position.y;
    drawStrip(vec2(leftX, leftY), vec2(rightX, rightY));
    drawLine(vec2(leftX, leftY), vec2(rightX, rightY));
} )";

static const char* waveFragmentShader = R"(
#version 330 core
in vec4 fColour;
out vec4 color;
void main() {
    color = fColour;
})";


/////////////////////////////////////////////////////////

static const char* rmsVertexShader = R"(
#version 330 core
layout (location = 0) in vec3 position;
uniform float waveData[256];
uniform vec3 colour;
out vec4 fColour;
void main() {
    fColour = vec4(colour, 1.0);
    if(waveData[0] != 0.0) {
        gl_Position = vec4(position, 1.0);
    }
})";

static const char* rmsFragmentShader = R"(
#version 330 core
uniform float blend;
uniform vec2 resolution;
uniform float waveData[256];
in vec4 fColour;
out vec4 color;

void getAmplitudeForXPos (in float xPos, out float audioAmplitude)
{
// Buffer size - 1
   float perfectSamplePosition = 255.0 * xPos / resolution.x;
   int leftSampleIndex = int (floor (perfectSamplePosition));
   int rightSampleIndex = int (ceil (perfectSamplePosition));
   audioAmplitude = mix (waveData[leftSampleIndex], waveData[rightSampleIndex], fract (perfectSamplePosition));
}

#define THICKNESS 0.01
void main()
{
    float y = gl_FragCoord.y / resolution.y;
    
    float amplitude = 0.0;
    getAmplitudeForXPos (gl_FragCoord.x, amplitude);

// Centers & Reduces Wave Amplitude
    amplitude = 0.5 + amplitude / 4.0;
    float r = abs (THICKNESS / (amplitude-y));

    color = vec4 ((1.0 + fColour.r)/2.0, (1.0 + fColour.g)/2.0, (1.0 + fColour.b)/2.0, clamp((r - abs (r * 0.2)) * blend, 0.0, 0.9));
})";
