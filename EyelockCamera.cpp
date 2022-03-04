#include "EyelockCamera.h"
#include <sstream>
#include <iostream>
#include <fstream>
#include <filesystem>

EyelockCamera::EyelockCamera()
{
	bit_shift_ = 0;
}

bool EyelockCamera::LoadPGM(const std::string& filename, unsigned char* pDest, int sizeOfDest, CameraImageInfo& imageInfo)
{
	bool success = false;
	static int frameNumber = 1;
	static bool leftLEDState = true;
	FILE* inFile = nullptr;
	errno_t err = fopen_s(&inFile, filename.c_str(), "rb");
	if (0 == err)
	{
		/*determine pgm image type (only type three can be used)*/
		int ch = getc(inFile);
		if (ch != 'P')
		{
			return false;
		}
		ch = getc(inFile);
		if (ch != '5')
		{
			return false;
		}
		while (getc(inFile) != '\n');             /* skip to end of line*/
		while (getc(inFile) == '#')              /* skip comment lines */
		{
			while (getc(inFile) != '\n');          /* skip to end of comment line */
		}
		fseek(inFile, -1, SEEK_CUR);             /* backup one character*/

		int maxVal = 0;
		fscanf_s(inFile, "%d", &imageInfo.width_);
		fscanf_s(inFile, "%d", &imageInfo.height_);
		fscanf_s(inFile, "%d", &maxVal);
		while (getc(inFile) != '\n');          /* skip to end line - pixel data follows*/

		int numPixels = imageInfo.width_ * imageInfo.height_;
		if (numPixels == sizeOfDest)
		{
			fread(pDest, 1, numPixels, inFile);
			{
				success = true;
				// We have an image - see if we have a leading HEADER with CameraFrameInfo
				CameraImageInfo* pCameraFrameInfo = reinterpret_cast<CameraImageInfo*>(pDest);
				if (CX241v1 == pCameraFrameInfo->MagicNum)
				{
					if (CameraImageInfo_Version1 == pCameraFrameInfo->version)
					{
						imageInfo = *reinterpret_cast<CameraImageInfo1*>(pDest); // We have the camera info, so copy it!
						success = true;
					}
					else
					{
						success = false;
					}
				}
				else
				{
					std::filesystem::path path(filename);
					std::string fName = path.filename().string();
					// No meta-data in the PGM, must fake
					memset(&imageInfo, 0, sizeof(imageInfo));
					imageInfo.MagicNum = CX241v1;
					imageInfo.version = CameraImageInfo_Version1;
					imageInfo.frameHasMetaInformation = true; // We are lying!
					if (!fName.empty())
					{
						if (isdigit(fName[0]))
							imageInfo.frameNumber = std::stoi(fName);
						else
							imageInfo.frameNumber = frameNumber++;
					}
					else
						imageInfo.frameNumber = frameNumber++;
					imageInfo.pixel_count_ = imageInfo.width_ * imageInfo.height_;
					imageInfo.bit_shift_ = GetBitShift();
					imageInfo.num_forced_white_ = 0;
					imageInfo.FirmwareMajor = 0;
					imageInfo.FirmwareMinor = 0;
					imageInfo.isHighAmbientLightModeOn = true;
					imageInfo.isLeftIRLEDOn = leftLEDState;
					imageInfo.isRightIRLEDOn = !leftLEDState;
					imageInfo.isTransitionFrame = false;
					imageInfo.IntegrationTimeValue = 1240;
					imageInfo.IntegrationTimeMin = 50;
					imageInfo.IntegrationTimeMax = 2478;
					leftLEDState = !leftLEDState;
				}
			}
		}
		fclose(inFile);
	}
	return success;
}

std::string GetLine(std::ifstream& inFile)
{
	std::string line;
	std::streampos oldpos;
	bool commentLine = true;
	while (commentLine) {
		// store the file position so that the line can be accessed later
		// for integers if it is not a comment line
		oldpos = inFile.tellg();
		getline(inFile, line);
		if (line[0] == '#') {
			// found a comment
			//cout << line << endl;
		}
		else
			commentLine = false;
	}
	return line;
}

