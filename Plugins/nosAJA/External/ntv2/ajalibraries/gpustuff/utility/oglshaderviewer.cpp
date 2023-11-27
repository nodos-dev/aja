/* SPDX-License-Identifier: MIT */
//---------------------------------------------------------------------------------------------------------------------
//  oglshaderviewer.cpp
//
//	Copyright (C) 2012 AJA Video Systems, Inc.  Proprietary and Confidential information.  All rights reserved.
//---------------------------------------------------------------------------------------------------------------------
#include "../include/gl/glew.h"
#include "oglshaderviewer.h"
#include "ajabase/system/systemtime.h"
#include "ajabase/common/videoutilities.h"

#include <QDate>
#include <QThread>
#include <string>
#include <sstream>
#include <iomanip>


#include "ntv2debug.h"


//
// Shader Definitions
//

//
// Computer Shader: Swizzle422To444YCrCbToRGBCS 
//
// Convert incoming big endian 422 sampled YCrCb image data to
// little endian 444 sampled RGB.  Input image is single component
// 32-bit uint.  Output image is four component uint. 
//
const char *Swizzle422To444YCrCbToRGBCS = 
	"#version 430\n"
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
	"#extension GL_ARB_compute_shader : enable\n"
	"\n"
	"layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;\n"
	"\n"
	"layout(r32ui, location = 0) uniform uimage2D inTex;\n"
	"layout(rgb10_a2ui, location = 1) uniform uimage2D outTex;\n"
	"\n"
	"void main() {\n"
	"   ivec2 outPos = ivec2(gl_GlobalInvocationID.xy);\n"
	"   uint index = gl_WorkGroupID.x * 4 + gl_LocalInvocationID.x;\n"
	"   \n"
	"   uint texel1;"
	"   uint texel2;\n"
	"   uvec4 color2;\n"
	"   \n"
	"   // Swizzle from Big Endian to Little Endian and expand to 444 \n"
	"   if (bool(gl_LocalInvocationID.x == 0)) {\n"
	"       texel1 = imageLoad(inTex, ivec2(index, gl_GlobalInvocationID.y));\n"
	"		color2.r = ((texel1 & 0xFF)<<2) + ((texel1 & 0xC000)>>14); // Cr - P1 \n"
	"		color2.g = ((texel1 & 0x3F00)>>4) + ((texel1 & 0xF00000)>>20); // G - P2 \n"
	"		color2.b = ((texel1 & 0xFC000000)>>26) + ((texel1 & 0xF0000)>>10); // Cb - P3 \n" 
	"   } else if (bool(gl_LocalInvocationID.x == 1)) {\n"
	"       texel1 = imageLoad(inTex, ivec2(index, gl_GlobalInvocationID.y));\n"
	"       texel2 = imageLoad(inTex, ivec2(index - 1, gl_GlobalInvocationID.y));\n"
	"		color2.r = ((texel1 & 0x3F00)>>4) + ((texel1 & 0xF00000)>>20); // Cr - P2 \n"
	"		color2.g = ((texel1 & 0xFF)<<2) + ((texel1 & 0xC000)>>14);  // Y - P1 \n" 
	"		color2.b = ((texel2 & 0xFC000000)>>26) + ((texel2 & 0xF0000)>>10); // Cb - P3 \n" 
	"   } else if (bool(gl_LocalInvocationID.x == 2)) {\n"
	"       texel1 = imageLoad(inTex, ivec2(index, gl_GlobalInvocationID.y));\n"
	"       texel2 = imageLoad(inTex, ivec2(index - 1, gl_GlobalInvocationID.y));\n"
	"		color2.r = ((texel2 & 0x3F00)>>4) + ((texel2 & 0xF00000)>>20); // Cr - P2 \n"
	"		color2.g = ((texel2 & 0xFC000000)>>26) + ((texel2 & 0xF0000)>>10); // G - P3 \n" 
	"		color2.b = ((texel1 & 0xFF)<<2) + ((texel1 & 0xC000)>>14); // Cb - P1 \n"
	"   } else if (bool(gl_LocalInvocationID.x == 3)) {\n"
	"       texel2 = imageLoad(inTex, ivec2(index - 1, gl_GlobalInvocationID.y));\n"
	"		color2.r = ((texel2 & 0xFC000000)>>26) + ((texel2 & 0xF0000)>>10);  // Cr - P3 \n" 
	"		color2.g = ((texel2 & 0x3F00)>>4) + ((texel2 & 0xF00000)>>20);  // Y - P2 \n"
	"		color2.b = ((texel2 & 0xFF)<<2) + ((texel2 & 0xC000)>>14);  // Cb - P1 \n"
	"   } else if (bool(gl_LocalInvocationID.x == 4)) {\n"
	"       texel1 = imageLoad(inTex, ivec2(index - 1, gl_GlobalInvocationID.y));\n"
	"       texel2 = imageLoad(inTex, ivec2(index - 2, gl_GlobalInvocationID.y));\n"
	"		color2.r = ((texel2 & 0xFC000000)>>26) + ((texel2 & 0xF0000)>>10);  // Cr - P3 \n"
	"		color2.g = ((texel1 & 0xFF)<<2) + ((texel1 & 0xC000)>>14);  // Y - P1 \n"
	"		color2.b = ((texel1 & 0x3F00)>>4) + ((texel1 & 0xF00000)>>20); // Cb - P2 \n"
	"   } else if (bool(gl_LocalInvocationID.x == 5)) {\n"
	"       texel1 = imageLoad(inTex, ivec2(index - 1, gl_GlobalInvocationID.y));\n"
	"       texel2 = imageLoad(inTex, ivec2(index - 2, gl_GlobalInvocationID.y));\n"
	"		color2.r = ((texel1 & 0xFF)<<2) + ((texel1 & 0xC000)>>14); // Cr - P1 \n"
	"		color2.g = ((texel2 & 0xFC000000)>>26) + ((texel2 & 0xF0000)>>10); // Y - P3 \n"
	"		color2.b = ((texel2 & 0x3F00)>>4) + ((texel2 & 0xF00000)>>20); // Cb - P2 \n"
	"   }\n"
	"	color2.a = 0;\n"
	"\n"
	"   // Convert to float, perform color space conversion and store 4-component RGB result as uints \n"
	"   vec4 color3 = vec4(color2);\n"
	"   color2.r = uint(1.164 * (color3.g - 64.0) + 1.596 * (color3.r - 512.0));\n"
	"   color2.g = uint(1.164 * (color3.g - 64.0) - 0.813 * (color3.r - 512.0) - 0.392 * (color3.b - 512.0));\n"
	"   color2.b = uint(1.164 * (color3.g - 64.0)                        + 2.017 * (color3.b - 512.0));\n"
	"   imageStore(outTex, outPos, color2); \n"
	"}\n"
	"";

