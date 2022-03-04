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

	bool ConvertVideoTo8Bit(const uint16_t* pSrc, long sourceLength, uint8_t* pDest, long destLength, CameraImageInfo& imageInfo);

	int GetMinIntegrationTime() { return minIntegrationTime; }
	int GetMaxIntegrationTime() { return maxIntegrationTime; }

protected:
	bool ProcessCameraInfo(const unsigned char* pSrc, long imageLength, CameraImageInfo& imageInfo);

	static CameraIMX241* m_This;
	int minIntegrationTime;
	int maxIntegrationTime;
};

#endif