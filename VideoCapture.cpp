/*
 * TestMain.cpp
 *
 *  Created on: 11 Dec, 2014
 *      Author: anitha 
 */
#include "pch.h"
#include "VideoCapture.h"
#include "CameraInfoParser.h"

#pragma comment (lib, "quartz.lib")

DEFINE_GUID(MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16,
	0xf54a91d4, 0x7e26, 0x4929, 0xbc, 0xc3, 0xb6, 0x86, 0x6d, 0x53, 0x6a, 0xe5);

//
//HRESULT VideoCapture::Cy_i2c_write(USHORT uwI2cRegAddr,USHORT uwI2cValue)
//{
//    HRESULT hr = 0;
//    long value = 0, flags = 0;
//
//    //
//    // I2C Address
//    //
//
//    hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);
//    hr = CameraControl()->Set(CameraControl_Focus, VENDOR_CMD_I2C_ADDR, flags);
//
//    hr = VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
//    hr = VideoProcAmp()->Set(VideoProcAmp_Hue, uwI2cRegAddr, flags);
//
//    //
//    // I2C Write
//    //
//
//    hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);
//    hr = CameraControl()->Set(CameraControl_Focus, VENDOR_CMD_I2C_WRITE, flags);
//
//    hr = VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
//    value = uwI2cValue;
//    hr = VideoProcAmp()->Set(VideoProcAmp_Hue, value, flags);
//
//    //
//    // dummy state
//    //
//    hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);
//    hr = CameraControl()->Set(CameraControl_Focus, VENDOR_CMD_NONE, flags);
//    //
//    return hr;
//}
//
////
int VideoCapture::Cy_i2c_read(USHORT uwI2cRegAddr)
{

    HRESULT hr = 0;
    int uwI2cValue;
    long value = 0, flags = 0;

    //
    // I2C Set Address to read
    //

    hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);
    hr = CameraControl()->Set(CameraControl_Focus, VENDOR_CMD_I2C_ADDR, flags);

    hr = VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
    hr = VideoProcAmp()->Set(VideoProcAmp_Hue, uwI2cRegAddr, flags);

    //
    // I2C Read Value at address
    //

    hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);
    hr = CameraControl()->Set(CameraControl_Focus, VENDOR_CMD_I2C_READ, flags);
    hr = VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
    uwI2cValue = (int)value;
    //
    // I2C close out read command and reset to dummy state
    //
    hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);
    hr = CameraControl()->Set(CameraControl_Focus, VENDOR_CMD_NONE, flags);
    //
    return uwI2cValue;
}

bool VideoCapture::ReadCameraInfo(CameraInfoParser &cameraInfo)
{
	bool success = false;
	int startAddress = cameraInfo.StartAddress();
	int numBytes = cameraInfo.NumBytesToRead();
	std::vector<byte> vidData(numBytes);
	for (int i = 0; i < numBytes; i++)
		vidData[i] = Cy_i2c_read(startAddress + i);
	cameraInfo.SetCameraInfo(vidData);
	return success;
}

HRESULT VideoCapture::GetCameraFrame(long &sizeBuffer, long *buffer)
{
    HRESULT captureResult = E_FAIL;
    if (sgGetSampleGrabber())
    {
        captureResult = sgGetSampleGrabber()->GetCurrentBuffer(&sizeBuffer, buffer);
		switch (captureResult)
		{
		case VFW_E_NOT_CONNECTED:
			OutputDebugStringA("VFW_E_NOT_CONNECTED\n");
			break;
		case VFW_E_WRONG_STATE:
			OutputDebugStringA("VFW_E_WRONG_STATE\n");
			break;
		case E_INVALIDARG:
			OutputDebugStringA("E_INVALIDARG\n");
			break;
		case E_OUTOFMEMORY:
			OutputDebugStringA("E_OUTOFMEMORY\n");
			break;
		case E_POINTER:
			OutputDebugStringA("E_POINTER\n");
			break;
		case S_OK:
			OutputDebugStringA("S_OK\n");
			break;
		default:
			OutputDebugStringA("default\n");
			break;
		}
    }
    return captureResult;
}

