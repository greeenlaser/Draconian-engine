//external
#include "glad.h"
#include "glfw3.h"

//engine
#include "core.h"
#include "render.h"
#include "shader.h"
#include "texture.h"
#include "console.h"
#include "shutdown.h"
#include "search.h"

#include <string>
#include <iostream>

namespace Graphics
{
	Shader* Render::shader;
	bool Render::shaderInitialized = false;

	int Render::WindowSetup()
	{
		std::string projectPath = Core::Search::SearchByParent("Elypso engine");
		std::string vertexPath = projectPath + "\\src\\engine\\graphics\\shaders\\vertexShader.vs";
		std::string fragmentPath = projectPath + "\\src\\engine\\graphics\\shaders\\fragmentShader.fs";
		std::string testTexturePath = projectPath + "\\files\\testTexture.jpg";

		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::GLFW,
			Core::Console::ConsoleManager::Type::INFO,
			"Initializing GLFW...\n");
		glfwInit();
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::GLFW,
			Core::Console::ConsoleManager::Type::SUCCESS,
			"GLFW initialized successfully!\n\n");

		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::WINDOW_SETUP,
			Core::Console::ConsoleManager::Type::INFO,
			"Creating window...\n");
		//create a window object holding all the windowing data
		Render::window = glfwCreateWindow(
			Render::SCR_WIDTH,
			Render::SCR_HEIGHT,
			"Elypso engine",
			NULL,
			NULL);
		if (Render::window == NULL)
		{
			Core::Console::ConsoleManager::WriteConsoleMessage(
				Core::Console::ConsoleManager::Caller::GLFW,
				Core::Console::ConsoleManager::Type::ERROR,
				"Error: Failed to create GLFW window!\n\n");
			Core::ShutdownManager::Shutdown();
			return -1;
		}
		glfwMakeContextCurrent(Render::window);
		glfwSetFramebufferSizeCallback(
			Render::window,
			Render::UpdateAfterRescale);
		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::WINDOW_SETUP,
			Core::Console::ConsoleManager::Type::SUCCESS,
			"Window initialized successfully!\n\n");

		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::GLAD,
			Core::Console::ConsoleManager::Type::INFO,
			"Initializing GLAD...\n");
		//check if glad is initialized before continuing
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		{
			Core::Console::ConsoleManager::WriteConsoleMessage(
				Core::Console::ConsoleManager::Caller::GLAD,
				Core::Console::ConsoleManager::Type::ERROR,
				"Error: Failed to initialize GLAD!\n\n");
			return -1;
		}
		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::GLAD,
			Core::Console::ConsoleManager::Type::SUCCESS,
			"GLAD initialized successfully!\n\n");

		if (!shaderInitialized)
		{
			shader = new Shader(vertexPath, fragmentPath);

			shaderInitialized = true;
		}

		float vertices[] =
		{
			//positions          //colors           //texture coords
			 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
			 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 
		};
		unsigned int indices[] =
		{
			0, 1, 3, //first triangle
			1, 2, 3  //second triangle
		};

		glGenVertexArrays(1, &Render::VAO);
		glGenBuffers(1, &Render::VBO);
		glGenBuffers(1, &Render::EBO);

		glBindVertexArray(Render::VAO);

		glBindBuffer(GL_ARRAY_BUFFER, Render::VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Render::EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

		//position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		//color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		//texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		Texture tex(testTexturePath);
		tex.LoadTexture();

		return 0;
	}

	void Render::UpdateAfterRescale(GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void Render::Shutdown()
	{
		delete shader;
		shader = nullptr;
	}

	void Render::WindowLoop()
	{
		//clear the background to dark green
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		//clear color
		glClear(GL_COLOR_BUFFER_BIT);

		//bind texture
		glBindTexture(GL_TEXTURE_2D, Texture::texture);

		//render the triangle
		if (shader)
		{
			shader->Use();
			glBindVertexArray(Render::VAO);
			glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		}

		//swap the front and back buffers
		glfwSwapBuffers(Render::window);
		//poll for events and process them
		glfwPollEvents();
	}
}