#include <d3dcompiler.h>	// required for compiling shaders on the fly, consider pre-compiling instead
#pragma comment(lib, "d3dcompiler.lib") 

#include <DirectXMath.h>

using namespace DirectX;

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 2B - done
// TODO: Part 4E - done

struct SceneData
{

	GW::MATH::GVECTORF sunDirection;
	GW::MATH::GVECTORF sunColor;

	GW::MATH::GVECTORF sunAmbient;
	GW::MATH::GVECTORF cameraPos;

	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
};

struct MeshData
{
	GW::MATH::GMATRIXF  worldMatrix;
	OBJ_ATTRIBUTES material;
};

// Creation, Rendering & Cleanup
class Renderer
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		indexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		ConstSceneBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		ConstMeshBuffer;
	std::chrono::high_resolution_clock::time_point lastUpdate;

	// TODO: Part 2A - done

	GW::MATH::GMATRIXF rWorldMatrix;
	GW::MATH::GMATRIXF rViewMatrix;
	GW::MATH::GMATRIXF rProjectionMatrix;
	GW::MATH::GMatrix math;
	GW::MATH::GVECTORF lightDir;
	GW::MATH::GVECTORF lightColor;
	GW::MATH::GVECTORF rSunAmbient;
	GW::MATH::GVECTORF rCameraPos;

	// TODO: Part 2B - done

	SceneData sceneData;
	MeshData meshData;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		// TODO: Part 2A - done
		// TODO: Part 2B - done
		// TODO: Part 4E - done

		math.Create();

		//world
		math.IdentityF(rWorldMatrix);

		//view
		math.IdentityF(rViewMatrix);

		math.LookAtLHF
		(
			GW::MATH::GVECTORF{ 0.0f, 0.0f, -2.0f, 0.0f },
			GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.0f, 0.0f },
			GW::MATH::GVECTORF{ 0.0f, 1.0f, 0.0f, 0.0f }, rViewMatrix
		);

		//rViewMatrix.row4.y += -.5;

		//projection
		math.IdentityF(rProjectionMatrix);
		float ratio = 0.0f;
		d3d.GetAspectRatio(ratio);

		float left = -1.0f;
		float right = 1.0f;
		float bottom = -1.0f;
		float top = 1.0f;
		float nearPlane = 0.1f;
		float farPlane = 1000.0f;

		// Calculate the orthographic matrix manually
	   // Start with an identity matrix
		rProjectionMatrix.data[0] = 2.0f / (right - left);
		rProjectionMatrix.data[6] = 2.0f / (top - bottom);
		rProjectionMatrix.data[11] = 1.0f / (farPlane - nearPlane);
		rProjectionMatrix.data[4] = -(right + left) / (right - left);
		rProjectionMatrix.data[8] = -(top + bottom) / (top - bottom);
		rProjectionMatrix.data[12] = -nearPlane / (farPlane - nearPlane);

		math.TransposeF(rProjectionMatrix,rProjectionMatrix);
		
		//GW::MATH::GMatrix::ProjectionDirectXLHF(G_DEGREE_TO_RADIAN_F(65.0f), ratio, 0.1f, 100.0f, rProjectionMatrix);

		//lighting
		lightDir = { -1.0f, -1.0f, 2.0f, 1.0f };
		lightColor = { 0.9f, 0.9f, 1.0f, 1.0f };
		rSunAmbient = { 0.25f,0.25f,0.35f,1.0f };
		rCameraPos = rViewMatrix.row4;

		//init stuct data
		sceneData.sunDirection = lightDir;
		sceneData.sunColor = lightColor;
		sceneData.sunAmbient = rSunAmbient;
		sceneData.cameraPos = rCameraPos;
		sceneData.viewMatrix = rViewMatrix;
		sceneData.projectionMatrix = rProjectionMatrix;

		meshData.worldMatrix = rWorldMatrix;
		meshData.material = FSLogo_materials[0].attrib;

		lastUpdate = std::chrono::high_resolution_clock::now();

		IntializeGraphics();
	}

