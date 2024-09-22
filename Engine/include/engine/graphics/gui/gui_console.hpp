//Copyright(C) 2024 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once
#if ENGINE_MODE
#include <vector>
#include <string>

using std::vector;
using std::string;

namespace Graphics::GUI
{
	class GUIConsole
	{
	public:
		static inline bool firstScrollToBottom;

		static inline vector<string> consoleMessages;

		static void RenderConsole();
		static void AddTextToConsole(const string& message);
	private:
		static inline char inputTextBuffer[128];
		static constexpr int maxConsoleMessages = 1000;
	};
}
#endif