//Copyright(C) 2024 Lost Empire Entertainment
//This program comes with ABSOLUTELY NO WARRANTY.
//This is free software, and you are welcome to redistribute it under certain conditions.
//Read LICENSE.md for more information.

#pragma once
#if ENGINE_MODE
#include <map>
#include <string>

#include "configFile.hpp"

using EngineFile::ConfigFile;

using std::map;
using std::string;

namespace Graphics::GUI
{
	class GUISettings
	{
	public:
		static inline map<string, string> skyboxTextures;

		static inline bool renderSettings;
		static void RenderSettings();

	private:
		enum class NameDenyReason
		{
			invalidCharacter,
			invalidName
		};
		static inline NameDenyReason nameDenyReason;

		static constexpr int bufferSize = 32;
		static inline char gameNameChar[bufferSize];

		static void InputSettings();
		static void GUIStyleSettings();
		static void GraphicsSettings();
		static void OtherSettings();
	};
}
#endif