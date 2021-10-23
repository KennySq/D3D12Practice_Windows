cbuffer Constants : register(b0)
{
    float4 gVelocity;
    float4 gOffset;
    float4 gColor;
    float4x4 gProjection;
    
}

struct VertexInput
{
    float4 Position : POSITION0;
    float4 Color : COLOR0;
};

struct PixelInput
{
    float4 Position : SV_Position;
    float4 Color : COLOR0;
};

PixelInput vert(VertexInput input)
{
    PixelInput output = (PixelInput) 0;
    
    output.Position = mul(input.Position + gOffset, gProjection);
    output.Color = input.Color;
    
	return output;
}

float4 frag(PixelInput input) : SV_Target0
{
    float4 color = input.Color;
    
    return color;
}