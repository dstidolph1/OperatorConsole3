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

#define EXPORTING_MatlabMTF50Lib_2 1
#include "MatlabMTF50Lib_2.h"

static HMCRINSTANCE _mcr_inst = NULL; /* don't use nullptr; this may be either C or C++ */

#if defined( _MSC_VER) || defined(__LCC__) || defined(__MINGW64__)
#ifdef __LCC__
#undef EXTERN_C
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define NOMINMAX
#include <windows.h>
#undef interface

static char path_to_dll[_MAX_PATH];

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, void *pv)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        if (GetModuleFileName(hInstance, path_to_dll, _MAX_PATH) == 0)
            return FALSE;
    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
    }
    return TRUE;
}
#endif
#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

static int mclDefaultPrintHandler(const char *s)
{
    return mclWrite(1 /* stdout */, s, sizeof(char)*strlen(s));
}

#ifdef __cplusplus
} /* End extern C block */
#endif

#ifdef __cplusplus
extern "C" { // sbcheck:ok:extern_c
#endif

static int mclDefaultErrorHandler(const char *s)
{
    int written = 0;
    size_t len = 0;
    len = strlen(s);
    written = mclWrite(2 /* stderr */, s, sizeof(char)*len);
    if (len > 0 && s[ len-1 ] != '\n')
        written += mclWrite(2 /* stderr */, "\n", sizeof(char));
    return written;
}

#ifdef __cplusplus
} /* End extern C block */
#endif

/* This symbol is defined in shared libraries. Define it here
 * (to nothing) in case this isn't a shared library. 
 */
#ifndef LIB_MatlabMTF50Lib_2_C_API
#define LIB_MatlabMTF50Lib_2_C_API /* No special import/export declaration */
#endif

LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV MatlabMTF50Lib_2InitializeWithHandlers(
    mclOutputHandlerFcn error_handler,
    mclOutputHandlerFcn print_handler)
{
    int bResult = 0;
    if (_mcr_inst)
        return true;
    if (!mclmcrInitialize())
        return false;
    if (!GetModuleFileName(GetModuleHandle("MatlabMTF50Lib_2"), path_to_dll, _MAX_PATH))
        return false;
    {
        mclCtfStream ctfStream = 
            mclGetEmbeddedCtfStream(path_to_dll);
        if (ctfStream) {
            bResult = mclInitializeComponentInstanceEmbedded(&_mcr_inst,
                                                             error_handler, 
                                                             print_handler,
                                                             ctfStream);
            mclDestroyStream(ctfStream);
        } else {
            bResult = 0;
        }
    }  
    if (!bResult)
    return false;
    return true;
}

LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV MatlabMTF50Lib_2Initialize(void)
{
    return MatlabMTF50Lib_2InitializeWithHandlers(mclDefaultErrorHandler, 
                                                mclDefaultPrintHandler);
}

LIB_MatlabMTF50Lib_2_C_API 
void MW_CALL_CONV MatlabMTF50Lib_2Terminate(void)
{
    if (_mcr_inst)
        mclTerminateInstance(&_mcr_inst);
}

LIB_MatlabMTF50Lib_2_C_API 
void MW_CALL_CONV MatlabMTF50Lib_2PrintStackTrace(void) 
{
    char** stackTrace;
    int stackDepth = mclGetStackTrace(&stackTrace);
    int i;
    for(i=0; i<stackDepth; i++)
    {
        mclWrite(2 /* stderr */, stackTrace[i], sizeof(char)*strlen(stackTrace[i]));
        mclWrite(2 /* stderr */, "\n", sizeof(char)*strlen("\n"));
    }
    mclFreeStackTrace(&stackTrace, stackDepth);
}


LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV mlxFullChartMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[])
{
    return mclFeval(_mcr_inst, "fullChartMTF50", nlhs, plhs, nrhs, prhs);
}

LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV mlxFullChartSNR(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[])
{
    return mclFeval(_mcr_inst, "fullChartSNR", nlhs, plhs, nrhs, prhs);
}

LIB_MatlabMTF50Lib_2_C_API 
bool MW_CALL_CONV mlxQuickMTF50(int nlhs, mxArray *plhs[], int nrhs, mxArray *prhs[])
{
    return mclFeval(_mcr_inst, "quickMTF50", nlhs, plhs, nrhs, prhs);
}

LIB_MatlabMTF50Lib_2_CPP_API 
void MW_CALL_CONV fullChartMTF50(int nargout, mwArray& out, const mwArray& image)
{
    mclcppMlfFeval(_mcr_inst, "fullChartMTF50", nargout, 1, 1, &out, &image);
}

LIB_MatlabMTF50Lib_2_CPP_API 
void MW_CALL_CONV fullChartSNR(int nargout, mwArray& out, const mwArray& image)
{
    mclcppMlfFeval(_mcr_inst, "fullChartSNR", nargout, 1, 1, &out, &image);
}

LIB_MatlabMTF50Lib_2_CPP_API 
void MW_CALL_CONV quickMTF50(int nargout, mwArray& out, const mwArray& image, const 
                             mwArray& X_coords, const mwArray& Y_coords)
{
    mclcppMlfFeval(_mcr_inst, "quickMTF50", nargout, 1, 3, &out, &image, &X_coords, &Y_coords);
}