HRESULT VideoCapture::GetCameraFrame(std::vector<uint8_t>& image8Data, std::vector<uint16_t> &image10Data, CRect &rcMaxValue, uint8_t& maxPixelValue)
{
	HRESULT hr = E_FAIL;
	size_t length = size_t(gWidth) * size_t(gHeight);
	long size16Buffer = long(length) * long(sizeof(uint16_t));
	if (0 == image10Data.size())
	{
		image10Data.resize(length);
	}
	hr = GetCameraFrame(size16Buffer, (long*)&image10Data[0]);
	maxPixelValue = 0;
	if (SUCCEEDED(hr))
	{
		if (image8Data.size() != length)
			image8Data.resize(length);
		auto iSrc = image10Data.begin();
		auto iDest8 = image8Data.begin();
		CPoint pt(0,0);
		for (int y = 0; y < gHeight; y++,pt.y++)
		{
			pt.x = 0;
			for (int x = 0; x < gWidth; x++,pt.x++)
			{
				switch (m_BitShift)
				{
				case shift0:
					*iDest8 = 0xff & *iSrc;
					if (0 != (0x300 & *iSrc))
					{
						*iDest8 = 0xff; // white out
					}
					break;
				case shift1:
					*iDest8 = 0xff & (*iSrc >> 1);
					if (0 != (0x200 & *iSrc))
					{
						*iDest8 = 0xff; // white out
					}
					break;
				case shift2:
					*iDest8 = 0xff & (*iSrc >> 2);
					break;
				}
				if (rcMaxValue.PtInRect(pt))
				{
					if (*iDest8 > maxPixelValue)
						maxPixelValue = *iDest8;
				}
				//*iSrc = *iSrc << 6;
				iDest8++;
				iSrc++;
			}
		}
	}
	return hr;
}

HRESULT VideoCapture::irled_pulse_msec(float msec)
{
	//if (msec > 15.0f) msec = 15.0f;

	long lmsec = long(msec * 1000.0f);

	//
	HRESULT hr = 0;
	long value = 0, flags = 0;
	hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);
	if (!SUCCEEDED(hr)){ 
		fprintf(stderr, "ERROR: Get CameraControl_Focus\n");
		return E_FAIL;
	}
	hr = CameraControl()->Set(CameraControl_Focus,
			static_cast<long>(VENDOR_CMD_T::VENDOR_CMD_PULSE_WIDTH), flags);
	if (!SUCCEEDED(hr)){ 
		fprintf(stderr, "ERROR: Set VENDOR_CMD_PULSE_WIDTH\n");
		return E_FAIL;
	}
	hr = VideoProcAmp()->Get(VideoProcAmp_Hue, &value, &flags);
	if (!SUCCEEDED(hr)){ 
		fprintf(stderr, "ERROR: Get VideoProcAmp_Hue\n");
		return E_FAIL;
	}
	//

	//Msg(TEXT("value = %u"), value);

	hr = VideoProcAmp()->Set(VideoProcAmp_Hue, lmsec, flags);
	//
	if (!SUCCEEDED(hr)){ 
		fprintf(stderr, "ERROR: Set VideoProcAmp_Hue\n");
		return E_FAIL;
	}
	return hr;
}
/*

	VENDOR_CMD_LEFT_RIGHT_IR_LED_IN_TURN_PER_FRAME = 1,
	VENDOR_CMD_LEFT_IR_LED_ON_PER_FRAME,
	VENDOR_CMD_RIGHT_IR_LED_ON_PER_FRAME,
	VENDOR_CMD_BOTH_IR_LED_ON_PER_FRAME,
	VENDOR_CMD_BOTH_IR_LED_ALWAYS_OFF,
	VENDOR_CMD_BOTH_IR_LED_ALWAYS_ON,

*/
HRESULT VideoCapture::IrledFlashMode(unsigned int IrLedCtrlType)
{
	VENDOR_CMD_T Cmd;
	switch (IrLedCtrlType) {
	case VENDOR_CMD_LEFT_RIGHT_IR_LED_IN_TURN_PER_FRAME:
		Cmd = VENDOR_CMD_LEFT_RIGHT_IR_LED_IN_TURN_PER_FRAME; break;
	case VENDOR_CMD_LEFT_IR_LED_ON_PER_FRAME:
		Cmd = VENDOR_CMD_LEFT_IR_LED_ON_PER_FRAME; break;
	case VENDOR_CMD_RIGHT_IR_LED_ON_PER_FRAME:
		Cmd = VENDOR_CMD_RIGHT_IR_LED_ON_PER_FRAME; break;
	case VENDOR_CMD_BOTH_IR_LED_ON_PER_FRAME:
		Cmd = VENDOR_CMD_BOTH_IR_LED_ON_PER_FRAME; break;
	case VENDOR_CMD_BOTH_IR_LED_ALWAYS_OFF:
		Cmd = VENDOR_CMD_BOTH_IR_LED_ALWAYS_OFF; break;
	case VENDOR_CMD_BOTH_IR_LED_ALWAYS_ON:
		Cmd = VENDOR_CMD_BOTH_IR_LED_ALWAYS_ON; break;
	default:
		Cmd = VENDOR_CMD_NONE;
	}

	HRESULT hr;
	long value = 0, flags = 0;


	hr = CameraControl()->Get(CameraControl_Focus, &value, &flags);


	if (!SUCCEEDED(hr)){ 
		Msg(TEXT("Failed IrledFlashMode!  hr=0x%x"), hr);
		fprintf(stderr, "ERROR: IrledFlashMode\n");
	}

	value = static_cast<long>(Cmd);
	hr = CameraControl()->Set(CameraControl_Focus, value, flags);
	if (!SUCCEEDED(hr)){ 
		Msg(TEXT("Failed IrledFlashMode!  hr=0x%x"), hr);
		fprintf(stderr, "ERROR: IrledFlashMode\n");
	}
	return hr;
}