//
// Computer Shader: Swizzle444RGBToRGBCS 
//
// Convert incoming big endian 444 sampled RGB image data to
// little endian 444 sampled RGB.  Input image is single component
// 32-bit uint.  Output image is four component uint. 
//
const char *Swizzle444RGBToRGBCS = 
	"#version 430\n"
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
	"#extension GL_ARB_compute_shader : enable\n"
	"\n"
	"layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;\n"
	"\n"
	"layout(r32ui, location = 0) uniform uimage2D inTex;\n"
	"layout(rgb10_a2ui, location = 1) uniform uimage2D outTex;\n"
	"\n"
	"void main() {\n"
	"   ivec2 outPos = ivec2(gl_GlobalInvocationID.xy);\n"
	"   ivec2 inPos = ivec2(gl_GlobalInvocationID.xy);\n"
	"   \n"
	"   uint texel;"
	"   uvec4 color2;\n"
	"   \n"
	"   // Swizzle from Big Endian to Little Endian \n"
	"   texel = imageLoad(inTex, inPos);\n"
	"	color2.r = ((texel & 0xFC000000)>>26) + ((texel & 0xF0000)>>10); \n"
	"	color2.g = ((texel & 0x3F00)>>4) + ((texel & 0xF00000)>>20); \n"
	"	color2.b = ((texel & 0xFF)<<2) + ((texel & 0xC000)>>14); \n"
	"	color2.a = 0;\n"
	"   \n"
	"   // Store 4-component RGB result as uint \n"
	"   imageStore(outTex, outPos, color2); \n"
	"}\n"
	"";

