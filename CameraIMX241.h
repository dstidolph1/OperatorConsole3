#ifndef CameraIMX241_h
#define CameraIMX241_h

#include <string>
#include <mutex>
#include "EyelockCamera.h"

class CameraIMX241 : public EyelockCamera
{
protected:
	CameraIMX241() :
		minIntegrationTime(DEFAULT_MIN_INTEGRATION_TIME),
		maxIntegrationTime(DEFAULT_MAX_INTEGRATION_TIME)
	{
	}

public:
	static CameraIMX241& Instance();

	bool ConvertVideoTo8Bit(std::vector<uint8_t> &src, std::vector<uint8_t> &dest, CameraImageInfo& imageInfo);

	int GetMinIntegrationTime() { return minIntegrationTime; }
	int GetMaxIntegrationTime() { return maxIntegrationTime; }

protected:
	bool ProcessCameraInfo(const unsigned char* pSrc, long imageLength, CameraImageInfo& imageInfo);

	static CameraIMX241* m_This;
	int minIntegrationTime;
	int maxIntegrationTime;
};

#endif