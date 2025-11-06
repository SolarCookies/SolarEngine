#pragma once
#include "../../Page.h"

#include "../PackageManager/FileBrowser/DebugPack_FileBrowser.h"

class DebugPackManager : public Page
{
public:
	DebugPack_FileBrowser fileBrowser;
	void render(GUI& gui) override;
};