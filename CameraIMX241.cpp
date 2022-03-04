#include "CameraIMX241.h"
#include <string>

// This define controls overwriting the top 5 lines if no meta information and newer firmware
//#define OVERWRITE_IF_NO_METAINFO

typedef struct
{
	uint8_t pixel1Upper8;
	uint8_t pixel2Upper8;
	uint8_t pixel3Upper8;
	uint8_t pixel4Upper8;
	uint8_t pixelsLower2;
} PackedPixelData;

typedef struct {
	uint64_t EyelockStamp;  // 0x4579656c6f636b00 Reads: 'E', 'y', 'e', 'l', 'o', 'c', 'k', 0x00
	uint8_t FirmwareMajor;
	uint8_t FirmwareMinor;
	uint8_t isHighAmbientLightModeOn;
	uint8_t isLeftIRLEDOn;
	uint8_t isRightIRLEDOn;
	uint8_t isTransitionFrame;
	uint16_t IntegrationTimeValue;
} FIRMINFO_1;

typedef struct {
	uint64_t EyelockStamp;  // 0x4579656c6f636b00 Reads: 'E', 'y', 'e', 'l', 'o', 'c', 'k', 0x00
	uint8_t EyelockStampVersion;
	uint16_t EyelockStampSize;
	uint8_t FirmwareMajor;
	uint8_t FirmwareMinor;
	uint8_t isHighAmbientLightModeOn;
	uint8_t isLeftIRLEDOn;
	uint8_t isRightIRLEDOn;
	uint8_t isTransitionFrame;
	uint8_t IntegrationTimeMin;
	uint16_t IntegrationTimeMax;
	uint16_t IntegrationTimeValue;
} FIRMINFO_2;

typedef struct {
	uint64_t EyelockStamp;  // 0x4579656c6f636b00 Reads: 'E', 'y', 'e', 'l', 'o', 'c', 'k', 0x00    
	uint8_t EyelockStampVersion; // 1    
	uint16_t EyelockStampSize; // 28    
	uint8_t FirmwareMajor; // 1    
	uint8_t FirmwareMinor; // 33 or 34?    
	uint8_t isHighAmbientLightModeOn;    
	uint8_t isLeftIRLEDOn;    
	uint8_t isRightIRLEDOn;    
	uint8_t isTransitionFrame;    
	uint8_t IntegrationTimeMin;    
	uint16_t IntegrationTimeMax;    
	uint16_t IntegrationTimeValue;    
	uint8_t VideoFormat;    
	uint8_t PackingMode;    
	uint16_t ImageOutputWidth;    
	uint16_t ImageOutputHeight;
} FIRMINFO_3;


typedef struct {
	uint64_t EyelockStamp;  // 0x4579656c6f636b00 Reads: 'E', 'y', 'e', 'l', 'o', 'c', 'k', 0x00
	uint8_t EyelockStampVersion;
	uint16_t EyelockStampSize;
	uint8_t FirmwareMajor;
	uint8_t FirmwareMinor;
	uint8_t isHighAmbientLightModeOn;
	uint8_t isLeftIRLEDOn;
	uint8_t isRightIRLEDOn;
	uint8_t isTransitionFrame;
	uint8_t IntegrationTimeMin;
	uint16_t IntegrationTimeMax;
	uint16_t IntegrationTimeValue;
	uint8_t PackingMode;
	uint16_t ImageOutputWidth;
	uint16_t ImageOutputHeight;
}FIRMINFO_4;

#define EYELOCK_STAMP_VERSION_2 1
#define EYELOCK_STAMP_SIZE_2	176 //in bits

#define EYELOCK_STAMP_VERSION_3 2
#define EYELOCK_STAMP_SIZE_3 28

#define EYELOCK_STAMP_VERSION_4 4
#define EYELOCK_STAMP_SIZE_4 sizeof(FIRMINFO_4)

enum PackingMode {

	RAW10_10_16 = 0x01,  // Raw 10/16 format that will pad the highest 6 bits with 0's. This was used with the legacy code. 1.32 and before
	RAW10_16_16 = 0x02  //  Completely packed 16 bit output. We are packing Raw 10 into 16 bits by packing 4 pixels into 5 bytes. https://community.cypress.com/docs/DOC-16964
};