VideoCapture::VideoCapture() : m_pCameraControl(nullptr), m_pVideoProcAmp(nullptr),
	pmtConfig(nullptr)
{
	m_pGrabber = nullptr;
	m_pGrabberSettings = nullptr;
	pGrabber = nullptr;
	pVideoWindow = nullptr;
	pMediaEventEx = nullptr;
	pGraphBuilder = nullptr;
	pConfig = nullptr;
	pMediaControl = nullptr;
	pGraphBuilder = nullptr;
	pCaptureGraphBuilder2 = nullptr;
	pBufferSize = 0;
	pBuffer = nullptr;
	g_pSrcFilter = nullptr;
	m_BitShift = shift2;
	gWidth = PICTURE_WIDTH;
	gHeight = PICTURE_HEIGHT;
}


HRESULT VideoCapture::InitCOM()
{
	HRESULT nResult = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(nResult))
	{
		fprintf(stderr, "ERROR: Unable to initialize the COM library on this Threasd..\n");
	}
	TCHAR szBuffer[1024] = TEXT("ERROR: Couldn't Find the device eyeLock");  // Large buffer for long filenames or URLs
	//hr = FindCaptureDevice(0, L"eyeLock's_ov5640_V2.0_RAW", &g_pSrcFilter);
	//if(hr) hr = FindCaptureDevice(0, L"EyeLock_Iris_Biometric_System", &g_pSrcFilter);
	HRESULT hr = FindCaptureDevice(0, TEXT("EyeLock_Iris_Biometric_System"), &g_pSrcFilter);
	// hr = FindCaptureDevice(0, L"eyeLock's cx3 with ov5640 Ver-1.1", &g_pSrcFilter);
	if (hr < 0) {
		printf("ERROR: Couldn't Find the device EyeLock %x\n", hr);
		MessageBox(NULL, szBuffer, TEXT("ERROR: Couldn't Find the device EyeLock"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	else {
		printf("Found the Capture device\n");
	}
	hr = GetInterfaces();
	return hr;
}

IPin* VideoCapture::GetPin(IBaseFilter *pFilter, PIN_DIRECTION PinDir)
{
    BOOL       bFound = FALSE;
    IEnumPins  *pEnum;
    IPin       *pPin;

    pFilter->EnumPins(&pEnum);
    while(pEnum->Next(1, &pPin, 0) == S_OK)
    {
        PIN_DIRECTION PinDirThis;
        pPin->QueryDirection(&PinDirThis);
		bFound = (PinDir == PinDirThis);
        if (bFound)
            break;
        pPin->Release();
    }
    pEnum->Release();
    return (bFound ? pPin : 0);  
}

HRESULT VideoCapture::FindCaptureDevice(int /*eye*/, TCHAR * /*CameraFriendlyName*/, IBaseFilter **ppSrcFilter)
{
	HRESULT hr;
	IBaseFilter *pSrc = NULL;
	IMoniker *pMoniker = NULL; 
	ULONG cFetched;
	if (!ppSrcFilter)
		return E_POINTER;

	// Create the system device enumerator
	ICreateDevEnum *pDevEnum = NULL; 
	hr = CoCreateInstance (CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC, IID_ICreateDevEnum, (void **) &pDevEnum);
	if (FAILED(hr)) {
		return hr;
	}

	// Create an enumerator for the video capture devices
	IEnumMoniker *pClassEnum = NULL;
	hr = pDevEnum->CreateClassEnumerator (CLSID_VideoInputDeviceCategory, &pClassEnum, 0);
	if (FAILED(hr)){
		return hr;
	}

	

	// If there are no enumerators for the requested type, then
	// CreateClassEnumerator will succeed, but pClassEnum will be NULL.
	if (pClassEnum == NULL) {
		return E_FAIL;
	}

	// Note if the Next() call succeeds but there are no monikers,
	// it will return S_FALSE (which is not a failure). Therefore, we
	// check that the return code is S_OK instead of using SUCCEEDED() macro.
	// Possible future camera id procedure...
	int	DeviceCount = 0;	
	bool DeviceFound = false;
	while(S_OK == (pClassEnum->Next(1, &pMoniker, &cFetched))){

		DeviceCount++;
		printf("******************\n");
		printf("Device count: %d\n", DeviceCount);
		

		IPropertyBag *pPropBag;
		hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
		if (SUCCEEDED(hr)){
			VARIANT varName;
			VariantInit(&varName);
			hr = pPropBag->Read(L"FriendlyName", &varName, 0);
			
				TCHAR DeviceName[MAX_PATH];
				WideCharToMultiByte(CP_ACP, 0, varName.bstrVal, -1, (LPSTR)DeviceName, MAX_PATH, NULL, NULL);
				printf("Device: %s\n", (char*)DeviceName);

			if(SUCCEEDED(hr)){

				if(strstr((char*)DeviceName, "eyeLock") != NULL){
					printf("*eyeLock device*\n"); 
					DeviceFound=true;
					break;
				}
				if(strstr((char*)DeviceName, "EyeLock") != NULL){
					printf("*EyeLock device*\n"); 
					DeviceFound=true;
					break;
				}
				/*
				if((wcscmp(varName.bstrVal,CameraFriendlyName) == 0)){
						if( eye == 0 ) {
							// printf("Found the Device ov5640\n"); 
							//printf("Found the Device\n"); 
							DeviceFound=true;
							break;
						}
						else
							if( ++DeviceCount > 1 ) {
								DeviceFound=true;
								break; // Found second capture device.
							}
				}
				*/
			}
		} 
		
		// Prepare to Grab next moniker...
		pMoniker = NULL;
	}

	// If we found our capture device...bind to it.
	if( DeviceFound ) {
		// Bind Moniker to a filter object
		hr = pMoniker->BindToObject(0,0,IID_IBaseFilter, (void**)&pSrc);
		if (FAILED(hr)){
			return hr;
		}
	}
	else {
		return E_FAIL;
	}

	// Copy the found filter pointer to the output parameter.
	// Do NOT Release() the reference, since it will still be used
	// by the calling function.
	*ppSrcFilter = pSrc;
	
	
	return hr;
}

HRESULT VideoCapture::sgSetSampleGrabberMediaType(ISampleGrabber *pGrab)
{
        AM_MEDIA_TYPE mt;
        ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
        mt.majortype = MEDIATYPE_Video;

		//-----------------------
		//
		// two types of GUID
		//
		if(pmtConfig->subtype == MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16){
			mt.subtype = MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16; 
			//printf("GUID MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16\n"); 
		}
		if(pmtConfig->subtype == MEDIASUBTYPE_YUY2){
			mt.subtype = MEDIASUBTYPE_YUY2; 
			//printf("GUID MEDIASUBTYPE_YUY2\n"); 
		}
		//-----------------------

		//mt.subtype = MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16; 
		//mt.subtype = MEDIASUBTYPE_YUY2; 
        HRESULT hr = pGrab->SetMediaType(&mt);
        if (FAILED(hr)) {
			return hr;
			/*
			mt.subtype = MEDIASUBTYPE_YUY2; 
			hr = pGrab->SetMediaType(&mt);
			printf("***Success GUID \n"); 
			if (FAILED(hr)){ 

				printf("Failed GUID \n"); 
				printf("Failed GUID \n"); 
				printf("Failed GUID \n"); 

				return hr;
			}
			*/
        }
		//printf("***Success GUID \n"); 
        hr = pGrab->SetOneShot(FALSE);
        hr = pGrab->SetBufferSamples(TRUE);
        return hr;
}

unsigned char*  VideoCapture::sgGrabData(ISampleGrabber *pGrab)
{
        HRESULT hr;

        if (pGrab == 0)
                return 0;

		long Size = 1920*1080*2; 
        // long Size = 0;
        hr = pGrab->GetCurrentBuffer(&Size, NULL);
        if (FAILED(hr))
                return 0;
        else if (Size != pBufferSize) {
                pBufferSize = Size;
                if (pBuffer != 0)
                        delete[] pBuffer;
                pBuffer = new unsigned char[pBufferSize];
        }

        hr = pGrab->GetCurrentBuffer(&pBufferSize, (long*)pBuffer);
        if (FAILED(hr))
			return 0;
		else {        
            return pBuffer;                
        }
}


void VideoCapture::sgFreeMediaType(AM_MEDIA_TYPE& mt)
{
    if (mt.cbFormat != 0)
    {
        CoTaskMemFree((PVOID)mt.pbFormat);
        mt.cbFormat = 0;
        mt.pbFormat = NULL;
    }
    if (mt.pUnk != NULL)
    {        
        mt.pUnk->Release();
        mt.pUnk = NULL;
    }
}

IBaseFilter* VideoCapture::sgGetBaseFilter()
{
	return g_pSrcFilter;
}

ISampleGrabber* VideoCapture::sgGetSampleGrabber()
{
	return m_pGrabberSettings;
}


void VideoCapture::sgCloseSampleGrabber()
{
        if (pBuffer != 0) {
                delete[] pBuffer;
                pBuffer = 0;
                pBufferSize = 0;
        }

		if (g_pSrcFilter)
			SAFE_RELEASE(g_pSrcFilter);
		if (pGrabber)
			SAFE_RELEASE(pGrabber);

        gWidth = 0;
        gHeight = 0;
}

HRESULT VideoCapture::sgAddSampleGrabber(IGraphBuilder *pGraph)
{
        // Create the Sample Grabber.
        HRESULT hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
                                      IID_IBaseFilter, (void**) & g_pSrcFilter);
        if (FAILED(hr)) {
                return hr;
        }
        hr = pGraph->AddFilter(g_pSrcFilter, L"Sample Grabber");
        if (FAILED(hr)) {
                return hr;
        }

        g_pSrcFilter->QueryInterface(IID_ISampleGrabber, (void**)&pGrabber);
        return hr;
}

HRESULT VideoCapture::sgGetSampleGrabberMediaType(ISampleGrabber * pGrab)
{
	AM_MEDIA_TYPE mt = { 0 };
    HRESULT hr = pGrab->GetConnectedMediaType(&mt);
    if (FAILED(hr))
	{
		return hr;
    }

    VIDEOINFOHEADER *pVih = reinterpret_cast<VIDEOINFOHEADER *>(mt.pbFormat);
	if (pVih != NULL)
	{
		gWidth = pVih->bmiHeader.biWidth;
		gHeight = pVih->bmiHeader.biHeight;
	}
	else
	{
		gWidth = PICTURE_WIDTH;
		gHeight = PICTURE_HEIGHT;
	}
	printf("gWidth....%d\n", gWidth);
	printf("gHeight....%d\n", gHeight);

    sgFreeMediaType(mt);
    return hr;
}



long VideoCapture::sgGetBufferSize()
{
        return pBufferSize;
}

unsigned int VideoCapture::sgGetDataWidth()
{
        return gWidth;
}

unsigned int VideoCapture::sgGetDataHeight()
{
        return gHeight;
}

HRESULT VideoCapture::GetInterfaces()
{
	HRESULT hr = S_FALSE;

	if (!pGraphBuilder)
	{
		// Create the filter graph
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC,
			IID_IGraphBuilder, (void **)& pGraphBuilder);
		if (FAILED(hr))
			return hr;

		// Create the capture graph builder
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC,
			IID_ICaptureGraphBuilder2, (void **)& pCaptureGraphBuilder2);
		if (FAILED(hr))
			return hr;

		// Obtain interfaces for media control and Video Window
		hr = pGraphBuilder->QueryInterface(IID_IMediaControl, (LPVOID *)& pMediaControl);
		if (FAILED(hr))
			return hr;

		hr = pGraphBuilder->QueryInterface(IID_IVideoWindow, (LPVOID *)& pVideoWindow);
		if (FAILED(hr))
			return hr;

		hr = pGraphBuilder->QueryInterface(IID_IMediaEvent, (LPVOID *)& pMediaEventEx);
		if (FAILED(hr))
			return hr;
	}

	if (g_pSrcFilter)
	{

		//
		// not tested, never called
		//

		hr = g_pSrcFilter->QueryInterface(IID_IAMCameraControl, reinterpret_cast<void **>(&m_pCameraControl));
		if (FAILED(hr)) Msg(TEXT("Failed to get IID_IAMCameraControl!  hr=0x%x"), hr);



		hr = g_pSrcFilter->QueryInterface(IID_IAMVideoProcAmp, reinterpret_cast<void **>(&m_pVideoProcAmp));
		if (FAILED(hr)) Msg(TEXT("Failed to get IID_IAMVideoProcAmp!  hr=0x%x"), hr);


		// Set the window handle used to process graph events
		//hr = g_pME->SetNotifyWindow((OAHWND)hWnd, WM_GRAPHNOTIFY, 0);
	}
	return hr;
}


