#include "MatlabTestCode.h"
#include <iostream>
#include <numeric> // for iota

MatlabTestCode::MatlabTestCode()
{
}

MatlabTestCode::~MatlabTestCode()
{

}

bool MatlabTestCode::Initialize()
{
	bool success = false;
	int ret = 0;
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
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (const std::exception& exc) {
		std::string s = exc.what();
		OutputDebugStringA(s.c_str());
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
bool MatlabTestCode::RunTestFullChartMTF50(std::vector<uint16_t>& image, int width, int height, std::vector<FullChartMTF50Data>& outputData)
{
	bool success = false;
	try
	{
		md::ArrayFactory factory;
		size_t imageLength = size_t(width) * size_t(height);
		md::buffer_ptr_t<uint16_t> imageData = factory.createBuffer<uint16_t>(imageLength);
		uint16_t* pImage = imageData.get();
		memcpy(pImage, &image[0], imageLength * sizeof(imageData[0]));
		unsigned long long imageWidth = width;
		unsigned long long imageHeight = height;
		md::TypedArray<uint16_t> imageArray = factory.createArrayFromBuffer<uint16_t>({ imageHeight, imageWidth }, std::move(imageData), md::MemoryLayout::ROW_MAJOR);
		std::vector<md::Array>params{ imageArray };
		std::vector<matlab::data::Array> output = m_matlabLibrary->feval("fullChartMTF50", 1, params);
		for (auto outputElement : output)
		{
			md::ArrayType type = outputElement.getType();
			size_t numElements = outputElement.getNumberOfElements();
			outputData.reserve(numElements + outputData.capacity());
			for (auto nElement = 0; nElement < numElements; nElement++)
			{
				FullChartMTF50Data data;
				data.x = outputElement[0];
				data.y = outputElement[1];
				data.mtf50 = outputElement[3];
				outputData.push_back(data);
			}
		}
		success = !outputData.empty();
	}
	catch (matlab::OutOfMemoryException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidArrayTypeException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidMemoryLayoutException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidDimensionsInRowMajorArrayException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::NumberOfElementsExceedsMaximumException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::Exception& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	return success;
}

//bool MW_CALL_CONV mlxFullChartSNR(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);
//bool MW_CALL_CONV mlfFullChartSNR(int nargout, mxArray** out, mxArray* image);
//fullChartSNR():  expects a 2D, 8 bit array  for image, and returns a 2D array with mean intensity(first column), rms noise (second column) and signal to noise ratio (third column) for all gray boxes.
bool MatlabTestCode::RunTestFullChartSNR(std::vector<uint16_t>& image, int width, int height, std::vector<FullChartSNRData>& outputData)
{
	bool success = false;
	try
	{
		md::ArrayFactory factory;
		size_t imageLength = size_t(width) * size_t(height);
		md::buffer_ptr_t<uint16_t> imageData = factory.createBuffer<uint16_t>(imageLength);
		uint16_t* pImage = imageData.get();
		memcpy(pImage, &image[0], imageLength * sizeof(image[0]));
		unsigned long long imageWidth = width;
		unsigned long long imageHeight = height;
		md::TypedArray<uint16_t> imageArray = factory.createArrayFromBuffer<uint16_t>({ imageHeight, imageWidth }, std::move(imageData), md::MemoryLayout::ROW_MAJOR);
		std::vector<md::Array>params{ imageArray };
		auto output = m_matlabLibrary->feval("fullChartSNR", 1, params);
		size_t numVectorOutput = output.size();
		outputData.clear();
		for (auto outputElement : output)
		{
			size_t numElements = outputElement.getNumberOfElements();
			outputData.reserve(numElements + outputData.capacity());
			for (auto x = 0; x < numElements; x++)
			{
				FullChartSNRData data;
				data.meanIntensity = outputElement[0];
				data.RMSNoise = outputElement[1];
				data.SignalToNoise = outputElement[2];
				outputData.push_back(data);
			}
		}
		success = !outputData.empty();
	}
	catch (matlab::OutOfMemoryException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidArrayTypeException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidMemoryLayoutException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidDimensionsInRowMajorArrayException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::NumberOfElementsExceedsMaximumException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::Exception& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
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
		auto output = m_matlabLibrary->feval("quickMTF50", 1, params);
		outputData.clear();
		for (auto outputElement : output)
		{
			auto elements = outputElement.getNumberOfElements();
			outputData.reserve(elements + outputData.capacity());
			for (auto nElement = 0; nElement < elements; nElement++)
			{
				QuickMTF50Data data;
				data.mtf50 = outputElement[nElement];
				outputData.push_back(data);
			}
		}
		success = !outputData.empty();
	}
	catch (matlab::OutOfMemoryException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidArrayTypeException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidMemoryLayoutException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::InvalidDimensionsInRowMajorArrayException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::data::NumberOfElementsExceedsMaximumException& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	catch (matlab::Exception& e)
	{
		std::string s = e.what();
		OutputDebugStringA(s.c_str());
	}
	return success;
}

