struct CameraData
{
    float4 worldPosition;
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
    float4 aspectRatio;
};

struct GridPushConstants
{
    float gridSize;
    float gridCellSize;
    float gridMinPixelsBetweenCells;
    float padding0;
    float4 gridColorThin;
    float4 gridColorThick;
};

struct GridVertexOutput
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD0;
    float2 cameraPositionXZ : TEXCOORD1;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> cameraBuffer;

[[vk::push_constant]]
ConstantBuffer<GridPushConstants> gridPushConstants;

float satf(float value)
{
    return clamp(value, 0.0, 1.0);
}

float2 satv(float2 value)
{
    return clamp(value, float2(0.0, 0.0), float2(1.0, 1.0));
}

float max2(float2 value)
{
    return max(value.x, value.y);
}

float2 glslMod(float2 value, float divisor)
{
    return value - divisor * floor(value / divisor);
}

float log10f(float value)
{
    return log(value) / log(10.0);
}

static const float3 kGridPositions[4] = {
    float3(-1.0, 0.0, 1.0),
    float3( 1.0, 0.0, 1.0),
    float3( 1.0, 0.0, -1.0),
    float3(-1.0, 0.0, -1.0),
};

static const uint kGridIndices[6] = {
    0u, 1u, 2u, 2u, 3u, 0u,
};

[shader("vertex")]
GridVertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    float3 position = kGridPositions[kGridIndices[vertexId]] * gridPushConstants.gridSize;
    position.x += cameraBuffer.worldPosition.x;
    position.z += cameraBuffer.worldPosition.z;

    GridVertexOutput output;
    output.position = mul(cameraBuffer.viewProjection, float4(position, 1.0));
    output.uv = position.xz;
    output.cameraPositionXZ = cameraBuffer.worldPosition.xz;
    return output;
}

float4 gridColor(float2 uv, float2 cameraPositionXZ)
{
    float2 dudv = float2(
        length(float2(ddx(uv.x), ddy(uv.x))),
        length(float2(ddx(uv.y), ddy(uv.y))));

    const float lodLevel =
        max(0.0, log10f((length(dudv) * gridPushConstants.gridMinPixelsBetweenCells) / gridPushConstants.gridCellSize) + 1.0);
    const float lodFade = fract(lodLevel);
    const float lod0 = gridPushConstants.gridCellSize * pow(10.0, floor(lodLevel));
    const float lod1 = lod0 * 10.0;
    const float lod2 = lod1 * 10.0;

    dudv *= 4.0;
    uv += dudv / 2.0;

    const float lod0Alpha =
        max2(float2(1.0, 1.0) - abs(satv(glslMod(uv, lod0) / dudv) * 2.0 - float2(1.0, 1.0)));
    const float lod1Alpha =
        max2(float2(1.0, 1.0) - abs(satv(glslMod(uv, lod1) / dudv) * 2.0 - float2(1.0, 1.0)));
    const float lod2Alpha =
        max2(float2(1.0, 1.0) - abs(satv(glslMod(uv, lod2) / dudv) * 2.0 - float2(1.0, 1.0)));

    uv -= cameraPositionXZ;

    float4 color = float4(0.0, 0.0, 0.0, 0.0);
    if (lod2Alpha > 0.0)
    {
        color = gridPushConstants.gridColorThick;
        color.a *= lod2Alpha;
    }
    else if (lod1Alpha > 0.0)
    {
        color = lerp(gridPushConstants.gridColorThick, gridPushConstants.gridColorThin, lodFade);
        color.a *= lod1Alpha;
    }
    else
    {
        color = gridPushConstants.gridColorThin;
        color.a *= lod0Alpha * (1.0 - lodFade);
    }

    const float opacityFalloff = 1.0 - satf(length(uv) / gridPushConstants.gridSize);
    color.a *= opacityFalloff;
    return color;
}

[shader("fragment")]
float4 fragmentMain(GridVertexOutput input) : SV_Target0
{
    return gridColor(input.uv, input.cameraPositionXZ);
}
