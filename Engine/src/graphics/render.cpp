//Copyright(C) 2024 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#include <string>
#include <iostream>
#include <filesystem>

//external
#include "glad.h"
#include "glfw3.h"
#include "glm.hpp"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"
#include "stb_image.h"

//engine
#include "console.hpp"
#include "core.hpp"
#include "gui.hpp"
#include "render.hpp"
#include "texture.hpp"
#include "timeManager.hpp"
#include "pointlight.hpp"
#include "gameobject.hpp"
#include "grid.hpp"
#include "selectedobjectaction.hpp"
#include "selectedobjectborder.hpp"
#include "sceneFile.hpp"
#include "configFile.hpp"
#include "input.hpp"
#include "stringUtils.hpp"
#include "shader.hpp"

using glm::perspective;
using glm::radians;
using glm::translate;
using glm::rotate;
using std::filesystem::exists;
using std::filesystem::current_path;
using std::cout;
using std::endl;
using std::to_string;
using std::filesystem::path;

using Core::Input;
using Core::TimeManager;
using Core::Engine;
using Graphics::Shape::GameObjectManager;
using Graphics::GUI::EngineGUI;
using Graphics::Shape::PointLight;
using Graphics::Grid;
using Graphics::Shape::ActionTex;
using Graphics::Shape::Border;
using EngineFile::SceneFile;
using Core::ConsoleManager;
using Caller = Core::ConsoleManager::Caller;
using Type = Core::ConsoleManager::Type;
using EngineFile::ConfigFile;
using Utils::String;

namespace Graphics
{
	Camera Render::camera(Render::window, 0.05f);
	unsigned int framebuffer;
	unsigned int textureColorbuffer;
	unsigned int rbo;
	int framebufferWidth = 1280;
	int framebufferHeight = 720;

	void Render::RenderSetup()
	{
		GLFWSetup();
		WindowSetup();
		GladSetup();
		FramebufferSetup();

		ContentSetup();

		EngineGUI::Initialize();

		TimeManager::InitializeDeltaTime();
	}

