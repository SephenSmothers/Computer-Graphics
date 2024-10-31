// an ultra simple hlsl pixel shader
// TODO: Part 3A - done
// TODO: Part 4B - done
// TODO: Part 4C - done
// TODO: Part 4F - done

struct OutputToRasterizer
{
    float4 posH : SV_POSITION;
    float3 posW : WORLD;
    float3 normW : NORMAL;
};

struct OBJ_Attributes
{
    float3 Kd; // diffuse reflectivity
    float d; // dissolve (transparency) 
    float3 Ks; // specular reflectivity
    float Ns; // specular exponent
    float3 Ka; // ambient reflectivity
    float sharpness; // local reflection map sharpness
    float3 Tf; // transmission filter
    float Ni; // optical density (index of refraction)
    float3 Ke; // emissive reflectivity
    unsigned int illum; // illumination model
};

cbuffer SceneData : register(b0)
{
    float4 sunDirection;
    float4 sunColor;
    float4 sunAmbient;
    float4 cameraPos;
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
};

cbuffer MeshData : register(b1)
{
    float4x4 worldMatrix;
    OBJ_Attributes material;
};

float4 main(OutputToRasterizer input : PS_INPUT) : SV_TARGET
{
    // Normalize the world-space normal
    float3 normal = normalize(input.normW);

    float3 lightDirection = normalize(sunDirection.xyz - input.posW);

    float lambertian = max(0.000001f, dot(-lightDirection, normal));
    
    float3 viewDirection = normalize(cameraPos.xyz - input.posW);

    float3 halfway = normalize(-lightDirection + viewDirection);
    
    float dotResult = dot(normal, halfway);

    float3 intensity = max(pow(clamp(dotResult, 0.000001, 1), material.sharpness), 0.000001f);

    float specular = sunColor.xyz * material.Ks * intensity;

    float3 directDiffuse = material.Kd * sunColor.xyz * lambertian;

    float3 ambient = material.Kd * sunAmbient.xyz;

    float3 totalLighting = directDiffuse + ambient;

    totalLighting += specular;

    return float4(totalLighting, 1.0f);
}



