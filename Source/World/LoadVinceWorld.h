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
		Log("Loading Vince World Models from " + Path + "modelsAndAnims.hot", EType::PURPLE);

		HotFile Models = Hot::ReadFile(modelsAndAnims.Files[1]);

		for (const HotFileInfo& file : Models.Files) {
			if (file.Name.find("gator") != std::string::npos) {
				ObjectMeshData meshData;
				meshData.Name = file.Name;
				//Log("Found Gator Model: " + meshData.Name, EType::GREEN);
				meshData.gatorMesh = file.Data;
				models.push_back(meshData);
			}
		}
		Log("Number of Models in Zone modelsAndAnims: " + std::to_string(models.size()), EType::PURPLE);

		//Now go a folder up and get /common/modelsAndAnims.hot
		std::string NewPath = Path.substr(0, Path.find_last_of("/\\"));
		std::string CommonPath = NewPath.substr(0, NewPath.find_last_of("/\\") + 1) + "common/modelsAndAnims.hot";
		Log(CommonPath, EType::PURPLE);
		//check to see if common/modelsAndAnims.hot exists
		if(!std::filesystem::exists(CommonPath)) {
			Log("Common modelsAndAnims.hot not found at " + CommonPath, EType::Error);
			return models; // Return empty if not found
		}

		HotFile commonModelsAndAnims = Hot::ReadFile(CommonPath);
		Log("Loading Vince World Common Models from " + CommonPath, EType::PURPLE);

		HotFile CommonModels = Hot::ReadFile(commonModelsAndAnims.Files[1]);

		for (const HotFileInfo& file : CommonModels.Files) {
			if (file.Name.find("gator") != std::string::npos) {
				ObjectMeshData meshData;
				meshData.Name = file.Name;
				//Log("Found Gator Model: " + meshData.Name, EType::GREEN);
				meshData.gatorMesh = file.Data;
				models.push_back(meshData);
			}
		}
		Log("Number of Models in Common modelsAndAnims: " + std::to_string(models.size()), EType::PURPLE);

		return models;
	}

	// Loads the Vince world from a file and returns a pointer to the World object
	World* LoadWorld(const std::string& filename, World& world) {
		//check to see if file name is "world.hot"
		if (filename.find("world.hot") != std::string::npos) {
			HotFile World = Hot::ReadFile(filename);
			Log("Loading Vince World from " + filename, EType::PURPLE);

			//Log all names
			for (HotFileInfo& fileInfo : World.Files) {
				if (fileInfo.Name == "data.hot") {
					HotFile Data = Hot::ReadFile(fileInfo);
					//Log all names in data.hot
					for (HotFileInfo& dataInfo : Data.Files) {
						if (dataInfo.Name.find("index") != std::string::npos) {
							Log("Retrieved Level Index File: " + dataInfo.Name, EType::Success);
							std::vector<ObjectEntry> Objects = LoadObjectEntriesFromIndex(dataInfo.Data);
							Log("Number of Objects: " + std::to_string(Objects.size()), EType::PURPLE);

							std::vector<ObjectMeshData> models = GetAllModelsInVinceWorld(filename);

							for (const ObjectEntry& entry : Objects) {
								std::string DebugString = "Object: " + entry.Name + ", Zone: " + std::to_string(entry.Zone) +
									", Location: (" + std::to_string(entry.Location.x) + ", " +
									std::to_string(entry.Location.y) + ", " +
									std::to_string(entry.Location.z) + "), Rotation: (" +
									std::to_string(entry.rotation.x) + ", " +
									std::to_string(entry.rotation.y) + ", " +
									std::to_string(entry.rotation.z) + ", " +
									std::to_string(entry.rotation.w) + ")";
								Log(DebugString, EType::BLUE);

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
											Log("Found matching model: " + model.Name, EType::GREEN);
											// Create a mesh component and add it to the actor
											auto gatorMeshComponent = std::make_shared<GatorMeshComponent>(model.gatorMesh);
											auto meshActor = std::make_unique<Actor>();
											meshActor->AddComponent(gatorMeshComponent);
											meshActor->AddWorldPosition(entry.Location);
											//meshActor->SetRotation(entry.rotation);
											glm::quat q = glm::quat(entry.rotation); // Euler to quaternion
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
