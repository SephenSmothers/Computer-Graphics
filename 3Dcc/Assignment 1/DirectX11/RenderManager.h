#include <d3dcompiler.h>
#pragma comment(lib, "d3dcompiler.lib") //needed for runtime shader compilation. Consider compiling shaders before runtime 

void PrintLabeledDebugString(const char* label, const char* toPrint)
{
	std::cout << label << toPrint << std::endl;
#if defined WIN32 //OutputDebugStringA is a windows-only function 
	OutputDebugStringA(label);
	OutputDebugStringA(toPrint);
#endif
}

// TODO: Part 1C - done

struct Vertex
{
	float x, y, z, w;
};

struct SHADER_VARS
{
	GW::MATH::GMATRIXF worldMatrix;
	GW::MATH::GMATRIXF viewMatrix;
	GW::MATH::GMATRIXF projectionMatrix;
};


// TODO: Part 2B - done
// TODO: Part 2G - done

class RenderManager
{
	// proxy handles
	GW::SYSTEM::GWindow win;
	GW::GRAPHICS::GDirectX11Surface d3d;
	// what we need at a minimum to draw a triangle
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	vertexFormat;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	//The Variables I made
	GW::MATH::GMATRIXF worldMatrices[6];
	GW::MATH::GMATRIXF rViewMatrix;
	GW::MATH::GMATRIXF rProjectionMatrix;

	GW::MATH::GMatrix math;
	SHADER_VARS shader;
	GW::INPUT::GInput input;
	std::chrono::high_resolution_clock::time_point lastUpdate;
	unsigned int windowWidth;
	unsigned int windowHeight;

	// TODO: Part 2A - done
	// TODO: Part 2C - done
	// TODO: Part 2D - done
	// TODO: Part 2G - done
	// TODO: Part 3A - done
	// TODO: Part 3C - done
	// TODO: Part 4A - done

public:
	RenderManager(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;
		// TODO: Part 2A - done
		// TODO: Part 2C - done
		// TODO: Part 2G - done
		// TODO: Part 3A - done
		// TODO: Part 3B - done
		// TODO: Part 3C - done
		// TODO: Part 4A - done

		math.Create();
		input.Create(win);
		win.GetWidth(windowWidth);
		win.GetHeight(windowHeight);
		lastUpdate = std::chrono::high_resolution_clock::now();

		InitWorldMatrix(worldMatrices);
		InitViewMatrix(rViewMatrix);
		InitProjectionMatrix(rProjectionMatrix, _d3d);

		math.IdentityF(shader.worldMatrix);
		math.IdentityF(shader.viewMatrix);
		math.IdentityF(shader.projectionMatrix);

		shader.viewMatrix = rViewMatrix;
		shader.projectionMatrix = rProjectionMatrix;

		InitializeGraphics();
	}
private:
	//Constructor helper functions 
	void InitializeGraphics()
	{
		ID3D11Device* creator;
		d3d.GetDevice((void**)&creator);

		InitializeVertexBuffer(creator);

		//TODO: Part 2D - done

		D3D11_SUBRESOURCE_DATA bData = { &shader, 0, 0 };
		CD3D11_BUFFER_DESC bDesc(sizeof(SHADER_VARS), D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);
		creator->CreateBuffer(&bDesc, &bData, constantBuffer.GetAddressOf());

		InitializePipeline(creator);

		// free temporary handle
		creator->Release();
	}




