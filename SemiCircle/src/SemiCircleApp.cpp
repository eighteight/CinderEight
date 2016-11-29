#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

class YourApp : public App {
public:
    void setup() override;
    void draw() override;
    
private:
    // A batch combines a mesh with a shader, and that's all we need to draw something.
    gl::BatchRef mBatch;
};

void YourApp::setup()
{
    // Create a rectangle mesh. Try making it bigger to see how the circle remains sharp (no pixelation).
    auto mesh = gl::VboMesh::create( geom::Rect( { -128, -128, 128, 128 } ) ); // x1, y1, x2, y2
    
    // Create our shader. Instead of having the source in this C++ file,
    // we could also have chosen to load text files from the assets folder.
    gl::GlslProg::Format fmt;
    fmt.version( 150 );
    
    fmt.vertex(
               "uniform mat4 ciModelViewProjection;\n" // Cinder will pass this matrix to the shader for us.
               ""
               "in vec4 ciPosition;\n"  // Cinder will pass the mesh vertices to the shader for us.
               "in vec2 ciTexCoord0;\n" // Cinder will pass the mesh texture coordinates to the shader for us.
               "in vec4 ciColor;\n"     // Cinder will pass the vertex colors to the shader for us.
               ""
               "out vec2 vertTexCoord0;\n" // Our vertex shader will pass texture coordinates to the fragment shader.
               "out vec4 vertColor;\n"     // Our vertex shader will pass vertex colors to the fragment shader.
               ""
               "void main( void )\n"
               "{\n"
               "    vertTexCoord0 = ciTexCoord0;\n"
               "    vertColor = ciColor;\n"
               ""
               "    gl_Position = ciModelViewProjection * ciPosition;\n" // Required by OpenGL: it's the task of the vertex shader to convert from object to clip space.
               "}" );
    
    fmt.fragment(
                 "const float kInvPi = 1.0 / 3.14159265;\n"
                 
                 // Play with the following values to see their effect.
                 "const float kBluriness = 1.0;\n"
                 "const float kRadius = 0.75;\n"
                 "const float kThickness = 0.2;\n"
                 "const float kArc = 0.5;\n"
                 "const float kOffset = 0.25;\n"
                 
                 "in vec2 vertTexCoord0;\n" // Passed to us from the vertex shader and interpolated for this fragment (pixel).
                 "in vec4 vertColor;\n"     // Passed to us from the vertex shader and interpolated for this fragment (pixel).
                 
                 "out vec4 fragColor;\n" // The output of our fragment shader is a color for this fragment (pixel).
                 
                 "void main( void )\n"
                 "{\n"
                 // Convert texture coordinates from range [0,1] to [-1,1]
                 "    vec2 uv = 2.0 * vertTexCoord0 - 1.0;\n"
                 
                 // Calculate distance to (0,0).
                 "    float d = length( uv );\n"
                 
                 // Calculate angle, so we can draw segments, too.
                 "    float angle = atan( uv.x, uv.y ) * kInvPi * 0.5;\n"
                 "    angle = fract( angle - kOffset );\n"
                 
                 // Create an anti-aliased circle.
                 "    float w = kBluriness * fwidth( d );\n"
                 "    float circle = smoothstep( kRadius + w, kRadius - w, d );\n"
                 
                 // Optionally, you could create a hole in it:
                 "    float inner = kRadius - kThickness;\n"
                 "    circle -= smoothstep( inner + w, inner - w, d );\n"
                 
                 // Or only draw a portion (segment) of the circle.
                 "    float segment = step( angle, kArc );\n"
                 "    segment *= step( 0.0, angle );\n"
                 "    circle *= mix( segment, 1.0, step( 1.0, kArc ) );\n"
                 
                 // Output final color.
                 "    fragColor.a = vertColor.a * circle;\n"
                 "    fragColor.rgb = vertColor.rgb * fragColor.a;\n"
                 "}" );
    
    auto glsl = gl::GlslProg::create( fmt );
    
    mBatch = gl::Batch::create( mesh, glsl );
}

void YourApp::draw()
{
    gl::clear();
    
    gl::ScopedColor        scpColor( 1.0f, 0.4f, 0.1f ); // Set color to orange.
    gl::ScopedBlendPremult scpBlend;                     // Enable premultiplied alpha blending.
    
    gl::ScopedModelMatrix scpModel;       // Allows us to restore transformations after we're done.
    gl::translate( getWindowSize() / 2 ); // Try disabling this line to see what it does.
    mBatch->draw();                       // Draw our mesh with the supplied shader.
    
    gl::drawStrokedRect( { -128, -128, 128, 128 } ); // Draws the border of our rectangle.
}

CINDER_APP( YourApp, RendererGl )