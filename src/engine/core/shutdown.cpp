//external
#include "glad.h"
#include "glfw3.h"

//engine
#include "console.h"
#include "render.h"
#include "shutdown.h"

namespace Core
{
	void ShutdownManager::Shutdown()
	{
		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::SHUTDOWN,
			Core::Console::ConsoleManager::Type::INFO,
			"Cleaning up resources...\n");

		Graphics::Render::Shutdown();

		//de-allocate all resources once they've outlived their purpose
		glDeleteVertexArrays(1, &Graphics::Render::VAO);
		glDeleteBuffers(1, &Graphics::Render::VBO);
		glDeleteBuffers(1, &Graphics::Render::EBO);

		//clean all glfw resources after program is closed
		glfwTerminate();

		Core::Console::ConsoleManager::WriteConsoleMessage(
			Core::Console::ConsoleManager::Caller::SHUTDOWN,
			Core::Console::ConsoleManager::Type::SUCCESS,
			"Shutdown complete!\n");
	}
}