//
// Computer Shader: UintToFourComponentCS 
//
// Convert incoming 444 sampled RGB image data with all three 10-bit
// components stored as a single 32-bit uint into a four-component uint. 
//
const char *UintToFourComponent = 
	"#version 430\n"
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
	"#extension GL_ARB_compute_shader : enable\n"
	"\n"
	"layout(local_size_x = 6, local_size_y = 1, local_size_z = 1) in;\n"
	"\n"
	"layout(r32ui, location = 0) uniform uimage2D inTex;\n"
	"layout(rgb10_a2ui, location = 1) uniform uimage2D outTex;\n"
	"\n"
	"void main() {\n"
	"   ivec2 outPos = ivec2(gl_GlobalInvocationID.xy);\n"
	"   ivec2 inPos = ivec2(gl_GlobalInvocationID.xy);\n"
	"   \n"
	"   uint texel;"
	"   uvec4 color2;\n"
	"   \n"
	"   // Components are in the upper 30 bits. \n"
	"   texel = imageLoad(inTex, inPos);\n"
	"	color2.r = (texel & 0xffC) >> 2; \n"
	"	color2.g = (texel & 0x3FF000) >> 12; \n"
	"	color2.b = (texel & 0xFFC00000) >> 22; \n"
	"	color2.a = 0;\n"
	"   \n"
	"   // Store 4-component RGB result as uint \n"
	"   imageStore(outTex, outPos, color2); \n"
	"}\n"
	"";

//
// Vertex Shader: VertexShader 
//
// Simple vertex shader to map the vertex coordinates by the projection
// and view matrices and simply pass through the texture coordinates to
// the bound fragment shader.
//
const char *VertexShader =
	"#version 400\n"
	"\n"
	"in vec3 in_vert_coord;\n"
	"in vec2 in_tex_coord;\n"
	"uniform mat4 ViewMatrix;\n"
	"uniform mat4 ProjMatrix;\n"
	"\n"
	"out vec2 tex_coord;\n"
	"\n"
	"void main() {\n"
	"	vec4 pos = vec4( in_vert_coord, 1.0);\n"
	"	tex_coord = in_tex_coord;\n"
	"   gl_Position = ProjMatrix * ViewMatrix * pos;\n"
	"}\n"
	"";


//
// Fragment Shader: BigEndianTo10BitUin 
//
// Fragment shader that converts big endian 444 sampled texel
// to a little endian 444 sampled texel.  Input is is a simple
// component 32-bit uint texture.  Output is a four-component
// uint texture.  This fragment shader performs the same 
// operation as the Swizzle444RGBToRGBCS compute shader above.
//
const char *BigEndianTo10BitUint = 
	"#version 400\n"
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
    "uniform usampler2D tex;\n"
	"\n"
	"in  vec2 tex_coord;\n"
	"out uvec4 color2;\n"
	"void main() {\n"
	"    uint color1 = texelFetch2D(tex, ivec2(tex_coord), 0);\n"
	"    color2.b = ((color1 & 0xFF)<<2) + ((color1 & 0xC000)>>14);\n"
	"    color2.g = ((color1 & 0x3F00)>>4) + ((color1 & 0xF00000)>>20);\n"
	"	 color2.r = ((color1 & 0xFC000000)>>26)+ ((color1 & 0xF0000)>>10);\n" 
	"    color2.a = 0;\n"
	"}\n"
	"";

//
// Fragment Shader: BigEndianToFloat 
//
// Fragment shader that converts big endian 444 sampled texel
// to a little endian 444 sampled texel.  Input is is a simple
// component 32-bit uint texture.  Output is a four-component
// float texture.  
//
const char *BigEndianToFloat =    
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
    "uniform usampler2D tex;\n"
    "\n"
	"uint colorLookup(vec2 coord) {\n"
	"//Get Texel\n"
	"return(texture2D(tex, coord));\n"	
	"}\n"
	"\n"
	"void main(out vec4 color2) {\n"
	"    vec2 coord = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t);\n"
	"\n"
	"    uint color1 = colorLookup(coord);\n"
	"    float scale = 1.0f / 1023.0f;\n"
	"    float scalea = 1.0f / 3.0f;\n"		
	"    color2.r = float(((color1 & 0xFF)<<2) + ((color1 & 0xC000)>>14)) * scale;\n"
	"    color2.g = float(((color1 & 0x3F00)>>4) + ((color1 & 0xF00000)>>20)) * scale;\n"
	"	 color2.b = float(((color1 & 0xFC000000)>>26)+ ((color1 & 0xF0000)>>10)) * scale;\n" 
	"    color2.a = 0.0f;\n"
	"}\n"
	"";

