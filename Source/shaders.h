static const char* moonVertexShader = R"(
#version 330 core
layout (location = 0) in vec4 position;

void main() {
    gl_Position = position;
})";

static const char* moonGeometryShader = R"(
#version 330 core
#define WAVE_RADIUS 0.008
#define WAVE_GIRTH_RESOLUTION 12
#define PI 3.1415926538
#define GIRTH_ANGLE_OFFSET (2.0 * PI / float (WAVE_GIRTH_RESOLUTION))
layout (triangles) in;
layout (triangle_strip, max_vertices = 128) out;
uniform float blend;
uniform vec3 colour;
uniform float wtPosition;
uniform mat4 matrix;
out vec4 fColour;
out vec2 texCoord;

void emit(float x, float y, float z) { texCoord.x = x/2.0 + 0.5; gl_Position = matrix * vec4(x, y, z, 1.0); EmitVertex(); }
void emit(vec3 v) { emit(v.x, v.y, v.z); }
void emit(float x, float y) { emit(x, y, wtPosition); }
void emit(vec2 v) { emit(v.x, v.y); }

void extrude (in vec3 sliceOrigin, in int girthIndex, out vec3 extrapolation) {
    float zOffset = WAVE_RADIUS * cos (-float (girthIndex) * GIRTH_ANGLE_OFFSET);
    float yOffset = WAVE_RADIUS * sin (-float (girthIndex) * GIRTH_ANGLE_OFFSET);
    extrapolation = vec3 (sliceOrigin.x, sliceOrigin.y + yOffset, sliceOrigin.z + zOffset);
}

void drawWrapSegment(vec3 left, vec3 right) {
    if (left.y != 0.0 || right.y != 0.0) {
        fColour = vec4(colour, blend);
        texCoord.y = 0.0;
        vec3 leftCorner = vec3(left.x, left.y, left.z + WAVE_RADIUS);
        vec3 rightCorner = vec3(right.x, right.y, right.z + WAVE_RADIUS);
        
        emit(leftCorner);
        for (int i = 1; i <= WAVE_GIRTH_RESOLUTION; ++i) {
            emit(rightCorner);
            texCoord.y = float(i)/float(WAVE_GIRTH_RESOLUTION);
            extrude(left, i, leftCorner);
            extrude(right, i, rightCorner);
            emit(leftCorner);
        }
        emit(rightCorner);
        EndPrimitive();
    }
}

void main() {
    vec3 left = vec3(gl_in[0].gl_Position.x, gl_in[0].gl_Position.y, wtPosition);
    vec3 right = vec3(gl_in[2].gl_Position.x, gl_in[2].gl_Position.y, wtPosition);
    drawWrapSegment(left, right);
} )";

static const char* moonFragmentShader = R"(
#version 330 core
in vec4 fColour;
uniform sampler2D demoTexture;
in vec2 texCoord;
out vec4 color;
void main() {
    vec4 t = fColour * texture (demoTexture, texCoord);
    color = vec4(t.rgb, fColour.a);
})";


/////////////////////////////////////////////////////////

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
uniform float wtPosition;
uniform mat4 matrix;
float lineHeight, dispersionHeight = 0.002;
out vec4 fColour;

void emit(float x, float y, float z) { gl_Position = matrix * vec4(x, y, z, 1.0); EmitVertex(); }
void emit(vec3 v) { emit(v.x, v.y, v.z); }
void emit(float x, float y) { emit(x, y, wtPosition); }
void emit(vec2 v) { emit(v.x, v.y); }

void drawStrip(vec2 left, vec2 right) {
    if (left.y != 0.0 || right.y != 0.0) {
        fColour = vec4(colour, blend * ALPHA);
        emit(left);
        emit(right);
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        emit(left.x, 0.0);
        emit(right.x, 0.0);
        EndPrimitive();
    }
}

void drawLine(vec2 left, vec2 right) {
    if (left.y != 0.0 || right.y != 0.0) {
        fColour = vec4(colour, blend);
        emit(left.x, left.y + lineHeight);
        emit(left.x, left.y - lineHeight);
        emit(right.x, right.y + lineHeight);
        emit(right.x, right.y - lineHeight);
        EndPrimitive();
        
        vec4 whiterColour = vec4(min(colour.r + 0.3, 1.0), min(colour.g + 0.3, 1.0), min(colour.b + 0.3, 1.0), blend);
        
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        emit(left.x, left.y + lineHeight + dispersionHeight);
        emit(right.x, right.y + lineHeight + dispersionHeight);
        fColour = whiterColour;
        emit(left.x, left.y + lineHeight);
        emit(right.x, right.y + lineHeight);
        EndPrimitive();
        
        emit(left.x, left.y - lineHeight);
        emit(right.x, right.y - lineHeight);
        fColour = vec4(0.0, 0.0, 0.0, 0.0);
        emit(left.x, (left.y - lineHeight) - dispersionHeight);
        emit(right.x, (right.y - lineHeight) - dispersionHeight);
        EndPrimitive();
    }
}

void main() {
    lineHeight = 0.002;
    vec2 left = gl_in[0].gl_Position.xy;
    vec2 right = gl_in[2].gl_Position.xy;
    drawLine(left, right);
} )";

static const char* waveFragmentShader = R"(
#version 330 core
in vec4 fColour;
out vec4 color;
void main() {
    color = fColour;
})";
