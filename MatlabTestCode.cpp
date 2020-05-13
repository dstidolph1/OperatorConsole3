#include <afxwin.h>
#include "MatlabTestCode.h"
#include <iostream>
#include <numeric> // for iota
#include "Logging.h"

MatlabTestCode::MatlabTestCode()
{
}

MatlabTestCode::~MatlabTestCode()
{

}

bool MatlabTestCode::Initialize()
{
	bool success = false;
	try {
		auto mode = mc::MATLABApplicationMode::IN_PROCESS;
		std::vector<std::u16string> opts = { u"-logfile",
						u"C:\\temp\\matlab_app.log" };
		m_matlabApplication = mc::initMATLABApplication(mode, opts);

		const std::string STR_CTF_NAME = "C:\\Eyelock\\OperatorConsole3\\MatlabMTFLib_3.ctf"; // this could hold path information
		const std::u16string U16STR_CTF_NAME = matlab::cpplib::convertUTF8StringToUTF16String(STR_CTF_NAME);

		m_matlabLibrary = mc::initMATLABLibrary(m_matlabApplication, U16STR_CTF_NAME);
		success = true;
	}
	catch (matlab::cpplib::CppSharedLibException& e)
	{
		std::string s = "matlab::cpplib::CppSharedLibException in initMATLABLibrary: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (const std::exception& exc) {
		std::string s = "std::exception in initMATLABLibrary: ";
		s += exc.what();
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
		LOGMSG_ERROR(s.c_str());
	}
	return success;
}

void MatlabTestCode::Shutdown()
{
	m_matlabApplication.reset();
}

