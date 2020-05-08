
#pragma once

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

class CameraInfoParser
{
public:
	CameraInfoParser() : ver(0), wafer(0), chipNumber(0)
	{
		memset(&lot[0], 0, sizeof(lot));
	}

	// Returns the starting memory address to read for the camera ID
	int StartAddress() { return 0x37a0; }
	// Returns the number of bytes to read from the CX3 to read the camera ID
	int NumBytesToRead() { return 8; }

	// Once you have read the camera info into a byte vector, call this to parse the information
	bool SetCameraInfo(const std::vector<byte> &mem)
	{
		bool success = mem.size() == NumBytesToRead();
		if (success)
		{
			ver = mem[0] & 3;
			lot[0] = (mem[0] >> 2) & 0x0f;
			lot[1] = ((mem[0] >> 6) & 3) | ((mem[1] & 3) << 2);
			lot[2] = ((mem[1] >> 2) & 0x0f);
			lot[3] = ((mem[1] >> 6) & 3) | ((mem[2] & 3) << 2);
			lot[4] = ((mem[2] >> 2) & 0x0f);
			lot[5] = ((mem[2] >> 6) & 3) | ((mem[3] & 3) << 2);
			lot[6] = ((mem[3] >> 2) & 0x0f);
			lot[7] = ((mem[3] >> 6) & 3) | ((mem[4] & 3) << 2);
			lot[8] = ((mem[4] >> 2) & 0x0f);
			wafer = ((mem[4] >> 6) & 3) | ((mem[5] & 7) << 2);
			chipNumber = (mem[5] >> 3) | (mem[6] << 5) | ((mem[7] & 1) << 13);
		}
		return success;
	}

	std::string ToString()
	{
		std::string output = IntToHex(ver, 1) + "-";
		for (int i = 0; i < 9; i++)
			output += IntToHex(lot[i], 1);
		output += "-" + IntToHex(wafer, 2);
		output += "-" + IntToHex(chipNumber, 5);
		return output;
	}

	int GetVersion() { return ver; }
	int GetWafer() { return wafer; }
	int GetChipNumber() { return chipNumber; }
	std::vector<int> GetLotInfo()
	{
		std::vector<int> vLots;
		vLots.resize(9);
		for (int i = 0; i < 9; i++)
		{
			vLots[i] = lot[i];
		}
		return vLots;
	}

private:

	std::string IntToHex(int value, int numChars)
	{
		std::stringstream hexstream;
		hexstream << std::setfill('0') << std::setw(numChars) << std::hex << value;
		return hexstream.str();
	}

	int ver;
	int lot[9];
	int wafer;
	int chipNumber;

};