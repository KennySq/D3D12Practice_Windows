struct Vertex_Input
{
    float3 Position : POSITION0;
    float2 Texcoord : TEXCOORD0;
};

struct Pixel_Input
{
    float4 Position : SV_Position;
    float2 Texcoord : TEXCOORD0;
};

Pixel_Input vert(Vertex_Input input)
{
    Pixel_Input output = (Pixel_Input) 0;
    
    output.Position = float4(input.Position, 1.0f);
    output.Texcoord = input.Texcoord;
    
	return output;
}

float4 frag(Pixel_Input input) : SV_Target0
{
    float4 color = input.Texcoord.xyxy;
    
    return color;
}