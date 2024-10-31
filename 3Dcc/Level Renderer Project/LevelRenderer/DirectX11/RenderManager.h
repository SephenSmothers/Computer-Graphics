#pragma once
#include "load_object_oriented.h"

namespace fs = std::filesystem;

class Renderer
{
	//list of models
	Level_Objects level;
	GW::SYSTEM::GWindow win;
	GW::SYSTEM::GLog log;
	GW::GRAPHICS::GDirectX11Surface d3d;
	ID3D11DeviceContext* context;
	ID3D11Device* creator;
	ID3D11RenderTargetView* targetView;
	ID3D11DepthStencilView* depthStencil;

	Microsoft::WRL::ComPtr<ID3D11Buffer> ConstSceneBuffer;

	GW::MATH::GMATRIXF view;
	GW::MATH::GMATRIXF projection;
	GW::MATH::GVECTORF lightDir;
	GW::MATH::GVECTORF lightColor;
	GW::MATH::GVECTORF rSunAmbient;
	GW::MATH::GVECTORF rCameraPos;
	SceneData sceneData;

	GW::MATH::GMatrix math;
	GW::INPUT::GInput input;
	std::chrono::high_resolution_clock::time_point lastUpdate;
	unsigned int windowWidth;
	unsigned int windowHeight;

	GW::AUDIO::GAudio audio;
	GW::AUDIO::GMusic music1, leftClkSound;

public:
	Renderer(GW::SYSTEM::GWindow _win, GW::GRAPHICS::GDirectX11Surface _d3d)
	{
		win = _win;
		d3d = _d3d;

		math.Create();
		input.Create(win);
		audio.Create();
		music1.Create("../Music/Night Of The Wolf2.wav", audio, 0.1f);
		leftClkSound.Create("../Music/swordclash.wav", audio, 1.0f);
		music1.Play(true);
		win.GetWidth(windowWidth);
		win.GetHeight(windowHeight);
		lastUpdate = std::chrono::high_resolution_clock::now();

		//view
		GW::MATH::GMatrix::IdentityF(view);

		GW::MATH::GMatrix::LookAtLHF
		(
			GW::MATH::GVECTORF{ 0.75f, 0.25f, -1.5f, 0.0f },
			GW::MATH::GVECTORF{ 0.15f, 0.75f, 0.0f, 0.0f },
			GW::MATH::GVECTORF{ 0.0f, 1.0f, 0.0f, 0.0f }, view
		);

		//projection
		GW::MATH::GMatrix::IdentityF(projection);
		float ratio = 0.0f;
		d3d.GetAspectRatio(ratio);
		GW::MATH::GMatrix::ProjectionDirectXLHF(G_DEGREE_TO_RADIAN_F(65.0f), ratio, 0.1f, 100.0f, projection);

		//lighting
		lightDir = { -1.0f, 1.0f, 2.0f, 1.0f };
		lightColor = { 0.9f, 0.9f, 1.0f, 1.0f };
		rSunAmbient = { 0.25f,0.25f,0.35f,1.0f };
		rCameraPos = view.row4;

		//init stuct data
		sceneData.sunDirection = lightDir;
		sceneData.sunColor = lightColor;
		sceneData.sunAmbient = rSunAmbient;
		sceneData.cameraPos = rCameraPos;
		sceneData.viewMatrix = view;
		sceneData.projectionMatrix = projection;

		//loading the level

		log.Create("Errors.txt");
		log.EnableConsoleLogging("Errors.txt");
		bool success = false;
		success = level.LoadLevel("../GameLevel.txt", "../Models", log);

		level.UploadLevelToGPU(win, d3d);
	}

	void Render()
	{
		UpdateCamera();
		level.RenderLevel(sceneData);
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
		float leftMouseClick = 0;

		GW::GReturn returnDel;

		// Get current time
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

		// Calculate the time elapsed since frame
		std::chrono::duration<double> elapsedTime = std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastUpdate);

		GW::MATH::GMATRIXF viewMatrixCpy = view;
		math.InverseF(viewMatrixCpy, viewMatrixCpy);

		//get key states
		input.GetState(G_KEY_SPACE, spaceKeyState);
		input.GetState(G_KEY_LEFTSHIFT, leftShiftState);
		input.GetState(G_KEY_W, upKeyState);
		input.GetState(G_KEY_A, leftKeyState);
		input.GetState(G_KEY_S, downKeyState);
		input.GetState(G_KEY_D, rightKeyState);
		input.GetMousePosition(xMouseKeyState, yMouseKeyState);
		returnDel = input.GetMouseDelta(deltaMouseX, deltaMouseY);
		input.GetState(G_BUTTON_LEFT, leftMouseClick);

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

		rotationSpeed = 3.14159f * secondsPassed;

		xRadians = (65 * deltaMouseY) / (windowHeight + yMouseKeyState * rotationSpeed * -1);
		yRadians = 65 * ratio * deltaMouseX / (windowWidth + xMouseKeyState * rotationSpeed);

		if (returnDel == GW::GReturn::REDUNDANT)
		{
			xRadians = 0;
			yRadians = 0;

		}
		math.RotateXLocalF(viewMatrixCpy, G_DEGREE_TO_RADIAN_F(xRadians), viewMatrixCpy);
		math.RotateYGlobalF(viewMatrixCpy, G_DEGREE_TO_RADIAN_F(yRadians), viewMatrixCpy);

		math.InverseF(viewMatrixCpy, viewMatrixCpy);
		view = viewMatrixCpy;
		sceneData.viewMatrix = view;

		if (leftMouseClick != 0) {
			leftClkSound.Play();
			leftMouseClick = 0;
		}

		// Store the current time as the last update time for the next call
		lastUpdate = currentTime;
	}

	private:

};