//bool MW_CALL_CONV mlxFullChartMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);
//bool MW_CALL_CONV mlfFullChartMTF50(int nargout, mxArray** out, mxArray* image);
//fullChartMTF50(image as 2D, 8 bit array),  retVal is a 2D array of doubles.  
//There will be 46 rows and 3 columns in retVal.  Each row is a detected edge.  
//There are only 46 because with our setup the field of view only allows us to see 46 edges.  
//Column 1 are the X-coordinates of the edges.  Column 2 are the Y-coordinates of the edges.  
//Column 3 are all the MTF50 vals for the edges
bool MatlabTestCode::RunTestFullChartMTF50(std::vector<uint16_t>& image, int width, int height, const std::vector<CPoint>& regPts, std::vector<FullChartMTF50Data>& outputData)
{
	bool success = false;
	try
	{
		md::ArrayFactory factory;
		size_t imageLength = size_t(width) * size_t(height);
		md::buffer_ptr_t<uint16_t> imageData = factory.createBuffer<uint16_t>(imageLength);
		uint16_t* pImage = imageData.get();
		memcpy(pImage, &image[0], imageLength * sizeof(imageData[0]));
		md::TypedArray<double> xArray = factory.createArray<double>({ 4, 1 },
			{ double(regPts[0].x), double(regPts[1].x), double(regPts[2].x), double(regPts[3].x) });
		md::TypedArray<double> yArray = factory.createArray<double>({ 4, 1 },
			{ double(regPts[0].y), double(regPts[1].y), double(regPts[2].y), double(regPts[3].y) });
		unsigned long long imageWidth = width;
		unsigned long long imageHeight = height;
		md::TypedArray<uint16_t> imageArray = factory.createArrayFromBuffer<uint16_t>({ imageHeight, imageWidth }, std::move(imageData), md::MemoryLayout::ROW_MAJOR);
		std::vector<md::Array>params{ imageArray, xArray, yArray };
		std::vector<matlab::data::Array> output = m_matlabLibrary->feval("fullChartMTF50", 1, params);
		for (auto outputElement : output)
		{
			size_t numElements = outputElement.getNumberOfElements();
			size_t numRows = numElements / 3;
			outputData.reserve(numElements + outputData.capacity());
			for (auto nRow = 0; nRow < numRows; nRow++)
			{
				FullChartMTF50Data data;
				data.x = outputElement[nRow][0];
				data.y = outputElement[nRow][1];
				data.mtf50 = outputElement[nRow][2];
				outputData.push_back(data);
			}
		}
		success = !outputData.empty();
	}
	catch (matlab::OutOfMemoryException& e)
	{
		std::string s = "matlab::OutOfMemoryException in fullChartMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidArrayTypeException& e)
	{
		std::string s = "matlab::data::InvalidArrayTypeException in fullChartMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidMemoryLayoutException& e)
	{
		std::string s = "matlab::data::InvalidMemoryLayoutException in fullChartMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidDimensionsInRowMajorArrayException& e)
	{
		std::string s = "matlab::data::InvalidDimensionsInRowMajorArrayException in fullChartMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::NumberOfElementsExceedsMaximumException& e)
	{
		std::string s = "matlab::data::NumberOfElementsExceedsMaximumException in fullChartMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::Exception& e)
	{
		std::string s = "matlab::Exception in fullChartMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	return success;
}

//bool MW_CALL_CONV mlxFullChartSNR(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);
//bool MW_CALL_CONV mlfFullChartSNR(int nargout, mxArray** out, mxArray* image);
//fullChartSNR():  expects a 2D, 8 bit array  for image, and returns a 2D array with mean intensity(first column), rms noise (second column) and signal to noise ratio (third column) for all gray boxes.
//fullChartSNR(): expects a 2D, 8 bit array for image, and returns  a 2D array of doubles.
//There will be 20 rows and 5 columns in retVal.
//There will be 1 row per gray box. 
//  Column 1 are the X - coordinates of the box centers.
//  Column 2 are the Y - coordinates of the box centers.
//  Column 3 are the mean intensity vals.
//  Column 4 are the noise vals.
//  Column 5 are all the SNR vals.
bool MatlabTestCode::RunTestFullChartSNR(std::vector<uint16_t>& image, int width, int height, const std::vector<CPoint>& regPts, std::vector<FullChartSNRData>& outputData)
{
	bool success = false;
	try
	{
		md::ArrayFactory factory;
		size_t imageLength = size_t(width) * size_t(height);
		md::buffer_ptr_t<uint16_t> imageData = factory.createBuffer<uint16_t>(imageLength);
		uint16_t* pImage = imageData.get();
		memcpy(pImage, &image[0], imageLength * sizeof(image[0]));
		md::TypedArray<double> xArray = factory.createArray<double>({ 4, 1 },
			{ double(regPts[0].x), double(regPts[1].x), double(regPts[2].x), double(regPts[3].x) });
		md::TypedArray<double> yArray = factory.createArray<double>({ 4, 1 },
			{ double(regPts[0].y), double(regPts[1].y), double(regPts[2].y), double(regPts[3].y) });
		unsigned long long imageWidth = width;
		unsigned long long imageHeight = height;
		md::TypedArray<uint16_t> imageArray = factory.createArrayFromBuffer<uint16_t>({ imageHeight, imageWidth }, std::move(imageData), md::MemoryLayout::ROW_MAJOR);
		std::vector<md::Array>params{ imageArray, xArray, yArray };
		std::vector<matlab::data::Array> output = m_matlabLibrary->feval("fullChartSNR", 1, params);
		outputData.clear();
		for (auto outputElement : output) // process vector of 20 rows
		{
			size_t numElements = outputElement.getNumberOfElements(); // Should be 5
			size_t numRows = numElements / 5;
			md::ArrayType A1 = outputElement.getType();
			outputData.reserve(numElements + outputData.capacity());
			for (auto nRow = 0; nRow < numRows; nRow++)
			{
				FullChartSNRData data;
				data.x = outputElement[nRow][0];
				data.y = outputElement[nRow][1];
				data.meanIntensity = outputElement[nRow][2];
				data.RMSNoise = outputElement[nRow][3];
				data.SignalToNoise = outputElement[nRow][4];
				outputData.push_back(data);
			}
		}
		success = !outputData.empty();
	}
	catch (matlab::OutOfMemoryException& e)
	{
		std::string s = "matlab::OutOfMemoryException in fullChartSNR: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidArrayTypeException& e)
	{
		std::string s = "matlab::data::InvalidArrayTypeException in fullChartSNR: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidMemoryLayoutException& e)
	{
		std::string s = "matlab::data::InvalidMemoryLayoutException in fullChartSNR: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidDimensionsInRowMajorArrayException& e)
	{
		std::string s = "matlab::data::InvalidDimensionsInRowMajorArrayException in fullChartSNR: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::NumberOfElementsExceedsMaximumException& e)
	{
		std::string s = "matlab::data::NumberOfElementsExceedsMaximumException in fullChartSNR: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::Exception& e)
	{
		std::string s = "matlab::Exception in fullChartSNR: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	return success;
}

//bool MW_CALL_CONV mlxQuickMTF50(int nlhs, mxArray* plhs[], int nrhs, mxArray* prhs[]);
//bool MW_CALL_CONV mlfQuickMTF50(int nargout, mxArray** out, mxArray* image, mxArray* X_coords, mxArray* Y_coords);

//quickMTF50(image as 2D, 8 bit array, 4 X-coordinates as array of 4 doubles, 4 Y-coordinates as array of 4 doubles).
//The 4X and 4Y coordinates represent the locations of the four fiducials numbered from top left to bottom left in clockwise direction.  
//retVal is an array of 5 doubles representing 5 MTF50 vals for edges 12,27,32, 33, 50.
bool MatlabTestCode::RunTestQuickMTF50(std::vector<uint8_t>& image, int width, int height, const std::vector<CPoint>& regPts, std::vector<QuickMTF50Data>& outputData)
{
	bool success = false;
	try
	{
		md::ArrayFactory factory;
		size_t imageLength = size_t(width) * size_t(height);
		md::buffer_ptr_t<uint8_t> imageData = factory.createBuffer<uint8_t>(imageLength);
		uint8_t* pImage = imageData.get();
		memcpy(pImage, &image[0], imageLength);
		md::TypedArray<double> xArray = factory.createArray<double>({ 4, 1 },
			{ double(regPts[0].x), double(regPts[1].x), double(regPts[2].x), double(regPts[3].x) });
		md::TypedArray<double> yArray = factory.createArray<double>({ 4, 1 },
			{ double(regPts[0].y), double(regPts[1].y), double(regPts[2].y), double(regPts[3].y) });
		unsigned long long imageWidth = width;
		unsigned long long imageHeight = height;
		md::TypedArray<uint8_t> imageArray = factory.createArrayFromBuffer<uint8_t>({ imageHeight, imageWidth }, std::move(imageData), md::MemoryLayout::ROW_MAJOR);
		std::vector<md::Array>params{ imageArray, xArray, yArray };
		std::vector<matlab::data::Array> output = m_matlabLibrary->feval("quickMTF50", 1, params);
		outputData.clear();
		for (auto outputElement : output)
		{
			auto numElements = outputElement.getNumberOfElements();
			size_t numRows = numElements / 3;
			outputData.reserve(numElements + outputData.capacity());
			for (auto nRow = 0; nRow < numRows; nRow++)
			{
				QuickMTF50Data data;
				data.x = outputElement[nRow][0];
				data.y = outputElement[nRow][1];
				data.mtf50 = outputElement[nRow][2];
				outputData.push_back(data);
			}
		}
		success = !outputData.empty();
	}
	catch (matlab::OutOfMemoryException& e)
	{
		std::string s = "matlab::OutOfMemoryException in quickMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidArrayTypeException& e)
	{
		std::string s = "matlab::data::InvalidArrayTypeException in quickMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidMemoryLayoutException& e)
	{
		std::string s = "matlab::data::InvalidMemoryLayoutException in quickMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::InvalidDimensionsInRowMajorArrayException& e)
	{
		std::string s = "matlab::data::InvalidDimensionsInRowMajorArrayException in quickMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::data::NumberOfElementsExceedsMaximumException& e)
	{
		std::string s = "matlab::data::NumberOfElementsExceedsMaximumException in quickMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	catch (matlab::Exception& e)
	{
		std::string s = "matlab::Exception in quickMTF50: ";
		s += e.what();
		LOGMSG_ERROR(s.c_str());
		AfxGetMainWnd()->SetWindowTextA(s.c_str());
		MessageBeep(MB_ICONWARNING);
	}
	return success;
}

