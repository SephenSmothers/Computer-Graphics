// an ultra simple hlsl vertex shader
// TODO: Part 1C - done
// TODO: Part 2B - done
// TODO: Part 2F - done
// TODO: Part 2G - done
// TODO: Part 3B - done

cbuffer SHADER_VARS
{
    float4x4 VS_WorldMatrix;
	float4x4 VS_ViewMatrix;
    float4x4 VS_ProjectionMatrix;
};

float4 main(float4 inputVertex : POSITION) : SV_POSITION
{
    float4x4 VS_WorldMatrixCopy = VS_WorldMatrix;
    float4x4 VS_ViewMatrixCopy = VS_ViewMatrix;
    float4x4 VS_ProjectionMatrixCopy = VS_ProjectionMatrix;
    
    VS_WorldMatrixCopy = transpose(VS_WorldMatrixCopy);
    VS_ViewMatrixCopy = transpose(VS_ViewMatrixCopy);
    VS_ProjectionMatrixCopy = transpose(VS_ProjectionMatrixCopy);
	
    inputVertex = mul(inputVertex, VS_WorldMatrixCopy);
    inputVertex = mul(inputVertex, VS_ViewMatrixCopy);
    inputVertex = mul(inputVertex, VS_ProjectionMatrixCopy);
	
	return inputVertex; 
}