//
// Fragment Shader: UintToUint 
//
// Simple fragment shader that performs a texture lookup into
// a 4-component uint texture and returns the resulting texel
// as a 4-component uint.  This shader is used in this sample
// to blit from the OGL scratch surface into the output 
// FBO maintaining the color values as 10-bit uints. 
//
const char *UintToUint =
	"#version 400\n"
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
	"uniform usampler2D tex;\n"
	"\n"
	"in  vec2 tex_coord;\n"
	"out uvec4 color;\n"
	"void main() {\n"
	"	color = texelFetch2D(tex, ivec2(tex_coord), 0);\n"
	"	color.a = 0;\n"
	"}\n"
	"";

//
// Fragment Shader: UintToUint 
//
// Simple fragment shader that performs a texture lookup into
// a 4-component uint texture and returns the resulting texel
// as a 4-component float.  This shader is used in this sample
// to blit from the output FBO into the on screen Window. 
//
const char *UintToFloat =
	"#version 400\n"
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
	"uniform usampler2D tex;\n"
	"\n"
	"in  vec2 tex_coord;\n"
	"out vec4 color2;\n"
	"void main() {\n"
	"	uvec4 color1 = texelFetch2D(tex, ivec2(tex_coord), 0);\n"
	"	float scaleRGB = 1.0f / 1023.0f;\n"
	"	float scaleA = 1.0f / 3.0f;\n"
	"   color2.r = float(color1.b) * scaleRGB;\n"
	"	color2.g = float(color1.g) * scaleRGB;\n"
	"	color2.b = float(color1.r) * scaleRGB;\n"
	"	color2.a = 0.0f;\n"
	"}\n"
	"";

//
// Fragment Shader: FloatToFloat 
//
// Simple fragment shader that performs a texture lookup into
// a 4-component float texture and returns the resulting texel
// as a 4-component float.  This shader is used in this sample
// when the image color values are not 10-bit uint but 8-bit float
//
const char *FloatToFloat =
	"// Needed to use integer textures:\n"
	"#extension GL_EXT_gpu_shader4 : enable\n"
	"uniform usampler2D tex;\n"
	"\n"
	"void main(out vec4 color) {\n"
	"	vec2 coord = vec2(gl_TexCoord[0].s, gl_TexCoord[0].t);\n"
	"	color = texture2D(tex, coord);\n"
	"}\n"
	"";


COglShaderViewer::COglShaderViewer(QWidget *parent, QGLWidget*		sharedWidget, IOglTransfer *gpuTransfer, AJA_PixelFormat pixelFormat)
:QGLWidget(parent, sharedWidget), CVideoProcessor<COglObject>(gpuTransfer), mPixelFormat(pixelFormat)
{
    setAutoBufferSwap(false);
	w=0;
	h=0;
    resize(320, 240);
	scratchOglObject = NULL;
	first = true;
	vertShader = 0;
	inputShader = 0;
	inputProgram = 0;
	outputShader = 0;
	outputProgram = 0;
	computeShader = 0;
	computeProgram = 0;
}



COglShaderViewer::~COglShaderViewer()
{


}

    
void COglShaderViewer::resizeViewport(const QSize &size)
{
    w = size.width();
    h = size.height();
    doResize = true;
}    

