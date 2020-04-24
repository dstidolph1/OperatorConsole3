/*
 * VideoCapture.h
 *
 *  Created on: 11 Dec, 2014
 *      Author: anitha 
 */

#ifndef _VIDEO_CAPTURE
#define _VIDEO_CAPTURE

//#include <stdio.h>
//#include <atlstr.h>
//#include <streams.h>

//#include <cv.h>
//#include <highgui.h>
//#include <cxcore.h>


//#include "EyelockLEDControl.h"
//#include "..\CyAPI_include\CyAPI.h"
#include <dshow.h>
#include "dxtrans.h"
#include <initguid.h>
#include <string>
#include <sstream>
#include <iomanip>
#include "CameraInfoParser.h"
#include "QEdit.h"

#define PICTURE_WIDTH 2592
#define PICTURE_HEIGHT 1944

typedef enum
{
	shift2, // Use top 8 bits
	shift1, // Use middle 8 bits
	shift0 // Use bottom 8 Bits
} BitShiftType;

typedef enum vendor_cmd_t {

	// dummy command
	VENDOR_CMD_NONE = 0,

	// compatible with old vendor commands
	VENDOR_CMD_LEFT_RIGHT_IR_LED_IN_TURN_PER_FRAME = 1,
	VENDOR_CMD_LEFT_IR_LED_ON_PER_FRAME,
	VENDOR_CMD_RIGHT_IR_LED_ON_PER_FRAME,
	VENDOR_CMD_BOTH_IR_LED_ON_PER_FRAME,
	VENDOR_CMD_BOTH_IR_LED_ALWAYS_OFF,
	VENDOR_CMD_BOTH_IR_LED_ALWAYS_ON,

	// compatible with old vendor commands
	/*IR_LED_TYPE_LEFT_RIGHT_IR_LED_IN_TURN_PER_FRAME = 1,
	IR_LED_TYPE_LEFT_IR_LED_ON_PER_FRAME,
	IR_LED_TYPE_RIGHT_IR_LED_ON_PER_FRAME,
	IR_LED_TYPE_BOTH_IR_LED_ON_PER_FRAME,
	IR_LED_TYPE_BOTH_IR_LED_OFF_PER_FRAME,*/

	// new vendor commands
	VENDOR_CMD_FW_VERSION = 10,
	VENDOR_CMD_IR_LED_TYPE,
	VENDOR_CMD_I2C_ADDR,
	VENDOR_CMD_I2C_READ,
	VENDOR_CMD_I2C_WRITE,
	VENDOR_CMD_PULSE_TIMING,
	VENDOR_CMD_PULSE_WIDTH,
	
	VENDOR_CMD_LED_CTRL,
	VENDOR_CMD_I2C_SET_BURST_NUM,
	VENDOR_CMD_I2C_BURST_WRITE,
	VENDOR_CMD_I2C_BURST_READ,

} VENDOR_CMD_T;

typedef enum vendor_cmd_op_t{
	VENDOR_CMD_OP_SET = 0,
	VENDOR_CMD_OP_GET,
} VENDOR_CMD_OP_T;

enum eLedCtrl{
	LED_OFF = 0x00,
	LED_R_ON,
	LED_R_OFF,
	LED_G_ON,
	LED_G_OFF,
	LED_B_ON,
	LED_B_OFF,
	LED_ON = 0xFF,
};

class CameraIDInfo
{
public:
	int ver;
	int lot[9];
	int wafer;
	int chipNumber;

	std::string IntToHex(int value, int numChars)
	{
		std::stringstream stream;
		stream << std::setfill('0') << std::setw(numChars) << std::hex << value;
		return stream.str();
	}

	std::string ToString()
	{
		std::string output = IntToHex(ver,1) + "-";
		for (int i = 0; i < 9; i++)
			output += IntToHex(lot[i],1) + "-";
		output += IntToHex(wafer, 2) + "-";
		output += IntToHex(chipNumber, 5);
		return output;
	}
};

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

class VideoCapture{
private:
	int gWidth; 
	int gHeight;
	ISampleGrabber *pGrabber; 
	long pBufferSize; 
	unsigned char* pBuffer; 
	IBaseFilter *m_pGrabber; 
	ISampleGrabber *m_pGrabberSettings; 
	IAMStreamConfig *pConfig; 
	IMediaControl *pMediaControl; 
	IGraphBuilder *pGraphBuilder; 
	ICaptureGraphBuilder2 *pCaptureGraphBuilder2; 
	IVideoWindow  *pVideoWindow;
	IMediaEventEx *pMediaEventEx;
	BitShiftType m_BitShift;

	IAMCameraControl *m_pCameraControl;
	IAMVideoProcAmp	*m_pVideoProcAmp;

	AM_MEDIA_TYPE *pmtConfig;
	std::vector<uint16_t> m_cameraFrame;
	 
public:	
	VideoCapture();
	virtual ~VideoCapture() {};

	unsigned int GetWidth() { return gWidth; }
	unsigned int GetHeight() { return gHeight; }

	IAMCameraControl *CameraControl() { return m_pCameraControl; }
	IAMVideoProcAmp *VideoProcAmp() { return m_pVideoProcAmp; }
	 
	HRESULT irled_pulse_msec(float msec);
	HRESULT IrledFlashMode(unsigned int IrLedCtrlType);

	void SetBitShift(BitShiftType shift) { m_BitShift = shift; }

	int Cy_i2c_read(USHORT uwI2cRegAddr);
 //   HRESULT Cy_i2c_write(USHORT uwI2cRegAddr,USHORT uwI2cValue);


	bool ReadCameraInfo(CameraInfoParser &cameraInfo);

	IBaseFilter *g_pSrcFilter; // Source Rendering filter
	HRESULT InitCOM(); 
	HRESULT FindCaptureDevice(int eye, TCHAR *CameraFriendlyName, IBaseFilter **ppSrcFilter); 
	IPin *GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir); 
	HRESULT sgSetSampleGrabberMediaType(ISampleGrabber *pGrabber); 
	unsigned char* sgGrabData(ISampleGrabber *pGrabber); 
	void sgFreeMediaType(AM_MEDIA_TYPE& mt); 
	IBaseFilter* sgGetBaseFilter();
	ISampleGrabber* sgGetSampleGrabber(); 
	void sgCloseSampleGrabber();
	HRESULT sgAddSampleGrabber(IGraphBuilder *pGraph);
	HRESULT sgGetSampleGrabberMediaType(ISampleGrabber *pGrabber);
	long sgGetBufferSize();
	unsigned int sgGetDataWidth();
	unsigned int sgGetDataHeight();
	unsigned int sgGetDataChannels(); 
	HRESULT GetInterfaces();
	void CloseInterfaces(void); 
	HRESULT vcCaptureVideo();  
	void vcStopCaptureVideo(); 
	void Msg(TCHAR *szFormat, ...); 
	HRESULT StartVideoCapture();

	HRESULT RestartVideoCapture();

    HRESULT GetCameraFrame(long &sizeBuffer, long *buffer);

	HRESULT GetCameraFrame(std::vector<uint8_t>& image8Data, std::vector<uint16_t> &image10Data, CRect &rcMaxValue, uint8_t &maxPixelValue);
};

#endif

