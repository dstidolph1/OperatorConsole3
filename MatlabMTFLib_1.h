/*
 * MATLAB Compiler: 8.0 (R2020a)
 * Date: Tue Apr 21 11:50:56 2020
 * Arguments:
 * "-B""macro_default""-W""lib:MatlabMTFLib_1,version=1.0""-T""link:lib""-d""C:\
 * Users\ArnoldEstrada\Dropbox\Fino_Files\Professional Life\Eye
 * Lock\Projects\Lens Focus Test
 * Fixture\MatlabMTFLib_1\for_testing""-v""C:\Users\ArnoldEstrada\Dropbox\Fino_F
 * iles\Professional Life\Eye Lock\Projects\Lens Focus Test
 * Fixture\fullChartMTF50.m""C:\Users\ArnoldEstrada\Dropbox\Fino_Files\Professio
 * nal Life\Eye Lock\Projects\Lens Focus Test
 * Fixture\fullChartSNR.m""C:\Users\ArnoldEstrada\Dropbox\Fino_Files\Professiona
 * l Life\Eye Lock\Projects\Lens Focus Test Fixture\quickMTF50.m"
 */

#ifndef MatlabMTFLib_1_h
#define MatlabMTFLib_1_h 1

#if defined(__cplusplus) && !defined(mclmcrrt_h) && defined(__linux__)
#  pragma implementation "mclmcrrt.h"
#endif
#include "mclmcrrt.h"
#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_MatlabMTFLib_1_C_API 
#define LIB_MatlabMTFLib_1_C_API /* No special import/export declaration */
#endif

/* GENERAL LIBRARY FUNCTIONS -- START */

extern LIB_MatlabMTFLib_1_C_API 
bool MW_CALL_CONV MatlabMTFLib_1InitializeWithHandlers(
       mclOutputHandlerFcn error_handler, 
       mclOutputHandlerFcn print_handler);

extern LIB_MatlabMTFLib_1_C_API 
bool MW_CALL_CONV MatlabMTFLib_1Initialize(void);

extern LIB_MatlabMTFLib_1_C_API 
void MW_CALL_CONV MatlabMTFLib_1Terminate(void);

extern LIB_MatlabMTFLib_1_C_API 
void MW_CALL_CONV MatlabMTFLib_1PrintStackTrace(void);

/* GENERAL LIBRARY FUNCTIONS -- END */

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_MatlabMTFLib_1_C_API 
bool MW_CALL_CONV mlxFullChartMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_MatlabMTFLib_1_C_API 
bool MW_CALL_CONV mlxFullChartSNR(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

extern LIB_MatlabMTFLib_1_C_API 
bool MW_CALL_CONV mlxQuickMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[]);

/* C INTERFACE -- MLX WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

/* C INTERFACE -- MLF WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- START */

extern LIB_MatlabMTFLib_1_C_API bool MW_CALL_CONV mlfFullChartMTF50(int nargout, mxArray** out, mxArray* image);

extern LIB_MatlabMTFLib_1_C_API bool MW_CALL_CONV mlfFullChartSNR(int nargout, mxArray** out, mxArray* image);

extern LIB_MatlabMTFLib_1_C_API bool MW_CALL_CONV mlfQuickMTF50(int nargout, mxArray** out, mxArray* image, mxArray* X_coords, mxArray* Y_coords);

#ifdef __cplusplus
}
#endif
/* C INTERFACE -- MLF WRAPPERS FOR USER-DEFINED MATLAB FUNCTIONS -- END */

#endif
