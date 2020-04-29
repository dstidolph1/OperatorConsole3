//
// MATLAB Compiler: 8.0 (R2020a)
// Date: Mon Apr 27 12:59:46 2020
// Arguments:
// "-B""macro_default""-W""cpplib:MatlabMTF50Lib_2,all,version=1.0""-T""link:lib
// ""-d""C:\Users\ArnoldEstrada\Dropbox\Fino_Files\Professional Life\Eye
// Lock\Projects\Lens Focus Test
// Fixture\MatlabMTF50Lib_2\for_testing""-v""C:\Users\ArnoldEstrada\Dropbox\Fino
// _Files\Professional Life\Eye Lock\Projects\Lens Focus Test
// Fixture\fullChartMTF50.m""C:\Users\ArnoldEstrada\Dropbox\Fino_Files\Professio
// nal Life\Eye Lock\Projects\Lens Focus Test
// Fixture\fullChartSNR.m""C:\Users\ArnoldEstrada\Dropbox\Fino_Files\Professiona
// l Life\Eye Lock\Projects\Lens Focus Test Fixture\quickMTF50.m"
//

#ifndef MatlabMTF50Lib_2_h
#define MatlabMTF50Lib_2_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#include "mclcppclass.h"
#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_MatlabMTF50Lib_2_C_API 
#define LIB_MatlabMTF50Lib_2_C_API /* No special import/export declaration */
#endif

/* GENERAL LIBRARY FUNCTIONS -- START */

extern LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV MatlabMTF50Lib_2InitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV MatlabMTF50Lib_2Initialize(void);

extern LIB_MatlabMTF50Lib_2_C_API 
void MW_CALL_CONV MatlabMTF50Lib_2Terminate(void);

extern LIB_MatlabMTF50Lib_2_C_API 
void MW_CALL_CONV MatlabMTF50Lib_2PrintStackTrace(void);

/* GENERAL LIBRARY FUNCTIONS -- END */

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV mlxFullChartMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV mlxFullChartSNR(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV mlxQuickMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

#ifdef __cplusplus
}
#endif


/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

#ifdef __cplusplus

/* On Windows, use __declspec to control the exported API */
#if defined(_MSC_VER) || defined(__MINGW64__)

#ifdef EXPORTING_MatlabMTF50Lib_2
#define PUBLIC_MatlabMTF50Lib_2_CPP_API __declspec(dllexport)
#else
#define PUBLIC_MatlabMTF50Lib_2_CPP_API __declspec(dllimport)
#endif

#define LIB_MatlabMTF50Lib_2_CPP_API PUBLIC_MatlabMTF50Lib_2_CPP_API

#else

#if !defined(LIB_MatlabMTF50Lib_2_CPP_API)
#if defined(LIB_MatlabMTF50Lib_2_C_API)
#define LIB_MatlabMTF50Lib_2_CPP_API LIB_MatlabMTF50Lib_2_C_API
#else
#define LIB_MatlabMTF50Lib_2_CPP_API /* empty! */ 
#endif
#endif

#endif

extern LIB_MatlabMTF50Lib_2_CPP_API void MW_CALL_CONV fullChartMTF50(int nargout, mwArray& out, const mwArray& image);

extern LIB_MatlabMTF50Lib_2_CPP_API void MW_CALL_CONV fullChartSNR(int nargout, mwArray& out, const mwArray& image);

extern LIB_MatlabMTF50Lib_2_CPP_API void MW_CALL_CONV quickMTF50(int nargout, mwArray& out, const mwArray& image, const mwArray& X_coords, const mwArray& Y_coords);

/* C++ INTERFACE -- WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */
#endif

#endif