	void InitializeVertexBuffer(ID3D11Device* creator)
	{
		// TODO: Part 1B - done 
		// TODO: Part 1C - done 
		// TODO: Part 1D - done 

		const int arraySize = 104;

		Vertex verts[arraySize];

		float left = 0.0f;
		float right = 0.0f;
		float top = 0.0f;
		float bottom = 0.0f;

		// Generate vertices for the grid
		int vertexIndex = 0;

		left = -0.5f;
		right = 0.5f;
		top = 0.5f;
		bottom = 0.5f;
		for (int i = 0; i < arraySize / 2; i += 2)
		{
			verts[vertexIndex++] = { left, top, 0.0f, 1.0f };
			verts[vertexIndex++] = { right, bottom, 0.0f, 1.0f };
			top -= 0.04f;
			bottom -= 0.04f;
		}

		left = -0.5f;
		right = -0.5f;
		top = 0.5f;
		bottom = -0.5f;
		for (int j = arraySize / 2; j < arraySize; j += 2)
		{
			verts[vertexIndex++] = { left, top, 0.0f, 1.0f };
			verts[vertexIndex++] = { right, bottom, 0.0f, 1.0f };
			left += 0.04f;
			right += 0.04f;
		}

		CreateVertexBuffer(creator, &verts[0], sizeof(verts));
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
		// TODO: Part 1C - done 
		D3D11_INPUT_ELEMENT_DESC attributes[1];

		attributes[0].SemanticName = "POSITION";
		attributes[0].SemanticIndex = 0;
		attributes[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		attributes[0].InputSlot = 0;
		attributes[0].AlignedByteOffset = 0;
		attributes[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
		attributes[0].InstanceDataStepRate = 0;

		creator->CreateInputLayout(attributes, ARRAYSIZE(attributes),
			vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
			vertexFormat.GetAddressOf());
	}


public:
	void Render()
	{
		PipelineHandles curHandles = GetCurrentPipelineHandles();
		SetUpPipeline(curHandles);

		// TODO: Part 1B - done 
		// TODO: Part 1D - done 
		// TODO: Part 3D - done

		D3D11_MAPPED_SUBRESOURCE mappedResource = {};
		ZeroMemory(&mappedResource, sizeof(D3D11_MAPPED_SUBRESOURCE));

		HRESULT res;

		for (int i = 0; i < 6; i++) 
		{
			res = curHandles.context->Map(constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			// Update the world matrix in the SHADER_VARS instance
			shader.worldMatrix = worldMatrices[i];

			// Copy the updated data into the constant buffer
			memcpy(mappedResource.pData, &shader, sizeof(SHADER_VARS));

			// Draw the grid
			curHandles.context->Unmap(constantBuffer.Get(), 0);
			curHandles.context->Draw(104, 0);
		}

		ReleasePipelineHandles(curHandles);
	}

	void UpdateCamera()
	{
		float deltaX = 0;
		float deltaY = 0;
		float deltaZ = 0;
		float deltaMouseX = 0;
		float deltaMouseY = 0;
		float cameraSpeed = 0.3f;
		float rotationSpeed = 0.0f;
		float xRadians = 0.0f;
		float yRadians = 0.0f;

		//key state holders
		float spaceKeyState = 0;
		float leftShiftState = 0;
		float leftKeyState = 0;
		float rightKeyState = 0;
		float upKeyState = 0;
		float downKeyState = 0;
		float xMouseKeyState = 0;
		float yMouseKeyState = 0;

		GW::GReturn res;

		// Get current time
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

		// Calculate the time elapsed since frame
		std::chrono::duration<double> elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastUpdate);

		GW::MATH::GMATRIXF viewMatrixCpy = shader.viewMatrix;
		math.InverseF(viewMatrixCpy, viewMatrixCpy);

		//get key states
		input.GetState(G_KEY_SPACE, spaceKeyState);
		input.GetState(G_KEY_LEFTSHIFT, leftShiftState);
		input.GetState(G_KEY_W, upKeyState);
		input.GetState(G_KEY_A, leftKeyState);
		input.GetState(G_KEY_S, downKeyState);
		input.GetState(G_KEY_D, rightKeyState);
		input.GetMousePosition(xMouseKeyState, yMouseKeyState);
		res = input.GetMouseDelta(deltaMouseX, deltaMouseY);

		deltaX = rightKeyState - leftKeyState;
		deltaY = spaceKeyState - leftShiftState;
		deltaZ = upKeyState - downKeyState;

		float secondsPassed = (float)(elapsedTime.count());
		float yTranslate = deltaY * cameraSpeed * secondsPassed;

		float ratio = (float)windowWidth / (float)windowHeight;

		//wasd movement
		GW::MATH::GMATRIXF translationMatrix;
		math.IdentityF(translationMatrix);

		translationMatrix.row4.x = deltaX * (float)elapsedTime.count();
		translationMatrix.row4.y = 0;
		translationMatrix.row4.z = deltaZ * (float)elapsedTime.count();

		math.TranslateGlobalF(viewMatrixCpy, GW::MATH::GVECTORF{ 0.0f, yTranslate, 0.0f, 0.0f }, viewMatrixCpy);
		math.MultiplyMatrixF(translationMatrix, viewMatrixCpy, viewMatrixCpy);
		
		//mouse movement
		GW::MATH::GMATRIXF xRotationMatrix;
		math.IdentityF(xRotationMatrix);

		GW::MATH::GMATRIXF yRotationMatrix;
		math.IdentityF(yRotationMatrix);

		rotationSpeed = 3.14159f * secondsPassed;

		xRadians = (65 * deltaMouseY) / (windowHeight + yMouseKeyState * rotationSpeed * -1);
		yRadians = 65 * ratio * deltaMouseX / (windowWidth + xMouseKeyState * rotationSpeed);

		if (res == GW::GReturn::REDUNDANT)
		{
			xRadians = 0;
			yRadians = 0;
		}

		math.RotateXLocalF(xRotationMatrix, G_DEGREE_TO_RADIAN_F(xRadians), xRotationMatrix);
		math.RotateYLocalF(yRotationMatrix, G_DEGREE_TO_RADIAN_F(yRadians), yRotationMatrix);

		math.MultiplyMatrixF(xRotationMatrix, viewMatrixCpy, viewMatrixCpy);
		math.MultiplyMatrixF(yRotationMatrix, viewMatrixCpy, viewMatrixCpy);

		math.InverseF(viewMatrixCpy, viewMatrixCpy);
		shader.viewMatrix = viewMatrixCpy;

		// Store the current time as the last update time for the next call
		lastUpdate = currentTime;
	}

	// TODO: Part 4B - done
	// TODO: Part 4C - done
	// TODO: Part 4D - done
	// TODO: Part 4E - done
	// TODO: Part 4F - done
	// TODO: Part 4G - done

private:
	struct PipelineHandles
	{
		ID3D11DeviceContext* context;
		ID3D11RenderTargetView* targetView;
		ID3D11DepthStencilView* depthStencil;
	};
	//Render helper functions
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

		handles.context->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());
		//TODO: Part 2E - done
		handles.context->IASetInputLayout(vertexFormat.Get());
		handles.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST); //TODO: Part 1B - done 
	}

	void SetRenderTargets(PipelineHandles handles)
	{
		ID3D11RenderTargetView* const views[] = { handles.targetView };
		handles.context->OMSetRenderTargets(ARRAYSIZE(views), views, handles.depthStencil);
	}

	void SetVertexBuffers(PipelineHandles handles)
	{
		// TODO: Part 1C - done 
		const UINT strides[] = { sizeof(float) * 4 };
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

	void InitWorldMatrix(GW::MATH::GMATRIXF* _matrix)
	{
		for (int i = 0; i < 6; i++) {
			math.IdentityF(worldMatrices[i]);
		}

		//front
		math.TranslateGlobalF(worldMatrices[0], GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.0f, 0.0f }, worldMatrices[0]);

		//right
		math.TranslateGlobalF(worldMatrices[1], GW::MATH::GVECTORF{ 0.5f, 0.0f, 0.0f, 0.0f }, worldMatrices[1]);
		math.TranslateGlobalF(worldMatrices[1], GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.5f, 0.0f }, worldMatrices[1]);
		math.RotateYGlobalF(worldMatrices[1], G_DEGREE_TO_RADIAN_F(90), worldMatrices[1]);

		//left
		math.RotateYGlobalF(worldMatrices[2], G_DEGREE_TO_RADIAN_F(90), worldMatrices[2]);
		math.TranslateGlobalF(worldMatrices[2], GW::MATH::GVECTORF{ -0.5f, 0.0f, 0.0f, 0.0f }, worldMatrices[2]);
		math.TranslateGlobalF(worldMatrices[2], GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.5f, 0.0f }, worldMatrices[2]);

		//back
		math.TranslateGlobalF(worldMatrices[3], GW::MATH::GVECTORF{ 0.0f, 0.0f, 1.0f, 0.0f }, worldMatrices[3]);
		 
		//top
		math.RotateXGlobalF(worldMatrices[4], G_DEGREE_TO_RADIAN_F(90), worldMatrices[4]);
		math.TranslateGlobalF(worldMatrices[4], GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.5f, 0.0f }, worldMatrices[4]);
		math.TranslateGlobalF(worldMatrices[4], GW::MATH::GVECTORF{ 0.0f, 0.5f, 0.0f, 0.0f }, worldMatrices[4]);

		//bottom
		math.RotateXGlobalF(worldMatrices[5], G_DEGREE_TO_RADIAN_F(90), worldMatrices[5]);
		math.TranslateGlobalF(worldMatrices[5], GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.5f, 0.0f }, worldMatrices[5]);
		math.TranslateGlobalF(worldMatrices[5], GW::MATH::GVECTORF{ 0.0f, -0.5f, 0.0f, 0.0f }, worldMatrices[5]);
	}

	void InitViewMatrix(GW::MATH::GMATRIXF& _matrix)
	{
		math.IdentityF(_matrix);
		math.LookAtLHF(GW::MATH::GVECTORF{ 0.25f, -0.125f, 0.5f, 0.0f }, GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.0f, 0.0f }, GW::MATH::GVECTORF{ 0.0f, 0.0f, 0.0f, 0.0f }, _matrix);
	}
	void InitProjectionMatrix(GW::MATH::GMATRIXF& _matrix, GW::GRAPHICS::GDirectX11Surface& surface)
	{
		float ratio = 0;
		surface.GetAspectRatio(ratio);
		GW::MATH::GMatrix::ProjectionDirectXLHF(65, ratio, 0.1, 100.0, _matrix);
	}

public:
	~RenderManager()
	{
		// ComPtr will auto release so nothing to do here yet
	}
};