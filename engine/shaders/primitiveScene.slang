struct CameraData
{
    float4 worldPosition;
    float4x4 view;
    float4x4 projection;
    float4x4 viewProjection;
    float4 aspectRatio;
};

struct PrimitivePushConstants
{
    float4x4 worldTransform;
    uint vertexOffsetBytes;
    uint indexOffsetBytes;
    uint padding0;
    uint padding1;
};

struct PrimitiveVertexData
{
    float4 position;
    float4 normal;
    float4 color;
    float4 reserved;
};

struct PrimitiveVertexOutput
{
    float4 position : SV_Position;
    float3 worldNormal : TEXCOORD0;
    float4 color : COLOR0;
};

[[vk::binding(0, 0)]]
ConstantBuffer<CameraData> cameraBuffer;

[[vk::binding(1, 0)]]
ByteAddressBuffer geometryBuffer;

[[vk::push_constant]]
ConstantBuffer<PrimitivePushConstants> primitivePushConstants;

uint loadIndex(uint indexOffsetBytes, uint vertexId)
{
    return geometryBuffer.Load(indexOffsetBytes + vertexId * 4);
}

float4 loadFloat4(uint byteOffset)
{
    return asfloat(geometryBuffer.Load4(byteOffset));
}

PrimitiveVertexData loadPrimitiveVertex(uint baseVertexOffsetBytes, uint vertexIndex)
{
    PrimitiveVertexData vertexData;
    const uint vertexByteOffset = baseVertexOffsetBytes + vertexIndex * 64;
    vertexData.position = loadFloat4(vertexByteOffset + 0);
    vertexData.normal = loadFloat4(vertexByteOffset + 16);
    vertexData.color = loadFloat4(vertexByteOffset + 32);
    vertexData.reserved = loadFloat4(vertexByteOffset + 48);
    return vertexData;
}

[shader("vertex")]
PrimitiveVertexOutput vertexMain(uint vertexId : SV_VertexID)
{
    const uint indexValue = loadIndex(primitivePushConstants.indexOffsetBytes, vertexId);
    const PrimitiveVertexData vertexData =
        loadPrimitiveVertex(primitivePushConstants.vertexOffsetBytes, indexValue);

    const float4 worldPosition = mul(primitivePushConstants.worldTransform, vertexData.position);
    const float3 worldNormal =
        normalize(mul((float3x3)primitivePushConstants.worldTransform, vertexData.normal.xyz));

    PrimitiveVertexOutput output;
    output.position = mul(cameraBuffer.viewProjection, worldPosition);
    output.worldNormal = worldNormal;
    output.color = vertexData.color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(PrimitiveVertexOutput input) : SV_Target0
{
    const float3 lightDirection = normalize(float3(0.35, 0.85, 0.4));
    const float diffuse = saturate(dot(normalize(input.worldNormal), lightDirection));
    const float lighting = 0.2 + diffuse * 0.8;
    return float4(input.color.rgb * lighting, input.color.a);
}
