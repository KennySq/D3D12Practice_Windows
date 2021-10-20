SamplerState DefaultSampler : register(s0);

Texture2D SampleTexture : register(t0);

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
    float4 color = float4(input.Texcoord.xy, 0.0f, 1.0f);
    
    color = SampleTexture.Sample(DefaultSampler, input.Texcoord);
    
  //  return float4(1, 1, 1, 1);
    return color;
}