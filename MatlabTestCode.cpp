#include "MatlabTestCode.h"
#include "MatlabMTFLib_1.h"
#include "matrix.h"
#include "MatlabMTFLib_1.h"

MatlabTestCode::MatlabTestCode()
{
}

MatlabTestCode::~MatlabTestCode()
{

}

bool MatlabTestCode::Initialize()
{
	bool success = MatlabMTFLib_1Initialize();
	return false;
}


//bool MW_CALL_CONV mlxFullChartMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);
//bool MW_CALL_CONV mlfFullChartMTF50(int nargout, mxArray** out, mxArray* image);
//fullChartMTF50():  expects a 2D, 8 bit array  for image, and returns a 2D array with xcoordinates (first column), y coordinates (second column) and MTF50 vals (third column) for all visible edges.
bool MatlabTestCode::RunTestFullChartMTF50(std::vector<uint8_t>& image, int width, int height, std::vector<CPoint>& coordinates, std::vector<int>& outputMTF50)
{
	return false;
}

//bool MW_CALL_CONV mlxFullChartSNR(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);
//bool MW_CALL_CONV mlfFullChartSNR(int nargout, mxArray** out, mxArray* image);
//fullChartSNR():  expects a 2D, 8 bit array  for image, and returns a 2D array with mean intensity(first column), rms noise (second column) and signal to noise ratio (third column) for all gray boxes.
bool MatlabTestCode::RunTestFullChartSNR(std::vector<uint8_t>& image, int width, int height, std::vector<float>& outputMeanIntensity, std::vector<float>& outputRMSNoise, std::vector<float>& outputSignalToNoise)
{
	return false;
}

//bool MW_CALL_CONV mlxQuickMTF50(int nlhs, mxArray* plhs[], int nrhs, mxArray* prhs[]);
//bool MW_CALL_CONV mlfQuickMTF50(int nargout, mxArray** out, mxArray* image, mxArray* X_coords, mxArray* Y_coords);
//quickMTF50():  expects a 2D, 8 bit array  for image, an array of 4 x coordinates and an array of 4 ycoordinates.  I returns array of 5 MTF50 vals (edges 12,27,32,33,50).
bool MatlabTestCode::RunTestQuickMTF50(std::vector<uint8_t>& image, int width, int height, std::vector<CPoint>& outputCoordinates, std::vector<int>& outputMTF50)
{
	try
	{
		mxArray* out = NULL;
		mxArray* imageParam = mxCreateNumericMatrix(height, width, mxUINT8_CLASS, mxREAL);  // Argument 6 - Image Data
		memcpy(mxGetPr(imageParam), &image[0], image.size());
		mwSize numElements[1] = { 4 };
		mxArray* xArray = mxCreateNumericArray(1, &numElements[0], mxINT32_CLASS, mxREAL);
		mxArray* yArray = mxCreateNumericArray(1, &numElements[0], mxINT32_CLASS, mxREAL);
		int *pXArray = (int*)mxGetPr(xArray);
		int* pYArray = (int*)mxGetPr(yArray);
		pXArray[0] = outputCoordinates[0].x;
		pYArray[0] = outputCoordinates[0].y;
		pXArray[1] = outputCoordinates[1].x;
		pYArray[1] = outputCoordinates[1].y;
		pXArray[2] = outputCoordinates[2].x;
		pYArray[2] = outputCoordinates[2].y;
		pXArray[3] = outputCoordinates[3].x;
		pYArray[3] = outputCoordinates[3].y;

		bool result = mlfQuickMTF50(5, &out, imageParam, xArray, yArray);
		printf("Result=%d\n", (int)result);
	}
	catch (mwException& e)
	{
		printf("Exception %s\n", e.what());
		e.print_stack_trace();
	}
	return false;
}

