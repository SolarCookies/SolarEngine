#pragma once

#include <string>
#include "Utils/ini.h"
#include <filesystem>
#include <functional>
#include "Windows/Window.h"
#include "Shaders/Texture.h"
#include "half/half.hpp"
#include "World/Camera.h"
#include "Package/VP/FileSystem/Chunks/Models/VertexTypes.h"

inline float window_width = 1920;
inline float window_height = 1080;

inline bool ExtractAll = false; // True when extracting all files from a package, this is to prevent unnecessary gpu loading as extracting all files does not have a view to display them anyway

inline Texture* PreviewTexture = nullptr; //If Valid, texture is previewed in viewport


inline std::function<void(const std::vector<unsigned char>&)> GlobalSaveFunction = nullptr;

inline std::vector<unsigned char> FileViewBuffer = {}; // Buffer for the hex file viewer (Global so all headers that include this one can access it)
inline int FileViewType = 0; // Type of file being viewed in the hex viewer for chunks this would be (0 = Dat, 1 = GPU)
inline std::string FileName = ""; // Used in details and hex viewers titles to show the name of the file being viewed

inline bool DisplayTags = false; // Display tags in the file viewer

inline std::string RootDicrectory = ""; // Contains the main executable to the game
inline std::string PackagesDicrectory = ""; // Contains all assets in the game
inline std::string BundlesDicrectory = ""; // Contains localization
inline std::string DebugPackDicrectory = ""; // Contains all assets in the game unknown as to why its duplicated
inline std::string WadDicrectory = ""; // Only used on Windows port or on the fly recompiling shaders with graphic settings
inline std::string SaveDirectory = ""; // Contains save files for the game (Only PC port at the moment due to TIP using Xenia Directory)
inline std::string GardenDirectory = "";

inline void SaveSettings() {
	// Save all settings to the ini file
	mINI::INIFile file("settings.ini");
	mINI::INIStructure ini;
	file.read(ini);
	ini["Settings"]["Root_Dicrectory"] = RootDicrectory;
	ini["Settings"]["Packages_Dicrectory"] = PackagesDicrectory;
	ini["Settings"]["Bundles_Dicrectory"] = BundlesDicrectory;
	ini["Settings"]["DebugPack_Dicrectory"] = DebugPackDicrectory;
	ini["Settings"]["Wad_Dicrectory"] = WadDicrectory;
	ini["Settings"]["Save_Directory"] = SaveDirectory;
	ini["Settings"]["Garden_Directory"] = GardenDirectory;
	file.write(ini);
}

inline void LoadSettings() {
	// Load all settings from the ini file
	mINI::INIFile file("settings.ini");
	mINI::INIStructure ini;
	file.read(ini);
	RootDicrectory = ini["Settings"]["Root_Dicrectory"].empty() ? "" : ini["Settings"]["Root_Dicrectory"];
	PackagesDicrectory = ini["Settings"]["Packages_Dicrectory"].empty() ? "" : ini["Settings"]["Packages_Dicrectory"];
	BundlesDicrectory = ini["Settings"]["Bundles_Dicrectory"].empty() ? "" : ini["Settings"]["Bundles_Dicrectory"];
	DebugPackDicrectory = ini["Settings"]["DebugPack_Dicrectory"].empty() ? "" : ini["Settings"]["DebugPack_Dicrectory"];
	WadDicrectory = ini["Settings"]["Wad_Dicrectory"].empty() ? "" : ini["Settings"]["Wad_Dicrectory"];
	SaveDirectory = ini["Settings"]["Save_Directory"].empty() ? "" : ini["Settings"]["Save_Directory"];
	GardenDirectory = ini["Settings"]["Garden_Directory"].empty() ? "" : ini["Settings"]["Garden_Directory"];
}

inline void UpdateRootDicrectory(std::string newPath) {
	RootDicrectory = newPath;

	if (std::filesystem::exists(RootDicrectory + "/Viva Pinata.exe") && std::filesystem::exists(RootDicrectory + "/bundles_packages/1.pkg")) {
		//PC
		PackagesDicrectory = RootDicrectory + "\\bundles_packages";
		BundlesDicrectory = RootDicrectory + "\\bundles";
		DebugPackDicrectory = RootDicrectory + "\\debug";
		WadDicrectory = RootDicrectory + "\\datacx";
		std::string appDataPath = std::getenv("LOCALAPPDATA") ? std::getenv("LOCALAPPDATA") : "";
		SaveDirectory = appDataPath + "\\Saved Games" + "\\Microsoft Games" + "\\Viva Pinata";
		GardenDirectory = SaveDirectory + "\\Saves";
	}
	else if (std::filesystem::exists(RootDicrectory + "/default.xex") && std::filesystem::exists(RootDicrectory + "/Beta/packages/1.pkg")) {
		//TIP
		PackagesDicrectory = RootDicrectory + "\\Beta\\packages";
		BundlesDicrectory = RootDicrectory + "\\Beta\\bundles";
		DebugPackDicrectory = RootDicrectory + "\\Beta\\packed";
		WadDicrectory = "";
		SaveDirectory = ""; // TIP uses Xenia's save directory which is not configurable from here
		GardenDirectory = "";
	}

	SaveSettings();
}

inline void ResetSettings() {
	RootDicrectory = "";
	PackagesDicrectory = "";
	BundlesDicrectory = "";
	DebugPackDicrectory = "";
	WadDicrectory = "";
	SaveSettings();
}

inline bool IsTIP() {
	return std::filesystem::exists(RootDicrectory + "/default.xex") && std::filesystem::exists(RootDicrectory + "/Beta/packages/1.pkg");
}


struct Vertex1 {
	Vector3 position; // 12 bytes
	Vector3 normal; // 12 bytes
	Vector2 texCoord; // 8 bytes
};

struct Object1 {
	std::vector<Vertex1> objectsVerts;
	std::vector<GLuint> objectsIndices;
	std::vector<unsigned char> rawVertBlock;
	std::vector<unsigned char> rawIndexBlock;
	int VertexSize;
};

inline std::string CurrentModelName = "";
inline std::vector<Object1> CurrentModel; // Current object being viewed in the viewport

namespace globals {
	inline VinceWindow window1(window_width, window_height, "SolarEngine");
	inline Camera* cam = nullptr;
}