//
// Draw colorbar pattern
//
// Draw Pattern
GLvoid drawPattern() 
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ); 

	int lBarWidth = 1920 / 8;
	int lBarHeight = 1080;
	int startx = 0;
	int stopx = lBarWidth;

	// Black
	glColor4f(0.0, 0.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Blue
	glColor4f(0.0, 0.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Green
	glColor4f(0.0, 1.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Yellow
	glColor4f(0.0, 1.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Red
	glColor4f(1.0, 0.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;
 
	// Magenta
	glColor4f(1.0, 0.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// Cyan
	glColor4f(1.0, 1.0, 0.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	startx = stopx;
	stopx += lBarWidth;

	// White
	glColor4f(1.0, 1.0, 1.0, 1.0);
	glRecti(startx, 0, stopx, lBarHeight);

	glFinish();
}


bool COglShaderViewer::Process()
{
	odprintf("COglShaderViewer::Process()");

    srand(QTime::currentTime().msec());
    if (doResize) {
        glViewport(0, 0, w, h);
        doResize = false;
    }
	COglObject *inGpuObject = NULL;	
	if(mGpuQueue[INQ])
	{		
		inGpuObject = mGpuQueue[INQ]->StartConsumeNextBuffer();
		odprintf("inGpuObject Start");
	}

	COglObject *outGpuObject = NULL;	
	if(mGpuQueue[OUTQ])
	{
		outGpuObject = mGpuQueue[OUTQ]->StartProduceNextBuffer();
		odprintf("outGpuObject Start");

	}		
	if(inGpuObject && outGpuObject) 
	{					
		assert(glGetError() == GL_NO_ERROR);
		(static_cast<IOglTransfer*>(mGpuTransfer))->AcquireObject(inGpuObject);
		assert(glGetError() == GL_NO_ERROR);
		(static_cast<IOglTransfer*>(mGpuTransfer))->AcquireObject(outGpuObject);
		assert(glGetError() == GL_NO_ERROR);

		// First time through, create a scratch OGL object for GPU processing
		if ( first ) {
			first = false;
			scratchOglObject = new COglObject;

			GpuObjectDesc desc;
			desc._internalformat = GL_RGB10_A2UI;
			desc._format = GL_RGBA_INTEGER;
			desc._type = GL_UNSIGNED_INT_10_10_10_2;
			desc._width = inGpuObject->GetWidth();//3840; //
			desc._height = inGpuObject->GetHeight();
			desc._numChannels = 3;
			desc._useTexture = true;
			desc._useRenderToTexture = true;

			scratchOglObject->Init(desc);
		}

		GLfloat view_matrix[16];
		GLfloat proj_matrix[16];

		//
		// Unpack 10-bit DPX data converting single 32-bit uint per pixel to four components (RGBA) per pixel
		// performing Big Endian to Little Endian Conversion, 422-to-444 expansion and Color Space Conversion if required
		//

		// Enable the compute shader
		glUseProgram(computeProgram);
		assert(glGetError() == GL_NO_ERROR);

		GLuint inTex;
		inTex = glGetUniformLocation(computeProgram, "inTex");
		glUniform1i(inTex, 0);
		assert(glGetError() == GL_NO_ERROR);

		GLuint outTex;
		outTex = glGetUniformLocation(computeProgram, "outTex");
		glUniform1i(outTex, 1);
		assert(glGetError() == GL_NO_ERROR);

		// Bind image textures
		glBindImageTexture(0, inGpuObject->GetTextureHandle(), 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32UI);
		assert(glGetError() == GL_NO_ERROR);

		glBindImageTexture(1, scratchOglObject->GetTextureHandle(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGB10_A2UI);
		assert(glGetError() == GL_NO_ERROR);

		glDispatchCompute(scratchOglObject->GetWidth()/6, scratchOglObject->GetHeight(), 1);
		assert(glGetError() == GL_NO_ERROR);

		glUseProgram(0);

		//
		// Blit and scaling to output surface
		//

		// Blit the scratch texture to the output texture using a GLSL shader to scale the image.
		outGpuObject->Begin();	

		glClearColor(0.0,0.0,1.0,1.0);
		glClear(GL_COLOR_BUFFER_BIT);			
		
		glViewport(0, 0, outGpuObject->GetWidth(), outGpuObject->GetHeight());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1.0, 1.0, -1.0, 1.0);	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glEnable(GL_TEXTURE_2D);
		assert(glGetError() == GL_NO_ERROR);

		// Enable the GLSL shader
		glUseProgram(inputProgram);
		assert(glGetError() == GL_NO_ERROR);

		// Set uniform variables
		glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);
		glUniformMatrix4fv(glGetUniformLocation(inputProgram, "ViewMatrix"), 1, GL_FALSE, view_matrix);

		glGetFloatv(GL_PROJECTION_MATRIX, proj_matrix);
		glUniformMatrix4fv(glGetUniformLocation(inputProgram, "ProjMatrix"), 1, GL_FALSE, proj_matrix);

		GLuint tex;
		tex = glGetUniformLocation(inputProgram, "tex");
		glUniform1i(tex, 0);
		assert(glGetError() == GL_NO_ERROR);

		// Get vertex attribute locations
		GLint tex_coord = glGetAttribLocation(inputProgram, "in_tex_coord");
		assert(glGetError() == GL_NO_ERROR);

		// Bind texture object
		glBindTexture(GL_TEXTURE_2D, scratchOglObject->GetTextureHandle());  

		// Draw the background as the source
		glBegin(GL_QUADS);

		// Use non-normalized texture coordinates.
		glVertexAttrib2f(tex_coord, scratchOglObject->GetWidth(), scratchOglObject->GetHeight()); glVertex2f(1.0f, 1.0f);
		glVertexAttrib2f(tex_coord, scratchOglObject->GetWidth(), 0);  glVertex2f(1.0f, -1.0f); 
		glVertexAttrib2f(tex_coord, 0, 0);  glVertex2f(-1.0f, -1.0f); 
		glVertexAttrib2f(tex_coord, 0, scratchOglObject->GetHeight()); glVertex2f(-1.0f, 1.0f);

		glEnd();
		assert(glGetError() == GL_NO_ERROR);

		glBindTexture(GL_TEXTURE_2D, 0); 

		// Disable the GLSL shader
		glUseProgram(0);

		outGpuObject->End();

		//
		// Draw to Window
		//

		//set OpenGL parameters back to the window settings to display the output texture
		glViewport(0, 0, w, h);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluOrtho2D(-1.0, 1.0, -1.0, 1.0);	
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		glClearColor(0.0,1.0,0.0,1.0);
		glClear(GL_COLOR_BUFFER_BIT);

		// Enable the GLSL shader
#ifdef DO_10BIT_OUTPUT
		glUseProgram(outputProgram);

		// Set uniform variables
		glGetFloatv(GL_MODELVIEW_MATRIX, view_matrix);
		glUniformMatrix4fv(glGetUniformLocation(outputProgram, "ViewMatrix"), 1, GL_FALSE, view_matrix);

		glGetFloatv(GL_PROJECTION_MATRIX, proj_matrix);
		glUniformMatrix4fv(glGetUniformLocation(outputProgram, "ProjMatrix"), 1, GL_FALSE, proj_matrix);

		GLuint tex2;
		tex2 = glGetUniformLocation(outputProgram, "tex");
		glUniform1i(tex2, 0);

		// Get vertex attribute locations
		GLuint tex_coord2 = glGetAttribLocation(outputProgram, "in_tex_coord");

		assert(glGetError() == GL_NO_ERROR);
#endif

		// Bind texture object 
		glBindTexture(GL_TEXTURE_2D, outGpuObject->GetTextureHandle());  

		assert(glGetError() == GL_NO_ERROR);

		// Draw the video output texture
		glBegin(GL_QUADS);	

		glVertexAttrib2f(tex_coord2, outGpuObject->GetWidth(), outGpuObject->GetHeight()); glVertex2f(1.0f, -1.0f);
		glVertexAttrib2f(tex_coord2, outGpuObject->GetWidth(), 1);  glVertex2f(1.0f, 1.0f); 
		glVertexAttrib2f(tex_coord2, 1, 1);  glVertex2f(-1.0f, 1.0f); 
		glVertexAttrib2f(tex_coord2, 1, outGpuObject->GetHeight()); glVertex2f(-1.0f, -1.0f);

		glEnd();

		// Disable the GLSL shader
		glUseProgram(0);

		(static_cast<IOglTransfer*>(mGpuTransfer))->ReleaseObject(inGpuObject);
		(static_cast<IOglTransfer*>(mGpuTransfer))->ReleaseObject(outGpuObject);
	}
	else if(inGpuObject)
	{
		(static_cast<IOglTransfer*>(mGpuTransfer))->AcquireObject(inGpuObject);
		QRectF target;		
		target.setRect(-1.0, -1.0, 2.0, 2.0);		
		drawTexture(target,inGpuObject->GetTextureHandle(),GL_TEXTURE_2D);		
		(static_cast<IOglTransfer*>(mGpuTransfer))->ReleaseObject(inGpuObject);


	}      
	if(mGpuQueue[INQ])
	{		
		mGpuQueue[INQ]->EndConsumeNextBuffer();
		odprintf("outGpuObject End");


	}	
	if(mGpuQueue[OUTQ])
	{
		mGpuQueue[OUTQ]->EndProduceNextBuffer();	
		odprintf("outGpuObject End");

		
	}	
	swapBuffers();

	return true;
}

bool COglShaderViewer::Init()
{
	GLint val;
	GLint res;

	// Init glew - NEED TO MOVE THIS SOMEPLACE ELSE
	glewInit();

	// Create GLSL vertex shader.  This single simple vertex shader is
	// used for both blitting the input video into the output framebuffer
	// object and for rendering the output framebuffer into the 
	// onscreen window.
	vertShader = glCreateShader(GL_VERTEX_SHADER);

	// Initialize GLSL vertex shader
	glShaderSourceARB(vertShader, 1, (const GLchar **)&VertexShader, NULL);

	// Compile vertex shader
	glCompileShaderARB(vertShader);

	// Check for errors
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &val);
	if (!val) {
		char infoLog[10000];
		glGetShaderInfoLog(vertShader, 10000, NULL, infoLog);
		odprintf("Failed to load GLSL vertex shader, INFO:\n\n%s\n", infoLog);
		return false;
	}

	// Create GLSL fragment shader for input image texture
	inputShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Initialize GLSL fragment shader for input image texture
#ifdef DO_10BIT_OUTPUT
	glShaderSourceARB(inputShader, 1, (const GLchar **)&UintToUint, NULL);
#else
	glShaderSourceARB(inputShader, 1, (const GLchar **)&UintToFloat, NULL);
#endif

	// Compile fragment shader
	glCompileShaderARB(inputShader);

	// Check for errors
	glGetShaderiv(inputShader, GL_COMPILE_STATUS, &val);
	if (!val) {
		char infoLog[10000];
		glGetShaderInfoLog(inputShader, 10000, NULL, infoLog);
		odprintf("Failed to load GLSL fragment shader for input image texture, INFO:\n\n%s\n", infoLog);
		return false;
	}

	// Create shader program for input image texture
	inputProgram = glCreateProgram();

	// Attach vertex shader to program
	glAttachShader(inputProgram, vertShader);

	// Attach fragment shader to program
	glAttachShader(inputProgram, inputShader);

	// Link shader program
	glLinkProgram(inputProgram);

	// Check for errors
	glGetProgramiv(inputProgram, GL_LINK_STATUS, &res);
	if (!res) {
		odprintf("Failed to link GLSL input image texture program\n");
		GLint infoLength;
		glGetProgramiv(inputProgram, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength) {
			char *buf;
			buf = (char *) malloc(infoLength);
			if (buf) {
				glGetProgramInfoLog(inputProgram, infoLength, NULL, buf);
				odprintf("Program Log: \n");
				odprintf("%s", buf);
				free(buf);
			}
		}
		return false;
	}

	// Create GLSL shader for output image texture from FBO
	outputShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Initialize GLSL shader for output image texture from FBO
#ifdef DO_10BIT_OUTPUT
	glShaderSourceARB(outputShader, 1, (const GLchar **)&UintToFloat, NULL);
#else
	glShaderSourceARB(outputShader, 1, (const GLchar **)&FloatToFloat, NULL);
#endif

	// Compile fragment shader
	glCompileShaderARB(outputShader);

	// Check for errors
	glGetShaderiv(outputShader, GL_COMPILE_STATUS, &val);
	if (!val) {
		char infoLog[10000];
		glGetShaderInfoLog(outputShader, 10000, NULL, infoLog);
		odprintf("Failed to load GLSL fragment shader for output image texture from FBO, INFO:\n\n%s\n", infoLog);
		return false;
	}

	// Create shader program for output image texture from FBO
	outputProgram = glCreateProgram();

	// Attach vertex shader to program
	glAttachShader(outputProgram, vertShader);

	// Attach fragment shader to program
	glAttachShader(outputProgram, outputShader);

	// Link shader program
	glLinkProgram(outputProgram);

	// Check for link errors
	glGetProgramiv(outputProgram, GL_LINK_STATUS, &res);
	if (!res) {
		odprintf("Failed to link GLSL output image texture program\n");
		GLint infoLength;
		glGetProgramiv(outputProgram, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength) {
			char *buf;
			buf = (char *) malloc(infoLength);
			if (buf) {
				glGetProgramInfoLog(outputProgram, infoLength, NULL, buf);
				odprintf("Program Log: \n");
				odprintf("%s", buf);
				free(buf);
			}
		}
		return false;
	}

	// Create GLSL compute shader for input image processing.  This includes big endian to 
	// little endian conversion and YCrCb422 to RGB444 colorspace conversion if required as
	// well as unpacking the image data from a single 32-bit uint per component to four
	// uints, one for each color component.
	computeShader = glCreateShader(GL_COMPUTE_SHADER);

	// Initialize GLSL compute shader for input image processing from a 
	// single-component 32-bit uint per pixel to four unint components per
	// pixel one for each color component.

//#define INPUT_IS_LITTLE_ENDIAN_RGB_DPX
//#define INPUT_IS_BIG_ENDIAN_RGB_DPX
//#define INPUT_IS_BIG_ENDIAN_YCRCB_DPX

	if (mPixelFormat == AJA_PixelFormat_RGB_DPX_LE)
	{
		glShaderSourceARB(computeShader, 1, (const GLchar **)&UintToFourComponent, NULL);	
	}
	else 
	if (mPixelFormat == AJA_PixelFormat_YCbCr_DPX)
	{
		glShaderSourceARB(computeShader, 1, (const GLchar **)&Swizzle422To444YCrCbToRGBCS, NULL);
	}
	else
	{
		//assume AJA_PixelFormat_RGB_DPX	
		glShaderSourceARB(computeShader, 1, (const GLchar **)&Swizzle444RGBToRGBCS, NULL);
	}


	// Compile fragment shader
	glCompileShaderARB(computeShader);

	// Check for errors
	glGetShaderiv(computeShader, GL_COMPILE_STATUS, &val);
	if (!val) {
		char infoLog[10000];
		glGetShaderInfoLog(computeShader, 10000, NULL, infoLog);
		odprintf("Failed to compile GLSL compute shader for 422 to 444 CSC, INFO:\n\n%s\n", infoLog);
		return false;
	}

	// Create shader program for output image texture from FBO
	computeProgram = glCreateProgram();

	// Attach compute shader to program
	glAttachShader(computeProgram, computeShader);

	// Link shader program
	glLinkProgram(computeProgram);

	// Check for link errors
	glGetProgramiv(computeProgram, GL_LINK_STATUS, &res);
	if (!res) {
		odprintf("Failed to link GLSL 422 to 444 compute program.\n");
		GLint infoLength;
		glGetProgramiv(computeProgram, GL_INFO_LOG_LENGTH, &infoLength);
		if (infoLength) {
			char *buf;
			buf = (char *) malloc(infoLength);
			if (buf) {
				glGetProgramInfoLog(computeProgram, infoLength, NULL, buf);
				odprintf("Program Log: \n");
				odprintf("%s", buf);
				free(buf);
			}
		}
		return false;
	}
	return true;
}

bool COglShaderViewer::Deinit()
{	
	return true;
}


bool COglShaderViewer::SetupThread()
{	
	//this is called inside the thread
	//make OGL context current
	makeCurrent();
	assert(glGetError() == GL_NO_ERROR);
	qglClearColor(Qt::blue);     
	glMatrixMode(GL_PROJECTION);
 	glLoadIdentity();
	gluOrtho2D(-1.0, 1.0, -1.0, 1.0);	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	assert(glGetError() == GL_NO_ERROR);
	return true;
}

bool COglShaderViewer::CleanupThread()
{
	//this is called inside the thread
	return true;
}

void COglShaderViewer::resizeEvent(QResizeEvent *evt)
{
    resizeViewport(evt->size());
}

void COglShaderViewer::paintEvent(QPaintEvent *)
{
    // Handled by the GLThread.
}

void COglShaderViewer::closeEvent(QCloseEvent *evt)
{    
    QGLWidget::closeEvent(evt);
}