const uint64_t EyelockStampValidation1 = 0x4579656c6f636b00;
CameraIMX241* CameraIMX241::m_This = nullptr;

CameraIMX241& CameraIMX241::Instance()
{
	if (nullptr == m_This)
	{
		m_This = new CameraIMX241();
	}
	return *m_This;
}

void IncQuad(unsigned char pixel, int& x, int& y, unsigned long long *quad, int width, int height)
{
	int quadIndex = 0;
	if (x >= (width / 2))
		quadIndex +=1;
	if (y >= (height / 2))
		quadIndex += 2;
	quad[quadIndex] += pixel;
	x++;
	if (x >= width)
	{
		x = 0;
		y++;
	}
}

/// @brief Process the first bytes of an image - verify the firmware data and process into CameraImageInfo
/// @param pSrc Pointer to image data
/// @param sourceImageLength Length of the image data
/// @param imageInfo Structure to be filled out of image data
/// @return success True if Firmware data present or we were able to figure out the data
bool CameraIMX241::ProcessCameraInfo(const unsigned char* pSrc, long sourceImageLength, CameraImageInfo& imageInfo)
{
	bool success = false;
	auto frameNumber = imageInfo.frameNumber;
	memset(&imageInfo, 0, sizeof(imageInfo));
	long raw10Size = 2592 * 1944 * 2;
	long packedSize = 2592 * 1944 * 5 / 4;
	imageInfo.MagicNum = CX241v1;
	imageInfo.version = CameraImageInfo_Version1;
	imageInfo.frameNumber = frameNumber;
	imageInfo.VideoFormat = CX3VideoFormat::RAW10; // default is old type
	// Get Shift value - use mutex to avoid locking for long time
	int shift = GetBitShift();
	// Scan header for meta information
	const FIRMINFO_1* pFirmwareInfo = reinterpret_cast<const FIRMINFO_1*>(pSrc);
	bool overwriteTopLinesNoMeta = false;
	if (EyelockStampValidation1 == pFirmwareInfo->EyelockStamp)
	{
		const FIRMINFO_2* pFirmware2Info = reinterpret_cast<const FIRMINFO_2*>(pSrc);
		const FIRMINFO_3* pFirmware3Info = reinterpret_cast<const FIRMINFO_3*>(pSrc);
		const FIRMINFO_4* pFirmware4Info = reinterpret_cast<const FIRMINFO_4*>(pSrc);
		if ((EYELOCK_STAMP_VERSION_4 == pFirmware4Info->EyelockStampVersion) &&
				(EYELOCK_STAMP_SIZE_4 == pFirmware4Info->EyelockStampSize))
		{
			imageInfo.frameHasMetaInformation = true;
			imageInfo.FirmwareMajor = pFirmware4Info->FirmwareMajor;
			imageInfo.FirmwareMinor = pFirmware4Info->FirmwareMinor;
			imageInfo.IntegrationTimeValue = pFirmware4Info->IntegrationTimeValue;
			imageInfo.isHighAmbientLightModeOn = pFirmware4Info->isHighAmbientLightModeOn;
			imageInfo.isLeftIRLEDOn = pFirmware4Info->isLeftIRLEDOn;
			imageInfo.isRightIRLEDOn = pFirmware4Info->isRightIRLEDOn;
			imageInfo.isTransitionFrame = pFirmware4Info->isTransitionFrame;
			imageInfo.IntegrationTimeMin = pFirmware4Info->IntegrationTimeMin;
			imageInfo.IntegrationTimeMax = pFirmware4Info->IntegrationTimeMax;
			if (RAW10_10_16 == pFirmware4Info->PackingMode)
			{
				imageInfo.VideoFormat = CX3VideoFormat::RAW10;
			}
			else if (RAW10_16_16 == pFirmware4Info->PackingMode)
			{
				imageInfo.VideoFormat = CX3VideoFormat::YUV422_8_2;
			}
			else
			{
				imageInfo.VideoFormat = CX3VideoFormat::YUV422_8_2;
			}
			imageInfo.height_ = pFirmware4Info->ImageOutputHeight;
			imageInfo.width_ = pFirmware4Info->ImageOutputWidth;
			success = true;
		}
		else if ((EYELOCK_STAMP_VERSION_3 == pFirmware3Info->EyelockStampVersion) &&
						 (EYELOCK_STAMP_SIZE_3 == pFirmware3Info->EyelockStampSize))
		{
			imageInfo.frameHasMetaInformation = true;
			imageInfo.FirmwareMajor = pFirmware3Info->FirmwareMajor;
			imageInfo.FirmwareMinor = pFirmware3Info->FirmwareMinor;
			imageInfo.IntegrationTimeValue = pFirmware3Info->IntegrationTimeValue;
			imageInfo.isHighAmbientLightModeOn = pFirmware3Info->isHighAmbientLightModeOn;
			imageInfo.isLeftIRLEDOn = pFirmware3Info->isLeftIRLEDOn;
			imageInfo.isRightIRLEDOn = pFirmware3Info->isRightIRLEDOn;
			imageInfo.isTransitionFrame = pFirmware3Info->isTransitionFrame;
			imageInfo.IntegrationTimeMin = pFirmware3Info->IntegrationTimeMin;
			imageInfo.IntegrationTimeMax = pFirmware3Info->IntegrationTimeMax;
			if (RAW10_10_16 == pFirmware3Info->PackingMode)
			{
				imageInfo.VideoFormat = CX3VideoFormat::RAW10;
			}
			else if (RAW10_16_16 == pFirmware3Info->PackingMode)
			{
				imageInfo.VideoFormat = CX3VideoFormat::YUV422_8_2;
			}
			else
			{
				imageInfo.VideoFormat = CX3VideoFormat::YUV422_8_2;
			}
			imageInfo.height_ = pFirmware3Info->ImageOutputHeight;
			imageInfo.width_ = pFirmware3Info->ImageOutputWidth;
			success = true;
		}
		else if ((EYELOCK_STAMP_VERSION_2 == pFirmware4Info->EyelockStampVersion) &&
						 (EYELOCK_STAMP_SIZE_2 == pFirmware4Info->EyelockStampSize))
		{
			imageInfo.frameHasMetaInformation = true;
			imageInfo.FirmwareMajor = pFirmware2Info->FirmwareMajor;
			imageInfo.FirmwareMinor = pFirmware2Info->FirmwareMinor;
			imageInfo.IntegrationTimeValue = pFirmware2Info->IntegrationTimeValue;
			imageInfo.isHighAmbientLightModeOn = pFirmware2Info->isHighAmbientLightModeOn;
			imageInfo.isLeftIRLEDOn = pFirmware2Info->isLeftIRLEDOn;
			imageInfo.isRightIRLEDOn = pFirmware2Info->isRightIRLEDOn;
			imageInfo.isTransitionFrame = pFirmware2Info->isTransitionFrame;
			imageInfo.IntegrationTimeMin = pFirmware2Info->IntegrationTimeMin;
			imageInfo.IntegrationTimeMax = pFirmware2Info->IntegrationTimeMax;
			imageInfo.VideoFormat = CX3VideoFormat::RAW10;
			imageInfo.height_ = 1944;
			imageInfo.width_ = 2592;
			success = true;
		}
		else // Is FIRMINFO_1
		{
			imageInfo.frameHasMetaInformation = true;
			imageInfo.FirmwareMajor = pFirmwareInfo->FirmwareMajor;
			imageInfo.FirmwareMinor = pFirmwareInfo->FirmwareMinor;
			imageInfo.isTransitionFrame = false;
			imageInfo.IntegrationTimeValue = 0;
			imageInfo.isHighAmbientLightModeOn = false;
			imageInfo.isLeftIRLEDOn = false;
			imageInfo.isRightIRLEDOn = false;
			imageInfo.isTransitionFrame = false;
			imageInfo.IntegrationTimeMin = 0;
			imageInfo.IntegrationTimeMax = 2000;
			imageInfo.VideoFormat = CX3VideoFormat::RAW10;
			imageInfo.height_ = 1944;
			imageInfo.width_ = 2592;
			success = true;
		}
	}
	else
	{
		imageInfo.frameHasMetaInformation = false;
		overwriteTopLinesNoMeta = true;
		imageInfo.height_ = 1944;
		imageInfo.width_ = 2592;
		if (sourceImageLength == raw10Size)
		{
			imageInfo.VideoFormat = CX3VideoFormat::RAW10;
		}
		else if (sourceImageLength == packedSize)
		{
			imageInfo.VideoFormat = CX3VideoFormat::YUV422_8_2;
		}
		else
		{
			imageInfo.isTransitionFrame = true;
		}
	}
	imageInfo.bit_shift_ = shift;
	imageInfo.pixel_count_ = imageInfo.height_ * imageInfo.width_;
	if (success)
	{
		minIntegrationTime = imageInfo.IntegrationTimeMin;
		maxIntegrationTime = imageInfo.IntegrationTimeMax;
	}
	return success;
}

