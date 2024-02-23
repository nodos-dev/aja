// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#version 450
#include "Common.glsl"

layout(binding = 0) uniform usampler2D Source;
layout(location = 0) out vec4 rt;
layout(location = 0) in vec2 uv;

layout (binding = 1, rgba8)  uniform writeonly image2D Output;

void main()
{

    vec4 Color     = (texelFetch(Source, ivec2((gl_FragCoord.xy - 0.5) / vec2(2, 1)) + ivec2(0, 0), 0) / 255.0).bgra;
	vec4 NextColor = (texelFetch(Source, ivec2((gl_FragCoord.xy - 0.5) / vec2(2, 1)) + ivec2(1, 0), 0) / 255.0).bgra;
    
	float Y1 = Color.g;
	vec2 C1 = Color.br;

	float Y2 = Color.a;
	vec2 C2  = C1;	// We do not have this data. Aim is to calculate this.

	float Y3 = NextColor.g;
	vec2 C3 = NextColor.br;
    
    vec3 Lin = IDXff(vec3(Y1, Y2, Y3));

	float Amount = 1.0;
	vec2 Diff = abs(Lin.xz - Lin.yy);
	float TotalDiff = Diff.x + Diff.y;
    
	if (TotalDiff == 0.0)
		Amount = 0.5;
	else
		Amount = Diff.x / TotalDiff;
	
	C2 = mix(C1, C3, 0.5);
	if (Amount < 0.5)
		C2 = mix(C1, C2, Amount * 2);
	else
		C2 = mix(C2, C3, (Amount - 0.5) * 2);
    
	// float X = fract(floor(uv.x * InInputTextureSize.x * 2.0) / 2.0);
	float Y = (int(gl_FragCoord.x) % 2) != 0 ? Y2 : Y1;
	vec2 C  = (int(gl_FragCoord.x) % 2) != 0 ? C2 : C1;

	rt = vec4(IDXff((ubo.Colorspace * vec4(Y, C, 1)).xyz), 1);
}
