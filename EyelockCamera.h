#ifndef EYELOCK_CAMERA_H
#define EYELOCK_CAMERA_H

#include <string>
#include <mutex>
#include <vector>

const int DEFAULT_MIN_INTEGRATION_TIME = 50;
const int DEFAULT_MAX_INTEGRATION_TIME = 2478;

enum CX3VideoFormat {
	RAW10 = 0x01,		/**< RAW10 Data Type. CSI-2 Data Type 0x2B
												16 Bit Output = 6'b0,RAW[9:0]        */
	YUV422_8_2 = 0x26	/**< YUV422 8-Bit Data type. CSI-2 Type 0x1E.
													24 Bit Output = 8'b0,P[15:0]
													16 Bit Output = P[15:0]
													Data Order: {Y1,U1},{Y2,V1},{Y3,U3},{Y4,V3}....  */
};

const unsigned long long CX241v1 = 0x4358323431763100; // Reads : 'C', 'X', '2', '4', '1', 'v', '1', 0x00
#define CameraImageInfo_Version1 1

/// @brief This is the first version of this structure.
/// Tracks the information gathered from the frame.  With new
/// camera's and new firmware we will have new versions.  Keep
/// these structures and create new ones and adapt the save/load
/// code to match
struct CameraImageInfo1
{
	unsigned long long MagicNum = CX241v1;
	int version = CameraImageInfo_Version1;
	bool frameHasMetaInformation = false;
	int width_;
	int height_;
	int pixel_count_;
	long histogram_[256];
	int bit_shift_;
	int num_forced_white_;
	uint8_t FirmwareMajor;
	uint8_t FirmwareMinor;
	uint8_t isHighAmbientLightModeOn;
	uint8_t isLeftIRLEDOn;
	uint8_t isRightIRLEDOn;
	uint8_t isTransitionFrame;
	uint16_t IntegrationTimeValue;
	std::chrono::time_point<std::chrono::system_clock> frameTime_;
	int frameNumber;
	uint16_t IntegrationTimeMin;
	uint16_t IntegrationTimeMax;
	CX3VideoFormat VideoFormat;
	unsigned long long quadrantLuminescence[4];
};

/// @brief Add new CameraInfo# and set to the latest.
typedef struct CameraImageInfo1 CameraImageInfo;

/// @brief This class provides the definition of the CameraImageInfo class
/// and the functions for loading/saving PGM files with that data embedded.
class EyelockCamera
{
public:
	EyelockCamera();

	bool LoadPGM(const std::string &filename, unsigned char*pDest, int sizeOfDest, CameraImageInfo& imageInfo);
	bool LoadPGM(const std::string& filename, std::vector<unsigned char> &pixels, CameraImageInfo& imageInfo);
	bool SavePGM(const std::string &filename, unsigned char*pImage, int sizeOfImage, CameraImageInfo& imageInfo);

	void SetBitShift(int shiftValue)
	{
		mutex_lock.lock();
		bit_shift_ = shiftValue;
		mutex_lock.unlock();
	}
	
	int GetBitShift()
	{
		mutex_lock.lock();
		int shift = bit_shift_;
		mutex_lock.unlock();
		return shift;
	}

	virtual int GetMinIntegrationTime() {return 50;}
	virtual int GetMaxIntegrationTime() {return 2478;}
	virtual int GetNormalWidth() {return 2592;}
	virtual int GetNormalHeight() {return 1944;}
	int GetImageSize() {return GetNormalWidth() * GetNormalHeight();}

protected:
	int bit_shift_ = 0;
	std::mutex mutex_lock;
};

#endif