#pragma	once
#include "Utils/ini.h"

/// The FileDatabase provides the functionality to take a checksum and return a custom user-friendly name for it. 
/// The Database is stored in an INI file for easy editing/version control.

struct FileRenameEntry
{
	std::string checksum; // The checksum of the file
	std::string name;     // The user-friendly name associated with the checksum
};

namespace FileDatabase
{

	/// Get all entries in the database.
	inline std::vector<FileRenameEntry> Entries;

	/// Load the database from an INI file.
	static void Load(const std::string& filename) {
		Entries.clear();
		mINI::INIFile file(filename);
		mINI::INIStructure ini;
		file.read(ini);

		// Iterate through the sections to find "Files"
		auto section = ini["Files"];
		for (const auto& pair : section) {
			// pair.first = checksum, pair.second = name
			Entries.push_back({ pair.first, pair.second });
		}
	}

	/// Save the database to an INI file.
	static void Save(const std::string& filename) {
		mINI::INIStructure ini;
		// Store all entries in a section called "Files"
		for (const auto& entry : Entries) {
			ini["Files"][entry.checksum] = entry.name;
		}
		mINI::INIFile file(filename);
		file.write(ini);
	}

	/// Add or update an entry in the database.
	static void AddEntry(const std::string& checksum, const std::string& name) {
		Entries.push_back({ checksum, name });
		Save("FileNameDatabase.ini"); // Save immediately after adding an entry so if the program crashes, we don't lose data
	}

	/// Remove an entry from the database by checksum.
	static void RemoveEntry(const std::string& checksum) {
		for (auto it = Entries.begin(); it != Entries.end(); ++it) {
			if (it->checksum == checksum) {
				Entries.erase(it);
				Save("FileNameDatabase.ini"); // Save immediately after removing an entry
				break;
			}
		}
	}

	/// Get the user-friendly name for a given checksum. Returns empty string if not found.
	static std::string GetName(const std::string& checksum) {
		for (const auto& entry : Entries) {
			if (entry.checksum == checksum) {
				return entry.name;
			}
		}
		return "";
	}

	/// Clear all entries in the database.
	static void Clear() {
		Entries.clear();
	}

}