void VideoCapture::CloseInterfaces(void)
{
        // Stop previewing data
        if (pMediaControl)
                pMediaControl->StopWhenReady();

        //g_psCurrent = Stopped;

        // Stop receiving events
       // if (g_pME)
            //    g_pME->SetNotifyWindow(NULL, WM_GRAPHNOTIFY, 0);

        // Relinquish ownership (IMPORTANT!) of the video window.
        // Failing to call put_Owner can lead to assert failures within
        // the video renderer, as it still assumes that it has a valid
        // parent window.
        if (pVideoWindow) {
                pVideoWindow->put_Visible(OAFALSE);
                pVideoWindow->put_Owner(NULL);
        }

#ifdef REGISTER_FILTERGRAPH
        // Remove filter graph from the running object table
        if (g_dwGraphRegister)
                RemoveGraphFromRot(g_dwGraphRegister);
#endif

        // Release DirectShow interfaces
        SAFE_RELEASE(pMediaControl);
        SAFE_RELEASE(pMediaEventEx);
        SAFE_RELEASE(pVideoWindow);
        SAFE_RELEASE(pGraphBuilder);
        SAFE_RELEASE(pCaptureGraphBuilder2);
}

HRESULT VideoCapture::vcCaptureVideo()  
{    

        HRESULT hr;
        IBaseFilter *pSrcFilter = NULL;

        // Get DirectShow interfaces
        hr = GetInterfaces();
        if (FAILED(hr)) {
                Msg(TEXT("Failed to get video interfaces!  hr=0x%x"), hr);
                return hr;
        }

        // Attach the filter graph to the capture graph
        hr = pCaptureGraphBuilder2->SetFiltergraph(pGraphBuilder);
        if (FAILED(hr)) {
                Msg(TEXT("Failed to set capture filter graph!  hr=0x%x"), hr);
                return hr;
        }

        // Use the system device enumerator and class enumerator to find
        // a video capture/preview device, such as a desktop USB video camera.
        //hr = FindCaptureDevice(&pSrcFilter);
        //if (FAILED(hr)) {
                // Don't display a message because FindCaptureDevice will handle it
               // return hr;
        //}

#if 0
        // Add Capture filter to our graph.
        hr = pGraphBuilder->AddFilter(pSrcFilter, L"Video Capture");
        if (FAILED(hr)) {
                Msg(TEXT("Couldn't add the capture filter to the graph!  hr=0x%x\r\n\r\n")
                    TEXT("If you have a working video capture device, please make sure\r\n")
                    TEXT("that it is connected and is not being used by another application.\r\n\r\n")
                    TEXT("The sample will now close."), hr);
                pSrcFilter->Release();
                return hr;
        }
#endif

        hr = sgAddSampleGrabber(pGraphBuilder);
        if (FAILED(hr)) {
                Msg(TEXT("Couldn't add the SampleGrabber filter to the graph!  hr=0x%x"), hr);
                return hr;
        }
        hr = sgSetSampleGrabberMediaType(m_pGrabberSettings);
        if (FAILED(hr)) {
                Msg(TEXT("Couldn't set the SampleGrabber media type!  hr=0x%x"), hr);
                return hr;
        }
        IBaseFilter* pGrab = sgGetBaseFilter();


        // Render the preview pin on the video capture filter
        // Use this instead of g_pGraph->RenderFile
        
        hr = pCaptureGraphBuilder2->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                      pSrcFilter, pGrab/*NULL*/, NULL);
        if (FAILED(hr)) {
                Msg(TEXT("Couldn't render the video capture stream.  hr=0x%x\r\n")
                    TEXT("The capture device may already be in use by another application.\r\n\r\n")
                    TEXT("The sample will now close."), hr);
                pSrcFilter->Release();
                return hr;
        }

        hr = sgGetSampleGrabberMediaType(m_pGrabberSettings);

        // Now that the filter has been added to the graph and we have
        // rendered its stream, we can release this reference to the filter.
        pSrcFilter->Release();

        // Set video window style and position
        //hr = SetupVideoWindow(prvWindow);
        //if (FAILED(hr)) {
               // Msg(TEXT("Couldn't initialize video window!  hr=0x%x"), hr);
              //  return hr;
       // }


        // Start previewing video data
        hr = pMediaControl->Run();
        if (FAILED(hr)) {
                Msg(TEXT("Couldn't run the graph!  hr=0x%x"), hr);
                return hr;
        }

        // Remember current state
        //g_psCurrent = Running;

        return S_OK;
}

