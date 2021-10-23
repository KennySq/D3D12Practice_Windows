#define THREADS_PER_BLOCK 128

struct Constants
{
    float4 Velocity;
    float4 Offset;
    float4 Color;
    float4x4 Projection;
    
    float4 Unused[9];
};

struct IndirectCommands
{
    uint2 Address;
    uint4 Arguments;
};

cbuffer RootConstants : register(b0)
{
    float OffsetX;
    float OffsetZ;
    float OffsetCull;
    
    float CommandCount;
}

StructuredBuffer<Constants> ConstantBuffer : register(t0);
StructuredBuffer<IndirectCommands> InputCommands : register(t1);
AppendStructuredBuffer<IndirectCommands> OutputCommands : register(u0);

[numthreads(THREADS_PER_BLOCK, 1, 1)]
void comp( uint3 DTid : SV_DispatchThreadID, uint Gi : SV_GroupIndex, uint3 Gid : SV_GroupID)
{
    uint index = (Gid.x * THREADS_PER_BLOCK) + Gi;
    
    if (index < CommandCount)
    {
        float4 left = float4(-OffsetX, 0.0f, OffsetZ, 1.0f) + ConstantBuffer[index].Offset;
        
    }

}