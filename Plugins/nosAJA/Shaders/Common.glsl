#extension GL_EXT_shader_16bit_storage : enable

struct umat4
{
    uvec4 x, y, z, w;
};

layout(binding = 2) uniform UBO
{
    mat4 Colorspace;
    mat4 ColorspaceT;
    uint Interlaced;
}
ubo;

uint InterlacedFlags = ubo.Interlaced;

layout(binding = 3) buffer SSBO
{
    uint16_t LUT[];
} ssbo;

#define N16 ((1<<16)-1)
#define N10 ((1<<10)-1)
#define N8  ((1<< 8)-1)


vec4 LinearToRec709(in vec4 c)
{
    c = clamp(c, vec4(0), vec4(1));
    return mix(pow(c, vec4(0.45)) * 1.099 - 0.099, c * 4.5, lessThan(c, vec4(0.018)));
}

vec4 Rec709ToLinear(in vec4 c)
{
    return mix(pow((c + 0.099) / 1.099, vec4(1.0/0.45)), c / 4.5, lessThan(c, vec4(0.081)));
}

vec4 HLGToLinear(in vec4 c) 
{
	c = max(c, vec4(0.0));
    const vec4 c0 = c * c / 3.0;
    const vec4 c1 = exp(c / 0.17883277 - 5.61582460179) + 0.02372241;
    return mix(c1, c0, lessThan(c, vec4(0.5)));
}

vec4 LinearToHLG(in vec4 c) 
{
    const vec4 c0 = sqrt(3 * c);
    const vec4 c1 = log(c - 0.02372241) * 0.17883277 + 1.00429346;
    return mix(c1, c0, lessThan(c, vec4(1.0/12.0)));
}

const vec4 m1 = vec4(0.1593017578125); // = 2610. / 4096. * .25;
const vec4 m2 = vec4(78.84375); // = 2523. / 4096. *  128;
const vec4 c1 = vec4(0.8359375); // = 2392. / 4096. * 32 - 2413./4096.*32 + 1;
const vec4 c2 = vec4(18.8515625); // = 2413. / 4096. * 32;
const vec4 c3 = vec4(18.6875); // = 2392. / 4096. * 32;

vec4 ST2084ToLinear(vec4 c)
{
    c = pow(c, 1. / m2);
    return pow(max(c - c1, vec4(0)) / (c2  - c3 * c), 1. / m1);
}

vec4 LinearToST2084(vec4 c)
{
    c = pow(c, m1);
    return pow((c1 + c2 * c) / (1 + c3 * c), m2);
}

uvec4 IDX4(uvec4 i)
{
    return uvec4(ssbo.LUT[i.x], ssbo.LUT[i.y], ssbo.LUT[i.z], ssbo.LUT[i.w]);
}

uvec3 IDX(uvec3 i)
{
    return uvec3(ssbo.LUT[i.x], ssbo.LUT[i.y], ssbo.LUT[i.z]);
}

uvec3 IDXuu(uvec3 c) { return IDX(min(c, N10)); }
uvec3 IDXfu( vec3 c) { return IDXuu(uvec3(round(c * N10))); }
 vec3 IDXuf(uvec3 c) { return IDXuu(c) / float(N16); }
 vec3 IDXff( vec3 c) { return IDXfu(c) / float(N16); }

/*
In:
    u8/10 -> Mul -> Idx -> f16
Out:
    f16 -> Idx -> Mul -> u8/10
*/


vec4  SDR_In (in vec3 c) 
{ 
    return  vec4(IDXff((ubo.Colorspace * vec4(c, 1)).xyz), 1); 
}

vec4  SDR_In_N (in uvec3 c, float N) 
{ 
    return  vec4(IDXff((ubo.Colorspace * vec4(c / N, 1)).xyz), 1); 
}

uvec3 SDR_Out_N(in vec3 c, float N)  
{ 
    return uvec3(round((ubo.Colorspace * vec4(IDXff(c), 1)).xyz * N)); 
}

vec4  SDR_In_10 (in uvec3 c) { return SDR_In_N (c, N10); }
vec4  SDR_In_8  (in uvec3 c) { return SDR_In_N (c, N8); }
uvec3 SDR_Out_10(in vec3 c)  { return SDR_Out_N(c, N10); }
uvec3 SDR_Out_8 (in vec3 c)  { return SDR_Out_N(c, N8); }
