#pragma once

#include <string>
#include <vector>
#include "zlib.h"
#include <iostream>

#include "../../../Utils/ZLibHelpers.h"

namespace Zlib {

	//calculates the checksum of a CAFF header (Where the checksum bytes are nulled)
	//CREDIT TO MOJOBOJO FOR THIS AWESOME, WORKING HEADER CHECKSUM RECALCULATOR :D 
	//Copied from https://github.com/weighta/Mumbos-Motors
	inline static uint32_t CAFF_checksum(std::vector<unsigned char> CAFFHeader) {
		uint32_t r11 = 0;

		for (size_t i = 0; i < CAFFHeader.size(); i++) {
			uint32_t r8 = CAFFHeader[i];
			uint32_t r10 = r11 << 4;

			if ((r8 & 0x80) > 0) {
				r11 = 0xFFFFFF80 | r8;
			}
			else {
				r11 = r8;
			}

			r11 = r11 + r10;
			r10 = r11 & 0xF0000000;

			if (r10 != 0) {
				r8 = r10 >> 24;
				r10 = r8 | r10;
				r11 = r10 ^ r11;
			}
		}

		return r11;
	}

}

