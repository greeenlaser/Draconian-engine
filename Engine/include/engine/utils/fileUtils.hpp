//<Elypso engine>
//    Copyright(C) < 2024 > < Greenlaser >
//
//    This program is free software : you can redistribute it and /or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License in LICENCE.md
//    and a copy of the EULA in EULA.md along with this program. 
//    If not, see < https://github.com/Lost-Empire-Entertainment/Elypso-engine >.

#pragma once

#include <string>
#include <filesystem>

using std::string;
using std::filesystem::path;

namespace Utils
{
	class File
	{
	public:
		/// <summary>
		/// Return the output from a bat file as a string.
		/// </summary>
		/// <param name="file">Where is the file located at?</param>
		/// <returns></returns>
		static string GetOutputFromBatFile(const char* file);

		/// <summary>
		/// Move or rename the selected file or folder.
		/// </summary>
		/// <param name="itemPath">Full path to the file or folder you are trying to move or rename.</param>
		/// <param name="targetPath">Full path to the target destination.</param>
		/// <param name="isRenaming">Should the file or folder be renamed?</param>
		static void MoveOrRenameFileOrFolder(path& itemPath, path& targetPath, bool isRenaming);

		/// <summary>
		/// Copy the selected file or folder to the target path.
		/// </summary>
		/// <param name="itemPath">Full path to the file or folder you are trying to copy.</param>
		/// <param name="targetPath">Full path to the target destination.</param>
		static void CopyFileOrFolder(path& itemPath, path& targetPath);

		/// <summary>
		/// Delete a file or folder.
		/// </summary>
		/// <param name="itemPath">Full path to the file or folder you are trying to delete.</param>
		static void DeleteFileOrfolder(path& itemPath);

		/// <summary>
		/// Create a new folder at the target destination.
		/// </summary>
		/// <param name="folderPath">Full path to the new folder you are trying to create the folder to.</param>
		static void CreateNewFolder(path& folderPath);
	private:
		static bool PathExists(path& itemPath);
	};
}