bool EyelockCamera::LoadPGM(const std::string& filename, std::vector<unsigned char>& pixels, CameraImageInfo& imageInfo)
{
	bool success = false;
	static int frameNumber = 1;
	static bool leftLEDState = true;
	std::ifstream inFile(filename, std::ios::binary);
	if (inFile.is_open())
	{
		std::stringstream ss;
		std::string line = "";
		std::getline(inFile, line);
		if (0 == line.compare("P5"))
		{
			std::streampos curPos = inFile.tellg();
			std::getline(inFile, line);
			if (!line.empty() && (line[0] == '#'))
			{
				// This line is a comment so we don't care about it
			}
			else
			{
				// was not a comment, so move pointer back and reread as width/height
				inFile.seekg(curPos);
			}
			int maxIntensity, width, height;
			// swallow - this should be a comment line
			std::string line = GetLine(inFile);
			ss << line; // read the 3rd line, width and height
			ss >> width >> height;
			line = GetLine(inFile);
			ss << line;
			ss >> maxIntensity; // don't really need - assume is 255
			int numPixels = width * height;
			pixels.resize(numPixels);
			std::streampos startPix = inFile.tellg();
			inFile.read(reinterpret_cast<char*>(&pixels[0]), numPixels-1);
			std::streampos endPix = inFile.tellg();
			{
				success = true;
				// We have an image - see if we have a leading HEADER with CameraFrameInfo
				CameraImageInfo* pCameraFrameInfo = reinterpret_cast<CameraImageInfo*>(&pixels[0]);
				if (CX241v1 == pCameraFrameInfo->MagicNum)
				{
					if (CameraImageInfo_Version1 == pCameraFrameInfo->version)
					{
						imageInfo = *reinterpret_cast<CameraImageInfo1*>(&pixels[0]); // We have the camera info, so copy it!
						success = true;
					}
					else
					{
						success = false;
					}
				}
				else
				{
					std::filesystem::path path(filename);
					std::string fName = path.filename().string();
					// No meta-data in the PGM, must fake
					memset(&imageInfo, 0, sizeof(imageInfo));
					imageInfo.MagicNum = CX241v1;
					imageInfo.version = CameraImageInfo_Version1;
					imageInfo.frameHasMetaInformation = true; // We are lying!
					if (!fName.empty())
					{
						if (isdigit(fName[0]))
							imageInfo.frameNumber = std::stoi(fName);
						else
							imageInfo.frameNumber = frameNumber++;
					}
					else
						imageInfo.frameNumber = frameNumber++;
					imageInfo.width_ = width;
					imageInfo.height_ = height;
					imageInfo.pixel_count_ = width * height;
					imageInfo.bit_shift_ = GetBitShift();
					imageInfo.num_forced_white_ = 0;
					imageInfo.FirmwareMajor = 0;
					imageInfo.FirmwareMinor = 0;
					imageInfo.isHighAmbientLightModeOn = true;
					imageInfo.isLeftIRLEDOn = leftLEDState;
					imageInfo.isRightIRLEDOn = !leftLEDState;
					imageInfo.isTransitionFrame = false;
					imageInfo.IntegrationTimeValue = 1240;
					imageInfo.IntegrationTimeMin = 50;
					imageInfo.IntegrationTimeMax = 2478;
					leftLEDState = !leftLEDState;
					success = true;
				}
			}
		}
		inFile.close();
	}
	return success;
}

/// <summary>
/// Write an image to a PGM file with CameraFrameInfo replacing the first 'x' bytes of the image data with the CameraImageInfo
/// </summary>
/// <param name="filename">name/path of file</param>
/// <param name="pImage">pointer to pixel data</param>
/// <param name="sizeOfImage">how many bytes in image</param>
/// <param name="imageInfo">CameraFrameInfo structure</param>
/// <returns></returns>
bool EyelockCamera::SavePGM(const std::string &filename, unsigned char* pImage, int sizeOfImage, CameraImageInfo& imageInfo)
{
	bool success = false;
	int imgSize = imageInfo.width_ * imageInfo.height_;

	std::ofstream outFile;
	outFile.open(filename.c_str(), std::ios::binary | std::ios::out);
	if (outFile) {
		outFile << "P5" << std::endl;
		if (!outFile.fail())
		{
			outFile << imageInfo.width_ << " " << imageInfo.height_ << std::endl;
		}
		if (!outFile.fail())
		{
			outFile << "255" << std::endl;
		}
		if (!outFile.fail())
		{
			int sizeImageInfo = sizeof(imageInfo);
			if (!outFile.fail())
			{
				outFile.write(reinterpret_cast<const char*>(&imageInfo), sizeImageInfo); // write CameraFrameInfo first
				if (!outFile.fail())
				{
					outFile.write(reinterpret_cast<const char*>(pImage) + sizeImageInfo, sizeOfImage - sizeImageInfo); // write image offset by the frame meta data so lead is dropped
				}
			}
		}
		if (!outFile.fail())
		{
			success = true;
		}
	}
	return success;
}