	void Render::GLFWSetup()
	{
		ConsoleManager::WriteConsoleMessage(
			Caller::INITIALIZE,
			Type::DEBUG,
			"Initializing GLFW...\n");

		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		ConsoleManager::WriteConsoleMessage(
			Caller::INITIALIZE,
			Type::DEBUG,
			"GLFW initialized successfully!\n\n");
	}
	void Render::WindowSetup()
	{
		ConsoleManager::WriteConsoleMessage(
			Caller::INITIALIZE,
			Type::DEBUG,
			"Creating window...\n");

		//create a window object holding all the windowing data
		window = glfwCreateWindow(
			1280,
			720,
			(Engine::name + " " + Engine::version).c_str(),
			NULL,
			NULL);

		if (window == NULL)
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::INITIALIZE,
				Type::EXCEPTION,
				"Failed to create GLFW window!\n\n");
			return;
		}

		glfwMakeContextCurrent(window);
		glfwSetFramebufferSizeCallback(window, UpdateAfterRescale);
		glfwSetWindowSizeLimits(window, 1280, 720, 7680, 4320);
		glfwSwapInterval(stoi(ConfigFile::GetValue("window_vsync")));

		int width, height, channels;
		string iconpath = Engine::filesPath + "\\icon.png";
		unsigned char* iconData = stbi_load(iconpath.c_str(), &width, &height, &channels, 4);

		GLFWimage icon{};
		icon.width = width;
		icon.height = height;
		icon.pixels = iconData;

		glfwSetWindowIcon(window, 1, &icon);

		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		glfwSetMouseButtonCallback(window, Input::MouseButtonCallback);
		glfwSetScrollCallback(window, Input::ScrollCallback);
		glfwSetKeyCallback(window, Input::KeyCallback);
		glfwSetCursorPosCallback(window, Input::MouseMovementCallback);

		glfwSetWindowCloseCallback(window, [](GLFWwindow* window) { Engine::Shutdown(); });

		ConsoleManager::WriteConsoleMessage(
			Caller::INITIALIZE,
			Type::DEBUG,
			"Window initialized successfully!\n\n");
	}
	void Render::GladSetup()
	{
		ConsoleManager::WriteConsoleMessage(
			Caller::INITIALIZE,
			Type::DEBUG,
			"Initializing GLAD...\n");

		//check if glad is initialized before continuing
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::INITIALIZE,
				Type::EXCEPTION,
				"Failed to initialize GLAD!\n\n");
			return;
		}

		ConsoleManager::WriteConsoleMessage(
			Caller::INITIALIZE,
			Type::DEBUG,
			"GLAD initialized successfully!\n\n");
	}
	void Render::FramebufferSetup()
	{
		//set up framebuffer
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

		//set up color attachment texture
		glGenTextures(1, &textureColorbuffer);
		glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
		glTexImage2D(
			GL_TEXTURE_2D, 
			0, 
			GL_RGB, 
			1280, 
			720, 
			0, 
			GL_RGB, 
			GL_UNSIGNED_BYTE, 
			NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glFramebufferTexture2D(
			GL_FRAMEBUFFER, 
			GL_COLOR_ATTACHMENT0, 
			GL_TEXTURE_2D, 
			textureColorbuffer, 
			0);

		//set up renderbuffer object 
		//for depth and stencil attachment
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(
			GL_RENDERBUFFER, 
			GL_DEPTH24_STENCIL8, 
			1280, 
			720);
		glFramebufferRenderbuffer(
			GL_FRAMEBUFFER, 
			GL_DEPTH_STENCIL_ATTACHMENT, 
			GL_RENDERBUFFER, 
			rbo);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			ConsoleManager::WriteConsoleMessage(
				Caller::INITIALIZE,
				Type::EXCEPTION,
				"Framebuffer is not complete!\n\n");
			return;
		}
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
	void Render::ContentSetup()
	{
		//enable face culling
		glEnable(GL_CULL_FACE);
		//cull back faces
		glCullFace(GL_BACK);
		//enable depth testing
		glEnable(GL_DEPTH_TEST);
		//enable blending
		glEnable(GL_BLEND);
		//set blending function
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		Grid::InitializeGrid();

		shared_ptr<GameObject> border = Border::InitializeBorder();
		GameObjectManager::SetBorder(border);
		GameObjectManager::AddOpaqueObject(border);

		shared_ptr<GameObject> actionTex = ActionTex::InitializeActionTex();
		GameObjectManager::SetActionTex(actionTex);
		GameObjectManager::AddTransparentObject(actionTex);

		glfwMaximizeWindow(window);
	}

	void Render::UpdateAfterRescale(GLFWwindow* window, int width, int height)
	{
	}

	void Render::SetWindowNameAsUnsaved(bool state)
	{
		SceneFile::unsavedChanges = state;
		
		string sceneName = path(Engine::scenePath).parent_path().stem().string();
		string windowTitle = 
			Engine::name + " " 
			+ Engine::version 
			+ " [" + sceneName + "]";

		string newName = state == true
			? windowTitle + " *"
			: windowTitle;

		glfwSetWindowTitle(window, newName.c_str());
	}

	void Render::WindowLoop()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glEnable(GL_DEPTH_TEST);
		glClearColor(
			backgroundColor.x,
			backgroundColor.y,
			backgroundColor.z,
			1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//camera transformation
		Input::ProcessKeyboardInput(window);

		//calculate the new projection matrix
		float fov = stof(ConfigFile::GetValue("camera_fov"));
		float nearClip = stof(ConfigFile::GetValue("camera_nearClip"));
		float farClip = stof(ConfigFile::GetValue("camera_farClip"));
		projection = perspective(
			radians(fov),
			Camera::aspectRatio,
			nearClip,
			farClip);

		//update the camera
		view = camera.GetViewMatrix();

		Grid::RenderGrid(view, projection);

		GameObjectManager::RenderAll(view, projection);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glClearColor(0.2f, 0.3f, 0.4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//all windows, including RenderToImguiWindow 
		//with scene content are called in the Render function
		EngineGUI::Render();

		//swap the front and back buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	void Render::RenderToImguiWindow()
	{
		ImGui::SetNextWindowSizeConstraints(ImVec2(300, 300), ImVec2(5000, 5000));
		ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowPos(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		ImGuiWindowFlags windowFlags =
			ImGuiWindowFlags_NoCollapse;

		if (ImGui::Begin("Scene", NULL, windowFlags))
		{
			ImVec2 contentRegionMin = ImGui::GetWindowContentRegionMin();
			ImVec2 contentRegionMax = ImGui::GetWindowContentRegionMax();
			ImVec2 availableSize = ImVec2(
				contentRegionMax.x - contentRegionMin.x, 
				contentRegionMax.y - contentRegionMin.y);

			float windowAspectRatio = availableSize.x / availableSize.y;
			float targetAspectRatio = windowAspectRatio;

			if (aspectRatio[currentIndex] == "16:9")
				targetAspectRatio = 16.0f / 9.0f;
			else if (aspectRatio[currentIndex] == "16:10")
				targetAspectRatio = 16.0f / 10.0f;
			else if (aspectRatio[currentIndex] == "21:9")
				targetAspectRatio = 21.0f / 9.0f;
			else if (aspectRatio[currentIndex] == "32:9")
				targetAspectRatio = 32.0f / 9.0f;
			else if (aspectRatio[currentIndex] == "4:3")
				targetAspectRatio = 4.0f / 3.0f;

			ImVec2 renderSize = availableSize;
			if (windowAspectRatio > targetAspectRatio)
			{
				renderSize.x = availableSize.y * targetAspectRatio;
			}
			else if (windowAspectRatio < targetAspectRatio)
			{
				renderSize.y = availableSize.x / targetAspectRatio;
			}

			renderSize.x = roundf(renderSize.x);
			renderSize.y = roundf(renderSize.y);

			ImVec2 padding(
				(availableSize.x - renderSize.x) * 0.5f,
				(availableSize.y - renderSize.y) * 0.5f);
			ImGui::SetCursorPos(ImVec2(
				contentRegionMin.x + padding.x, 
				contentRegionMin.y + padding.y));

			if (renderSize.x != framebufferWidth
				|| renderSize.y != framebufferHeight)
			{
				framebufferWidth = static_cast<int>(renderSize.x);
				framebufferHeight = static_cast<int>(renderSize.y);

				glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
				glTexImage2D(
					GL_TEXTURE_2D,
					0,
					GL_RGB,
					framebufferWidth,
					framebufferHeight,
					0,
					GL_RGB,
					GL_UNSIGNED_BYTE,
					NULL);

				glBindRenderbuffer(GL_RENDERBUFFER, rbo);
				glRenderbufferStorage(
					GL_RENDERBUFFER,
					GL_DEPTH24_STENCIL8,
					framebufferWidth,
					framebufferHeight);

				Camera::aspectRatio = targetAspectRatio;

				glViewport(0, 0, framebufferWidth, framebufferHeight);
			}

			isSceneSelected = ImGui::IsWindowFocused();
			if (isSceneSelected)
			{
				ImGui::CaptureMouseFromApp(false);
				Input::SceneWindowInput();
			}
			else ImGui::ResetMouseDragDelta();

			//render to imgui image and flip the Y-axis
			ImGui::Image(
				(void*)(intptr_t)textureColorbuffer, 
				renderSize, 
				ImVec2(0, 1), 
				ImVec2(1, 0));

			if (showSceneWindowDebugMenu) SceneWindowDebugMenu();

			ImVec2 toggleBtnSize = ImVec2(25.0f, 25.0f);
			float rightPadding = 10.0f;
			ImVec2 toggleBtnPos(
				ImGui::GetWindowSize().x - toggleBtnSize.x - rightPadding, 40.0f);
			string toggleBtnText = showSceneWindowDebugMenu
				? "v" : "<";
			ImGui::SetCursorPos(toggleBtnPos);
			if (ImGui::Button(toggleBtnText.c_str(), toggleBtnSize))
			{
				showSceneWindowDebugMenu = !showSceneWindowDebugMenu;
			}

			ImGui::End();
		}
	}

	void Render::SceneWindowDebugMenu()
	{
		ImVec2 childSize = ImVec2(300.0f, 500.0f);
		float rightPadding = 40.0f;
		ImVec2 childPos(
			ImGui::GetWindowSize().x - childSize.x - rightPadding, 40.0f);
		ImGui::SetCursorPos(childPos);

		ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));

		if (ImGui::BeginChild("##sceneWindowDebugMenu", childSize))
		{
			//
			// GENERAL
			//

			ImGui::Text("FPS: %.2f", TimeManager::displayedFPS);

			string strObjectsCount = "Objects: " + to_string(objectsCount);
			ImGui::Text(strObjectsCount.c_str());
			string strVerticesCount = "Vertices: " + to_string(verticesCount);
			ImGui::Text(strVerticesCount.c_str());

			ImGui::Text(
				"Position: %.2f, %.2f, %.2f",
				Render::camera.GetCameraPosition().x,
				Render::camera.GetCameraPosition().y,
				Render::camera.GetCameraPosition().z);
			ImGui::Text(
				"Angle: %.2f, %.2f, %.2f",
				Render::camera.GetCameraRotation().x,
				Render::camera.GetCameraRotation().y,
				Render::camera.GetCameraRotation().z);

			ImGui::Text("Current axis: %s", Input::axis.c_str());
			ImGui::Text("Current tool: %s", string(magic_enum::enum_name(Input::objectAction)).c_str());

			//
			// RENDER SETTINGS
			//

			ImGui::Separator();

			ImGui::Text("Toggle VSync");
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 50);
			bool vsyncEnabled = stoi(ConfigFile::GetValue("window_vsync"));
			if (ImGui::Checkbox("##vsync", &vsyncEnabled))
			{
				glfwSwapInterval(vsyncEnabled ? 1 : 0);
				ConfigFile::SetValue("window_vsync", to_string(vsyncEnabled));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			ImGui::Text("Toggle aspect ratio");
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - 100);
			Render::currentIndex = stoi(ConfigFile::GetValue("aspect_ratio"));
			string aspectRatioValue = Render::aspectRatio[Render::currentIndex];
			if (ImGui::Button(aspectRatioValue.c_str()))
			{
				Render::currentIndex++;
				if (Render::currentIndex >= Render::aspectRatio.size())
				{
					Render::currentIndex = 0;
				}
				ConfigFile::SetValue("aspect_ratio", to_string(Render::currentIndex));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			//
			// CAMERA SETTINGS
			//

			ImGui::Separator();

			ImGui::Text("FOV");
			float fov = stof(ConfigFile::GetValue("camera_fov"));
			if (ImGui::DragFloat("##fov", &fov, 0.1f, 70.0f, 110.0f))
			{
				if (fov > 110.0f) fov = 110.0f;
				if (fov < 70.0f) fov = 70.0f;

				ConfigFile::SetValue("camera_fov", to_string(fov));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			ImGui::Text("Camera near clip");
			float nearClip = stof(ConfigFile::GetValue("camera_nearClip"));
			float farClip = stof(ConfigFile::GetValue("camera_farClip"));
			if (ImGui::DragFloat("##camNearClip", &nearClip, 0.1f, 0.001f, farClip - 0.001f))
			{
				if (nearClip > farClip - 0.001f) nearClip = farClip - 0.001f;
				if (nearClip > 0.5f) nearClip = 0.5f;
				if (nearClip < 0.001f) nearClip = 0.001f;

				ConfigFile::SetValue("camera_nearClip", to_string(nearClip));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			ImGui::Text("Camera far clip");
			if (ImGui::DragFloat("##camFarClip", &farClip, 0.1f, nearClip + 0.001f, 10000))
			{
				if (farClip > 10000.0f) farClip = 10000.0f;
				if (farClip < nearClip + 0.001f) farClip = nearClip + 0.001f;
				if (farClip < 50.0f) farClip = 50.0f;

				ConfigFile::SetValue("camera_farClip", to_string(farClip));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			ImGui::Text("Camera move speed multiplier");
			float moveSpeed = stof(ConfigFile::GetValue("camera_speedMultiplier"));
			if (ImGui::DragFloat("##camMoveSpeed", &moveSpeed, 0.1f, 0.1f, 100.0))
			{
				if (moveSpeed > 100.0f) moveSpeed = 100.0f;
				if (moveSpeed < 0.1f) moveSpeed = 0.1f;

				ConfigFile::SetValue("camera_speedMultiplier", to_string(moveSpeed));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset##camMoveSpeed"))
			{
				ConfigFile::SetValue("camera_speedMultiplier", "1.0");
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			//
			// GRID SETTINGS
			//

			ImGui::Separator();

			ImGui::Text("Grid color");
			string gridColorString = ConfigFile::GetValue("grid_color");
			vector<string> gridColorSplit = String::Split(gridColorString, ',');
			vec3 gridColor = vec3(
				stof(gridColorSplit[0]),
				stof(gridColorSplit[1]),
				stof(gridColorSplit[2]));
			if (ImGui::ColorEdit3("##gridColor", value_ptr(gridColor)))
			{
				string finalString =
					to_string(gridColor[0]) + "," +
					to_string(gridColor[1]) + "," +
					to_string(gridColor[2]);

				ConfigFile::SetValue("grid_color", finalString);
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}
			ImGui::SameLine();
			if (ImGui::Button("Reset##gridColor"))
			{
				ConfigFile::SetValue("grid_color", "0.4, 0.4, 0.4");
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			ImGui::Text("Grid transparency");
			float gridTransparency = stof(ConfigFile::GetValue("grid_transparency"));
			if (ImGui::DragFloat("##gridTransparency", &gridTransparency, 0.001f, 0.0f, 1.0f))
			{
				if (gridTransparency > 1.0f) gridTransparency = 1.0f;
				if (gridTransparency < 0.0f) gridTransparency = 0.0f;

				ConfigFile::SetValue("grid_transparency", to_string(gridTransparency));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			ImGui::Text("Grid max distance");
			float gridMaxDistance = stof(ConfigFile::GetValue("grid_maxDistance"));

			if (gridMaxDistance > farClip)
			{
				gridMaxDistance = farClip;
				ConfigFile::SetValue("grid_maxDistance", to_string(gridMaxDistance));
			}
			if (ImGui::DragFloat("##gridMaxDistance", &gridMaxDistance, 0.1f, 10.0f, 200.0f))
			{
				if (gridMaxDistance > 200.0f) gridMaxDistance = 200.0f;
				if (gridMaxDistance < 10.0f) gridMaxDistance = 10.0f;

				ConfigFile::SetValue("grid_maxDistance", to_string(gridMaxDistance));
				if (!SceneFile::unsavedChanges) Render::SetWindowNameAsUnsaved(true);
			}

			ImGui::EndChild();
		}

		ImGui::PopStyleColor();
	}

	void Render::UpdateCounts()
	{
		if (!waitBeforeCountsUpdate)
		{
			objectsCount = static_cast<int>(GameObjectManager::GetObjects().size());

			verticesCount = 0;
			for (const shared_ptr<GameObject>& obj : GameObjectManager::GetObjects())
			{
				verticesCount += static_cast<int>(obj->GetMesh()->GetVertices().size());
			}
		}
	}
}