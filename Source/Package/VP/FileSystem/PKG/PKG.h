#include "../File.h"
#include "../CAFF/CAFF.h"

class Package : public File {
public:
	std::string Path;
	struct PKGHeader{
		int Version = 0;
		int CAFFCount = 0;
	};

	struct CAFFEntry {
		int Unk = 0;
		int Offset = 0;
		int Size = 0;
	};

	PKGHeader Header;
	std::vector<CAFFEntry> CAFFEntries;
	std::vector<CAFF> CAFFs;

	void IsBigEndian() {
		IsBigEndianFile = Header.Version > 2;
	}
	Package() = default;
	Package(const std::vector<unsigned char>& rawFile, std::string& path) : File(rawFile), Path(path) {
		LoadFile(rawFile);
	};

	void LoadFile(const std::vector<unsigned char>& rawFile) override {
		std::cout << "Loading PKG: " << Path << std::endl;
		//append rawFile and add 50000 bytes of padding to avoid out of bounds reads
		std::vector<unsigned char> paddedFile = rawFile;
		paddedFile.resize(rawFile.size() + 50000, 0);
		
		memcpy(&Header, paddedFile.data(), sizeof(PKGHeader));

		IsBigEndian();

		if (IsBigEndianFile) {
			Header.Version = _byteswap_ulong(Header.Version);
			Header.CAFFCount = _byteswap_ulong(Header.CAFFCount);
		}

		CAFFEntries.resize(Header.CAFFCount);
		memcpy(CAFFEntries.data(), paddedFile.data() + sizeof(PKGHeader), sizeof(CAFFEntry) * Header.CAFFCount);
		int index = 0;
		for (auto& entry : CAFFEntries) {
			index++;
			if (IsBigEndianFile) {
				entry.Unk = _byteswap_ulong(entry.Unk);
				entry.Offset = _byteswap_ulong(entry.Offset);
				entry.Size = _byteswap_ulong(entry.Size);
			}
			std::vector<unsigned char> caffData;
			caffData.resize(entry.Size);
			memcpy(caffData.data(), paddedFile.data() + entry.Offset, entry.Size);
			//add padding to caffData to avoid out of bounds reads
			caffData.resize(entry.Size + 50000, 0);
			//std::cout << "Loading CAFF: " << index << " at offset " << entry.Offset << " with size " << entry.Size << std::endl;
			CAFF caff = CAFF(caffData, IsBigEndianFile, index - 1);
			CAFFs.push_back(std::move(caff));
		}
	}

	void Destroy() override {
		File::Destroy();
		for(CAFF& caff : CAFFs) {
			caff.Destroy();
		}
		CAFFs.clear();
		CAFFEntries.clear();
	}
};