/// @brief Process the 6/10MB 10 bit image into a 5MB 8 bit image. Build CameraImageInfo structure and store at front of image
/// @param pSrc Pointer to 10 bit data to read from
/// @param sourceLength Length of the input video data
/// @param pDest Pointer to 8 bit data to write to
/// @param destLength Length of destination data
/// @param imageInfo Write image data to this structure
/// @return true if successful (known image data)
bool CameraIMX241::ConvertVideoTo8Bit(const uint16_t* pSrc, long sourceLength, uint8_t* pDest, long destLength, CameraImageInfo& imageInfo)
{
	bool success = ProcessCameraInfo(reinterpret_cast<const unsigned char*>(pSrc), sourceLength, imageInfo);
	if (success) // && !imageInfo.isTransitionFrame)
	{
		long outputSize = imageInfo.width_ * imageInfo.height_;
		int xQuad = 0, yQuad = 0;
		char buffer[256];
		sprintf_s(buffer, sizeof(buffer),  "ConvertVideoTo8Bit Frame #%d, isTransitionFrame=%d, shift=%d", imageInfo.frameNumber, (int)imageInfo.isTransitionFrame, imageInfo.bit_shift_);
		if (YUV422_8_2 == imageInfo.VideoFormat)
		{
			int pixelBlockCount = sourceLength / 5; // each pixel block is 4 pixels (upper 8 bits) followed by 1 byte holding bottom 2 bits of each pixel in the group 
			const uint8_t* p8BitSrc = reinterpret_cast<const unsigned char*>(pSrc);
			uint8_t* pConvertedPixel = pDest;
			switch (imageInfo.bit_shift_) {
			case 0: // bottom 8 bits
				success = true;
				for (auto nPixel = 0; nPixel < pixelBlockCount; nPixel++) {
					const PackedPixelData* pPixelBlock = reinterpret_cast<const PackedPixelData*>(&p8BitSrc[nPixel * 5]);
					uint16_t pixel1Value = (pPixelBlock->pixel1Upper8 << 2) | (pPixelBlock->pixelsLower2 >> 6);
					uint16_t pixel2Value = (pPixelBlock->pixel2Upper8 << 2) | ((pPixelBlock->pixelsLower2 >> 4) & 0x03);
					uint16_t pixel3Value = (pPixelBlock->pixel3Upper8 << 2) | ((pPixelBlock->pixelsLower2 >> 2) & 0x03);
					uint16_t pixel4Value = (pPixelBlock->pixel4Upper8 << 2) | (pPixelBlock->pixelsLower2 & 0x03);
					uint8_t shifted = 0xff & pixel1Value;
					if (0 != (pixel1Value & 0x300))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					shifted = 0xff & pixel2Value;
					if (0 != (pixel2Value & 0x300))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					shifted = 0xff & pixel3Value;
					if (0 != (pixel3Value & 0x300))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					shifted = 0xff & pixel4Value;
					if (0 != (pixel4Value & 0x300))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
				}
				break;
			case 1: // middle 8 bits
				success = true;
				for (auto nPixel = 0; nPixel < pixelBlockCount; nPixel++) {
					const PackedPixelData* pPixelBlock = reinterpret_cast<const PackedPixelData*>(&p8BitSrc[nPixel * 5]);
					uint16_t pixel1Value = (pPixelBlock->pixel1Upper8 << 1) | (pPixelBlock->pixelsLower2 >> 7);
					uint16_t pixel2Value = (pPixelBlock->pixel2Upper8 << 1) | ((pPixelBlock->pixelsLower2 >> 5) & 0x01);
					uint16_t pixel3Value = (pPixelBlock->pixel3Upper8 << 1) | ((pPixelBlock->pixelsLower2 >> 3) & 0x01);
					uint16_t pixel4Value = (pPixelBlock->pixel4Upper8 << 1) | ((pPixelBlock->pixelsLower2 >> 1) & 0x01);
					uint8_t shifted = 0xff & pixel1Value;
					if (0 != (pixel1Value & 0x100))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					shifted = 0xff & pixel2Value;
					if (0 != (pixel2Value & 0x100))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					shifted = 0xff & pixel3Value;
					if (0 != (pixel3Value & 0x100))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					shifted = 0xff & pixel4Value;
					if (0 != (pixel4Value & 0x100))
					{
						shifted = 0xff; // make it white because bits above are on
						imageInfo.num_forced_white_++;
					}
					*pConvertedPixel++ = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
				}
				break;
			case 2: // top 8 bits
				success = true;
				for (auto nPixel = 0; nPixel < pixelBlockCount; nPixel++) {
					const PackedPixelData* pPixelBlock = reinterpret_cast<const PackedPixelData*>(&p8BitSrc[nPixel * 5]);
					*pConvertedPixel++ = pPixelBlock->pixel1Upper8;
					imageInfo.histogram_[pPixelBlock->pixel1Upper8]++;
					IncQuad(pPixelBlock->pixel1Upper8, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					*pConvertedPixel++ = pPixelBlock->pixel2Upper8;
					imageInfo.histogram_[pPixelBlock->pixel2Upper8]++;
					IncQuad(pPixelBlock->pixel2Upper8, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					*pConvertedPixel++ = pPixelBlock->pixel1Upper8;
					imageInfo.histogram_[pPixelBlock->pixel3Upper8]++;
					IncQuad(pPixelBlock->pixel3Upper8, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
					*pConvertedPixel++ = pPixelBlock->pixel1Upper8;
					imageInfo.histogram_[pPixelBlock->pixel4Upper8]++;
					IncQuad(pPixelBlock->pixel4Upper8, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
				}
				break;
			default:
				success = false;
				break;
			}
		}
		else if (RAW10 == imageInfo.VideoFormat)
		{
			int pixelCount = outputSize / 2; // the buffer holds 2 bytes per pixel, so we divide by 2 for the pixel count
			switch (imageInfo.bit_shift_) {
			case 0:
				success = true;
				for (auto i = 0; i < pixelCount; i++) {
					uint8_t shifted = 0xff & pSrc[i];
					if ((pSrc[i] & 0x300) != 0)
					{
						imageInfo.num_forced_white_++;
						shifted = 0xff;
					}
					pDest[i] = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
				}
				break;
			case 1:
				success = true;
				for (auto i = 0; i < pixelCount; i++) {
					uint8_t shifted = 0xff & (pSrc[i] >> 1);
					if ((pSrc[i] & 0x200) != 0)
					{
						imageInfo.num_forced_white_++;
						shifted = 0xff;
					}
					pDest[i] = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
				}
				break;
			case 2:
				success = true;
				for (auto i = 0; i < pixelCount; i++)
				{
					uint8_t shifted = 0xff & (pSrc[i] >> 2);
					pDest[i] = shifted;
					imageInfo.histogram_[shifted]++;
					IncQuad(shifted, xQuad, yQuad, imageInfo.quadrantLuminescence, imageInfo.width_, imageInfo.height_);
				}
				break;
			default:
				success = false;
				break;
			}
		}
#if defined(OVERWRITE_IF_NO_METAINFO)
		if (overwriteTopLinesNoMeta)
		{
			uint8_t color = 0xff;
			if (YUV422_8_2 == imageInfo.VideoFormat)
				color = 0xc0;
			for (auto i = 0; i < 5; i++)
			{
				memset(&pDest[i* imageInfo.width], color, imageInfo.width);
			}
		}
#endif
		if (success)
			memcpy(pDest, &imageInfo, sizeof(imageInfo)); // Write the image info over the first bytes of the image
	}
	else
	{
		success = false;
	}
	return success;
}

