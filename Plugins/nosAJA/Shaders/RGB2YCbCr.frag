// Copyright MediaZ Teknoloji A.S. All Rights Reserved.

#version 450
#include "Common.glsl"

layout(binding  = 0) uniform sampler2D Source;
layout(location = 0) out uvec4 rt;
layout(location = 0) in vec2 uv;

void main()
{
    const int  Interlaced = int((InterlacedFlags & 3) != 0);
    const uint YOffset    = (InterlacedFlags >> 1) & 1;
    const ivec2 UV        = ivec2(gl_FragCoord.xy - 0.5) * ivec2(2, 1 + Interlaced);
    const ivec2 uv0       = UV + ivec2(0, YOffset);
    const ivec2 uv1       = UV + ivec2(1, YOffset);
    uvec3 Y0              = SDR_Out_8(texelFetch(Source, uv0, 0).xyz);
    uvec3 Y1              = SDR_Out_8(texelFetch(Source, uv1, 0).xyz);
    uvec2 C0              = (Y0.yz + Y1.yz) >> 1;
    rt                    = uvec4(C0.x, Y0.x, C0.y, Y1.x);
}
