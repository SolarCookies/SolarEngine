#pragma once
#include <string>
#include <vector>
#include <memory>

#include "../../../../FileSystem/DebugPack/DebugPack.h"

class DebugPack_FileBrowser
{
public:
	std::string PackDirectory = "";
	std::unique_ptr<DebugPack> DPack;

	DebugPack_FileBrowser() {
		InitRFB();
	};

	~DebugPack_FileBrowser() {
	};


	std::string CurrentCAFF = "";
	std::string CurrentAid = "";

	bool ShowImageWindow = false;

	bool UpdateImage = false;
	bool ShowRaw = false;

	char Searchbuf[255]{};
	std::string SearchTerm = "";


	bool FoundFiles = false;

	std::vector<std::string> files;
	std::vector<std::string> FileNames;

	void InitRFB();
	
	void RenderFileBrowser();

	void RenderTexture();
};