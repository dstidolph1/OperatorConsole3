#ifndef CameraIMX241_h
#define CameraIMX241_h

#include <string>
#include <mutex>
#include "EyelockCamera.h"

class CameraIMX241 : public EyelockCamera
{
protected:
	CameraIMX241();

public:
	static CameraIMX241& Instance();

	bool ConvertVideoTo8Bit(std::vector<uint8_t> &src, std::vector<uint8_t> &dest, CameraImageInfo& imageInfo);

	int GetMinIntegrationTime() { return minIntegrationTime; }
	int GetMaxIntegrationTime() { return maxIntegrationTime; }

protected:
	bool ProcessCameraInfo(const unsigned char* pSrc, long imageLength, CameraImageInfo& imageInfo);

	uint8_t ConvertByte(uint8_t pixelValue, int x, int y)
	{
		// RGRG
		// GBGB
		if (y & 1)
		{
			// GBGB
			if (x & 1)
				return m_blue[pixelValue];
			else
				return m_green[pixelValue];
		}
		else
		{
			if (x & 1)
				return m_green[pixelValue];
			else
				return m_red[pixelValue];
		}
	}

	static CameraIMX241* m_This;
	int minIntegrationTime;
	int maxIntegrationTime;
	uint8_t m_red[256];
	uint8_t m_green[256];
	uint8_t m_blue[256];
};

#endif