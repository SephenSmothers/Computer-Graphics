// an ultra simple hlsl vertex shader
// TODO: Part 1F - done
// TODO: Part 1H - done
// TODO: Part 2B - done
// TODO: Part 2D - done
// TODO: Part 4A - done
// TODO: Part 4B - done

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

OutputToRasterizer main(float3 inputVertex : POSITION, float3 uvw: UVW, float3 normal : NORMAL)
{   
    OutputToRasterizer output; 
    float4 inputVertexCpy = float4(inputVertex, 1);
    
    inputVertexCpy = mul(worldMatrix, inputVertexCpy);
    inputVertexCpy = mul(viewMatrix, inputVertexCpy);
    inputVertexCpy = mul(projectionMatrix, inputVertexCpy);
    
    output.posH = inputVertexCpy;
    output.posW = mul(worldMatrix, float4(inputVertex,1));
    output.normW = mul(worldMatrix, float4(normal, 1));
    
    return output;
}