cbuffer Constants : register(b0)
{
    row_major float4x4 gWorld;
    float4x4 gUnused0;
    float4x4 gUnused1;
    float4x4 gUnused2;
}

struct VertexInput
{
    float3 Position : POSITION0;
    float4 Color : COLOR0;
};

struct PixelInput
{
    float4 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

PixelInput vert(VertexInput input)
{
    PixelInput output = (PixelInput) 0;
    
    output.Position = mul(float4(input.Position, 1.0f), gWorld);
   // output.Position = float4(input.Position, 1.0f);
    output.Color = input.Color;
    
	return output;
}

float4 frag(PixelInput input) : SV_Target0
{
    float4 color;
    
    color = input.Color;
    
    return color;
}