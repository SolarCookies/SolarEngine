#pragma once
#include "World.h"
#include "../Actors/Actor.h"
#include "../Components/Meshes/GatorMeshComponent.h"
#include "../Package/Hot.h"
#include "../Windows/Log.hpp"
#include <sstream>
#include <regex>

struct ObjectEntry
{
	std::string Name;
	int Zone;
	glm::vec3 Location;
	glm::vec4 rotation;
};

struct ObjectMeshData {
	std::string Name;
	std::vector<unsigned char> gatorMesh;
};;

inline static void ExportUnsignedCharVectorToFile(const std::vector<unsigned char>& data, const std::string& filename) {
	std::ofstream file(filename, std::ios::binary);
	if (file.is_open()) {
		file.write(reinterpret_cast<const char*>(data.data()), data.size());
		file.close();
	} else {
		Log("Failed to open file for writing: " + filename, EType::Error);
	}
}

namespace Vince
{
	std::vector<ObjectEntry> LoadObjectEntriesFromIndex(const std::vector<unsigned char>& IndexFile) {
		std::vector<ObjectEntry> entries;
		std::string content(IndexFile.begin(), IndexFile.end());
		std::istringstream stream(content);
		std::string line;
		bool inObjectsSection = false;
		bool inObjectBlock = false;
		std::regex objectHeaderRegex(R"(\[([^\]]+)\])");
		std::string currentObjectName;
		ObjectEntry currentEntry;

		while (std::getline(stream, line)) {
			// Trim whitespace
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			line.erase(line.find_last_not_of(" \t\r\n") + 1);

			if (line == "[Objects]") {
				inObjectsSection = true;
				continue;
			}
			if (inObjectsSection && line == "}") {
				if (inObjectBlock) {
					// End of current object block
					entries.push_back(currentEntry);
					inObjectBlock = false;
					currentObjectName.clear();
					continue;
				}
				else {
					// End of [Objects] section
					break;
				}
			}
			if (!inObjectsSection) continue;

			std::smatch match;
			if (std::regex_match(line, match, objectHeaderRegex)) {
				currentObjectName = match[1];
				currentEntry = ObjectEntry{};
				currentEntry.Name = currentObjectName;
				inObjectBlock = false; // Wait for '{'
				continue;
			}
			if (line == "{") {
				inObjectBlock = true;
				continue;
			}
			if (!inObjectBlock) continue;

			if (line.find("Zone =") == 0) {
				int zone;
				sscanf(line.c_str(), "Zone = %d;", &zone);
				currentEntry.Zone = zone;
			}
			else if (line.find("Location =") == 0) {
				float x, y, z;
				sscanf(line.c_str(), "Location = %f %f %f;", &x, &y, &z);
				currentEntry.Location = glm::vec3(x, y, z);
			}
			else if (line.find("Rotation =") == 0) {
				float x, y, z, w;
				sscanf(line.c_str(), "Rotation = %f %f %f %f;", &x, &y, &z, &w);
				currentEntry.rotation = glm::vec4(x, y, z, w);
			}
		}
		return entries;
	}

