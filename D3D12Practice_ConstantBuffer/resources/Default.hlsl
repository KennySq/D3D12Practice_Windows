cbuffer Constants : register(b0)
{
    row_major float4x4 gWorld;
}

struct VertexInput
{
    float3 Position : POSITION0;
    float4 Color : COLOR0;
};

struct PixelInput
{
    float3 Position : SV_Position;
    float4 Color : TEXCOORD0;
};

PixelInput vert(VertexInput input)
{
    PixelInput output = (PixelInput) 0;
    
    output.Position = mul(float4(input.Position, 1.0f), gWorld);
    output.Color = input.Color;
    
	return output;
}

float4 frag(PixelInput input)
{
    float4 color;
    
    color = input.Color;
    
    return color;
}