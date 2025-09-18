#pragma once
#include <string>
#include <vector>
#include <imgui.h>
#include "../../Shaders/Texture.h"

enum class ContentType {
	None,
	Folder,
	Mesh
};

struct ContentThumbnail {
	ContentType Type = ContentType::None;
	std::unique_ptr<Texture> ContentTexture;
};

class ContentWindow {
public:
	ContentWindow() = default;
	~ContentWindow() = default;
	void RenderContentWindow();

	ContentThumbnail* GetTexture(ContentType Type) {
		for(ContentThumbnail& Thumbnail : ContentTextures) {
			if(Thumbnail.Type == Type) {
				return &Thumbnail;
			}
		}
		switch(Type) {
			case ContentType::Mesh:
				ContentTextures.push_back(ContentThumbnail{ContentType::Mesh, std::make_unique<Texture>("Assets/Icons/Mesh.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE)});
				return &ContentTextures.back();
			case ContentType::Folder:
				ContentTextures.push_back(ContentThumbnail{ContentType::Folder, std::make_unique<Texture>("Assets/Icons/Folder.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE)});
				return &ContentTextures.back();
			default:
				ContentTextures.push_back(ContentThumbnail{ ContentType::Mesh, std::make_unique<Texture>("Assets/Icons/Null.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE) });
				return &ContentTextures.back();
		}
	}

	void DrawItem(const char ID, std::string Name) {
		ImGui::SameLine();
		ImGui::BeginChild(ID, ImVec2(ContentWindowSize.x/20, (ContentWindowSize.x / 20) + 14), true);
		//ImGui::Text("%s", Name.c_str());
		//center image in child window
		ImGui::SetCursorPos(ImVec2((ContentWindowSize.x / 20 - ContentWindowSize.x / 30) / 2, (ContentWindowSize.x / 20 - ContentWindowSize.x / 30) / 2));
		ImGui::Image(GetTexture(ContentType::Mesh)->ContentTexture->ID, ImVec2(ContentWindowSize.x / 30, ContentWindowSize.x / 30));
		//center text
		ImGui::Text("%s", "Model");
		ImGui::EndChild();
	}

	ImVec2 ContentWindowSize;
	std::vector<ContentThumbnail> ContentTextures;
};