	//Get all models in the Vince world (modelsAndAnims.hot)
	std::vector<ObjectMeshData> GetAllModelsInVinceWorld(const std::string& filename) {
		std::vector<ObjectMeshData> models;
		std::string Path = filename.substr(0, filename.find_last_of("/\\") + 1);
		HotFile modelsAndAnims = Hot::ReadFile(Path + "modelsAndAnims.hot");
		HotFile Models = Hot::ReadFile(modelsAndAnims.Files[1]);
		HotFile Anims = Hot::ReadFile(modelsAndAnims.Files[0]);

		for (const HotFileInfo& file : Models.Files) {
			if (file.Name.find("gator") != std::string::npos) {
				ObjectMeshData meshData;
				meshData.Name = file.Name;
				//Log("Found Gator Model: " + meshData.Name, EType::GREEN);
				meshData.gatorMesh = file.Data;
				models.push_back(meshData);
			}
		}

		//Now go a folder up and get /common/modelsAndAnims.hot
		std::string NewPath = Path.substr(0, Path.find_last_of("/\\"));
		std::string CommonPath = NewPath.substr(0, NewPath.find_last_of("/\\") + 1) + "common/";

		//check to see if common/modelsAndAnims.hot exists
		if (!std::filesystem::exists(CommonPath + "modelsAndAnims.hot")) {
			Log("Common modelsAndAnims.hot not found at " + CommonPath, EType::Warning);
			return models;
		}

		HotFile commonModelsAndAnims = Hot::ReadFile(CommonPath + "modelsAndAnims.hot");

		HotFile CommonModels = Hot::ReadFile(commonModelsAndAnims.Files[1]);
		HotFile CommonAnims = Hot::ReadFile(commonModelsAndAnims.Files[0]);

		for (const HotFileInfo& file : CommonModels.Files) {
			if (file.Name.find("gator") != std::string::npos) {
				ObjectMeshData meshData;
				meshData.Name = file.Name;
				meshData.gatorMesh = file.Data;
				models.push_back(meshData);
			}
		}

		return models;

		//Export all textures to Test/Textures folder (Debugging purposes)
		HotFile CommonTextures = Hot::ReadFile(CommonPath + "textures.hot");
		for(const HotFileInfo& file : CommonTextures.Files) {
			ExportUnsignedCharVectorToFile(file.Data, "Test/Textures/" + file.Name);
		}

		HotFile NormalTextures = Hot::ReadFile(NewPath + "/textures.hot");
		for(const HotFileInfo& file : NormalTextures.Files) {
			ExportUnsignedCharVectorToFile(file.Data, "Test/Textures/" + file.Name);
		}

		return models;
	}

	// Loads the Vince world from a file and returns a pointer to the World object
	World* LoadWorld(const std::string& filename, World& world) {
		//check to see if file name is "world.hot"
		if (filename.find("world.hot") != std::string::npos) {
			HotFile World = Hot::ReadFile(filename);

			for (HotFileInfo& fileInfo : World.Files) {
				if (fileInfo.Name == "data.hot") {
					HotFile Data = Hot::ReadFile(fileInfo);
					//Log all names in data.hot
					for (HotFileInfo& dataInfo : Data.Files) {
						ExportUnsignedCharVectorToFile(dataInfo.Data, "Test/World/Data/" + dataInfo.Name);
						if (dataInfo.Name.find("index") != std::string::npos) {
							std::vector<ObjectEntry> Objects = LoadObjectEntriesFromIndex(dataInfo.Data);
							std::vector<ObjectMeshData> models = GetAllModelsInVinceWorld(filename);

							for (const ObjectEntry& entry : Objects) {
								for (const ObjectMeshData& model : models) {
									//Log("Checking model: " + model.Name, EType::GREEN);
									std::string modelName = model.Name.substr(0, model.Name.find_last_of('.'));
									std::string lowercaseEntryName = entry.Name;
									std::transform(lowercaseEntryName.begin(), lowercaseEntryName.end(), lowercaseEntryName.begin(),
										[](unsigned char c) { return std::tolower(c); });

									std::string lowercaseModelName = modelName;
									std::transform(lowercaseModelName.begin(), lowercaseModelName.end(), lowercaseModelName.begin(),
										[](unsigned char c) { return std::tolower(c); });

									size_t pos = lowercaseEntryName.find(lowercaseModelName);
									if (pos == 0) { // Only match at the start
										// Check if the next character is a digit or end of string
										size_t nextChar = lowercaseModelName.length();
										if (lowercaseEntryName.length() == nextChar || std::isdigit(lowercaseEntryName[nextChar])) {
											if (model.Name.find("gator") == std::string::npos) {
												Log("Skipping model: " + model.Name + " (not a gator model)", EType::Warning);
												continue;
											}
											// Create a mesh component and add it to the actor
											auto gatorMeshComponent = std::make_shared<GatorMeshComponent>(model.gatorMesh);
											auto meshActor = std::make_unique<Actor>();
											meshActor->AddComponent(gatorMeshComponent);
											meshActor->AddWorldPosition(entry.Location);
											//meshActor->SetRotation(entry.rotation);
											glm::quat q = glm::quat(entry.rotation.w, entry.rotation.x, entry.rotation.y, entry.rotation.z); // Convert vec4 to quaternion
											glm::vec3 euler = glm::eulerAngles(q); // Quaternion to Euler
											meshActor->SetWorldRotation(euler);
											world.AddActor(std::move(meshActor));
										}
									}
								}
							}
						}
					}
				}
			}
		}
	} // namespace VinceWorld
}