void VideoCapture::vcStopCaptureVideo()
{
	sgCloseSampleGrabber();
    CloseInterfaces();
}

void VideoCapture::Msg(TCHAR *szFormat, ...)
{
        TCHAR szBuffer[1024];  // Large buffer for long filenames or URLs
        const size_t NUMCHARS = sizeof(szBuffer) / sizeof(szBuffer[0]);
        const int LASTCHAR = NUMCHARS - 1;

        // Format the input string
        va_list pArgs;
        va_start(pArgs, szFormat);

        // Use a bounded buffer size to prevent buffer overruns.  Limit count to
        // character size minus one to allow for a NULL terminating character.
        (void)StringCchVPrintf(szBuffer, NUMCHARS - 1, szFormat, pArgs);
        va_end(pArgs);

        // Ensure that the formatted string is NULL-terminated
        szBuffer[LASTCHAR] = TEXT('\0');

        MessageBox(NULL, szBuffer, TEXT("PlayCap Message"), MB_OK | MB_ICONERROR);
}

HRESULT VideoCapture::RestartVideoCapture()
{
	vcStopCaptureVideo();
	HRESULT hr = StartVideoCapture();
	return hr;
}

HRESULT VideoCapture::StartVideoCapture()
{
	HRESULT hr; 	
#if 1
	TCHAR szBuffer[1024] = TEXT("ERROR: Couldn't Find the device eyeLock");  // Large buffer for long filenames or URLs
	//hr = FindCaptureDevice(0, L"eyeLock's_ov5640_V2.0_RAW", &g_pSrcFilter);
	//if(hr) hr = FindCaptureDevice(0, L"EyeLock_Iris_Biometric_System", &g_pSrcFilter);
	hr = FindCaptureDevice(0, TEXT("EyeLock_Iris_Biometric_System"), &g_pSrcFilter);
	// hr = FindCaptureDevice(0, L"eyeLock's cx3 with ov5640 Ver-1.1", &g_pSrcFilter);
	if(hr < 0){
		printf("ERROR: Couldn't Find the device EyeLock %x\n", hr);
		MessageBox(NULL, szBuffer, TEXT("ERROR: Couldn't Find the device EyeLock"), MB_OK | MB_ICONERROR);
		exit(0); 
	}else{
		printf("Found the Capture device\n");
	}
	// Find Interfaces
	if (!pGraphBuilder)
		hr = CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC, IID_IGraphBuilder, (LPVOID *)&pGraphBuilder);
	if (!pCaptureGraphBuilder2)
		hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, NULL, CLSCTX_INPROC, IID_ICaptureGraphBuilder2, (LPVOID *)&pCaptureGraphBuilder2);
    //hr = CoInitialize(0);    
	if (!pConfig)
		hr = pCaptureGraphBuilder2->FindInterface(&PIN_CATEGORY_CAPTURE, 0, g_pSrcFilter, IID_IAMStreamConfig, (void**)&pConfig);
    int iCount = 0, iSize = 0;	
	hr = pConfig->GetNumberOfCapabilities(&iCount, &iSize);
	printf("*iCount: %d\n", iCount);
	if (iSize == sizeof(VIDEO_STREAM_CONFIG_CAPS)) {
		// Use the video capabilities structure.
		for (int iFormat = 0; iFormat < iCount; iFormat++) {
			VIDEO_STREAM_CONFIG_CAPS scc;
            //AM_MEDIA_TYPE *pmtConfig;
            hr = pConfig->GetStreamCaps(iFormat, &pmtConfig, (BYTE*)&scc);
            if (SUCCEEDED(hr)) {
				printf("*iformat: %d\n", iFormat);
				/* Examine the format, and possibly use it. */
                if ((pmtConfig->majortype == MEDIATYPE_Video) && 
					//-----------------------
					// two types of GUID
					((pmtConfig->subtype == MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16) ||
					(pmtConfig->subtype == MEDIASUBTYPE_YUY2)) &&
					//-----------------------
                    (pmtConfig->formattype == FORMAT_VideoInfo) &&
                    (pmtConfig->cbFormat >= sizeof (VIDEOINFOHEADER)) &&
                    (pmtConfig->pbFormat != NULL)) {
						if(pmtConfig->subtype == MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16){
							printf("GUID MEDIASUBTYPE_EYELOCK_GREY_LOW_10_OF_16\n"); 
						}
						if(pmtConfig->subtype == MEDIASUBTYPE_YUY2){
							printf("GUID MEDIASUBTYPE_YUY2\n"); 
						}
						VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
						gWidth = pVih->bmiHeader.biWidth;
                        gHeight = pVih->bmiHeader.biHeight;	
						pConfig->SetFormat(pmtConfig);
						printf("*gWidth....%d\n", gWidth);
						printf("*gHeight....%d\n", gHeight);
						if(gWidth < 2500){ // 2mp 1920x1080
							break;
						}
						if(gWidth > 2500){ // 5mp 2592x1944
							break;
						}
                    }
					/*
                    if (iFormat == 26) { // 5mp
						VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER*)pmtConfig->pbFormat;
						gWidth = pVih->bmiHeader.biWidth;
                        gHeight = pVih->bmiHeader.biHeight;	
						pConfig->SetFormat(pmtConfig);
						printf("*gWidth....%d\n", gWidth);
						printf("*gHeight....%d\n", gHeight);
                    }
					*/
                    // Delete the media type when you are done.
                    // sgFreeMediaType(pmtConfig);
				}
			}
       }
           
    // set grabber properties
	if (!m_pGrabber)
		hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER, IID_IBaseFilter, (void**)&m_pGrabber); // create ISampleGrabber     
	if (!pGraphBuilder)
		hr = pCaptureGraphBuilder2->SetFiltergraph(pGraphBuilder); // set FilterGrap
	if (!pMediaControl)
		hr = pGraphBuilder->QueryInterface(IID_IMediaControl, (LPVOID *)&pMediaControl); // get MediaControl interface
	if (!m_pGrabberSettings)
		hr = m_pGrabber->QueryInterface(IID_ISampleGrabber, (void**)&m_pGrabberSettings);
	 
	// Set the Media Type
	sgSetSampleGrabberMediaType(m_pGrabberSettings); 

	// build filter graph
	pGraphBuilder->AddFilter(g_pSrcFilter, L"Device Filter");
    pGraphBuilder->AddFilter(m_pGrabber, L"Sample Grabber");


	if (!m_pCameraControl)
	{
		hr = g_pSrcFilter->QueryInterface(IID_IAMCameraControl, reinterpret_cast<void**>(&m_pCameraControl));
		if (FAILED(hr))
			Msg(TEXT("Failed to get IID_IAMCameraControl!  hr=0x%x"), hr);
	}

	if (!m_pVideoProcAmp)
	{
		hr = g_pSrcFilter->QueryInterface(IID_IAMVideoProcAmp, reinterpret_cast<void**>(&m_pVideoProcAmp));
		if (FAILED(hr))
			Msg(TEXT("Failed to get IID_IAMVideoProcAmp!  hr=0x%x"), hr);
	}
	IPin* pSourceOut_0 = GetPin(g_pSrcFilter, PINDIR_OUTPUT);
    IPin* pGrabberIn_0 = GetPin(m_pGrabber, PINDIR_INPUT);
	hr = pGraphBuilder->Connect(pSourceOut_0, pGrabberIn_0);

#endif
	// wrong place for this call????
	//hr = sgGetSampleGrabberMediaType(m_pGrabberSettings); 
	     
	// start playing
    hr = pMediaControl->Run();	
	
	return hr;
}