private:
	//constructor helper functions
	void IntializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);
		// TODO: Part 1G - done

		D3D11_SUBRESOURCE_DATA bData = { &FSLogo_indices[0], 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeof(FSLogo_indices), D3D11_BIND_INDEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, indexBuffer.GetAddressOf());

		// TODO: Part 2C - done

		//scene
		D3D11_SUBRESOURCE_DATA sData = { &sceneData, 0, 0 };
		CD3D11_BUFFER_DESC sDesc(sizeof(SceneData), D3D11_BIND_CONSTANT_BUFFER);
		creator->CreateBuffer(&sDesc, &sData, ConstSceneBuffer.GetAddressOf());

		//mesh
		D3D11_SUBRESOURCE_DATA mData = { &meshData, 0, 0 };
		CD3D11_BUFFER_DESC mDesc(sizeof(MeshData), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&mDesc, &mData, ConstMeshBuffer.GetAddressOf());


		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}

	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1C 
		CreateVertexBuffer(creator, &FSLogo_vertices[0], sizeof(_OBJ_VERT_) * FSLogo_vertexcount);
	}

	void CreateVertexBuffer(ID3D11Device* creator, const void* data, unsigned int sizeInBytes)
	{
		D3D11_SUBRESOURCE_DATA bData = { data, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeInBytes, D3D11_BIND_VERTEX_BUFFER);
		creator->CreateBuffer(&bDesc, &bData, vertexBuffer.GetAddressOf());
	}

	void InitializePipeline(ID3D11Device* creator)
	{
		UINT compilerFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#if _DEBUG
		compilerFlags |= D3DCOMPILE_DEBUG;
#endif
		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob = CompileVertexShader(creator, compilerFlags);
		Microsoft::WRL::ComPtr<ID3DBlob> psBlob = CompilePixelShader(creator, compilerFlags);

		CreateVertexInputLayout(creator, vsBlob);
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompileVertexShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string vertexShaderSource = ReadFileIntoString("../Shaders/VertexShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, errors;

		HRESULT compilationResult =
			D3DCompile(vertexShaderSource.c_str(), vertexShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "vs_4_0", compilerFlags, 0,
				vsBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreateVertexShader(vsBlob->GetBufferPointer(),
				vsBlob->GetBufferSize(), nullptr, vertexShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Vertex Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return vsBlob;
	}

	Microsoft::WRL::ComPtr<ID3DBlob> CompilePixelShader(ID3D11Device* creator, UINT compilerFlags)
	{
		std::string pixelShaderSource = ReadFileIntoString("../Shaders/PixelShader.hlsl");

		Microsoft::WRL::ComPtr<ID3DBlob> psBlob, errors;

		HRESULT compilationResult =
			D3DCompile(pixelShaderSource.c_str(), pixelShaderSource.length(),
				nullptr, nullptr, nullptr, "main", "ps_4_0", compilerFlags, 0,
				psBlob.GetAddressOf(), errors.GetAddressOf());

		if (SUCCEEDED(compilationResult))
		{
			creator->CreatePixelShader(psBlob->GetBufferPointer(),
				psBlob->GetBufferSize(), nullptr, pixelShader.GetAddressOf());
		}
		else
		{
			PrintLabeledDebugString("Pixel Shader Errors:\n", (char*)errors->GetBufferPointer());
			abort();
			return nullptr;
		}

		return psBlob;
	}

	void CreateVertexInputLayout(ID3D11Device* creator, Microsoft::WRL::ComPtr<ID3DBlob>& vsBlob)
	{
		// TODO: Part 1E - done
		D3D11_INPUT_ELEMENT_DESC attributes[3];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		attributes[1].SemanticName = "UVW";
		attributes[1].SemanticIndex = 0;
		attributes[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[1].InputSlot = 0;
		attributes[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[1].InstanceDataStepRate = 0;

		attributes[2].SemanticName = "NORMAL";
		attributes[2].SemanticIndex = 0;
		attributes[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		attributes[2].InputSlot = 0;
		attributes[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
		attributes[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[2].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}

public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);
		// TODO: Part 1H - done
		// TODO: Part 3B - done
		// TODO: Part 3C - done
		// TODO: Part 4D - done

		GW::MATH::GMATRIXF tempWorld;
		math.IdentityF(tempWorld);
		curHandles.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		//drawing text
		curHandles.context->Map(ConstMeshBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		meshData.material = FSLogo_materials[0].attrib;
		tempWorld = meshData.worldMatrix;
		math.IdentityF(meshData.worldMatrix);
		memcpy(mappedResource.pData, &meshData, sizeof(MeshData));
		curHandles.context->Unmap(ConstMeshBuffer.Get(), 0);
		curHandles.context->DrawIndexed(FSLogo_meshes[0].indexCount, FSLogo_meshes[0].indexOffset, 0);

		//drawing logo
		meshData.worldMatrix = tempWorld;
		curHandles.context->Map(ConstMeshBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		meshData.material = FSLogo_materials[1].attrib;
		RotateWorld();
		memcpy(mappedResource.pData, &meshData, sizeof(MeshData));
		curHandles.context->Unmap(ConstMeshBuffer.Get(), 0);

		curHandles.context->DrawIndexed(FSLogo_meshes[1].indexCount, FSLogo_meshes[1].indexOffset, 0);

		// TODO: Part 1D - done
		ReleasePipelineHandles(curHandles);
	}

	void RotateWorld()
	{
		// Get current time
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

		// Calculate the time elapsed since frame
		std::chrono::duration<double> elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastUpdate);

		float rotationSpeed = (3.14159f * elapsedTime.count()) * 20;

		math.RotateYGlobalF(meshData.worldMatrix, G_DEGREE_TO_RADIAN_F(rotationSpeed), meshData.worldMatrix);

		lastUpdate = currentTime;
	}

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};

	PipelineHandles GetCurrentPipelineHandles()
	{
		PipelineHandles retval;
		d3d.GetImmediateContext((void**)&retval.context);
		d3d.GetRenderTargetView((void**)&retval.targetView);
		d3d.GetDepthStencilView((void**)&retval.depthStencil);
		return retval;
	}

	void SetUpPipeline(PipelineHandles handles)
	{
		SetRenderTargets(handles);
		SetVertexBuffers(handles);
		SetShaders(handles);

		handles.context->VSSetConstantBuffers(0, 1, ConstSceneBuffer.GetAddressOf());
		handles.context->VSSetConstantBuffers(1, 1, ConstMeshBuffer.GetAddressOf());

		handles.context->PSSetConstantBuffers(0, 1, ConstSceneBuffer.GetAddressOf());
		handles.context->PSSetConstantBuffers(1, 1, ConstMeshBuffer.GetAddressOf());

		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		const UINT strides[] = { sizeof(_OBJ_VERT_) }; // TODO: Part 1E - done
		const UINT offsets[] = { 0 };
		ID3D11Buffer* const buffs[] = { vertexBuffer.Get() };
		handles.context->IASetVertexBuffers(0, ARRAYSIZE(buffs), buffs, strides, offsets);
	}

	void SetShaders(PipelineHandles handles)
	{
		handles.context->VSSetShader(vertexShader.Get(), nullptr, 0);
		handles.context->PSSetShader(pixelShader.Get(), nullptr, 0);
	}

	void ReleasePipelineHandles(PipelineHandles toRelease)
	{
		toRelease.depthStencil->Release();
		toRelease.targetView->Release();
		toRelease.context->Release();
	}


public:
	~Renderer()
	{
		// ComPtr will auto release so nothing to do here yet 
	}
};
