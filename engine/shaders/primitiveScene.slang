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
    float4x4 normalMatrix;
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
    float3 worldPosition : TEXCOORD1;
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

    const float3 worldPosition = mul(primitivePushConstants.worldTransform, vertexData.position).xyz;
    const float3 worldNormal = normalize(mul((float3x3)primitivePushConstants.normalMatrix, vertexData.normal.xyz));

    PrimitiveVertexOutput output;
    output.position = mul(cameraBuffer.viewProjection, float4(worldPosition, 1.0));
    output.worldNormal = worldNormal;
    output.worldPosition = worldPosition;
    output.color = vertexData.color;
    return output;
}

[shader("fragment")]
float4 fragmentMain(PrimitiveVertexOutput input) : SV_Target0
{
    const float3 lightPos = float3(100.0, 100.0, 100.0);
    const float3 lightColor = float3(1.0, 1.0, 1.0);
    const float ambientStrength = 0.1;
    const float specularStrength = 0.5;

    const float3 ambient = ambientStrength * lightColor;

    const float3 norm = normalize(input.worldNormal);
    const float3 lightDir = normalize(lightPos - input.worldPosition);
    const float diff = max(dot(norm, lightDir), 0.0);
    const float3 diffuse = diff * lightColor;

    const float3 viewDir = normalize(cameraBuffer.worldPosition.xyz - input.worldPosition);
    const float3 reflectDir = reflect(-lightDir, norm);
    const float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    const float3 specular = specularStrength * spec * lightColor;

    return float4((ambient + diffuse + specular) * input.color.xyz, input.color.a);
}
