// Translate FilterMeister language expressions to C
// This needs to go before CtlDef, to redefine RGB.
#include "components/filtermeister/FW_CTL_to_C.h"


#define plugInVendorName    "eNoises" // This is the category
#define plugInAETEComment   "Worley Noise filter"
#define plugInSuiteID       'Worley Noise'
#define plugInClassID       plugInSuiteID
#define plugInEventID       plugInClassID
#define plugInCopyrightYear "2023"
#define plugInDescription   "Worley Noise"
#define plugInVersion       "1.00"

// Plugin Name ---------------------------------------------------------
#define plugInName  "eWorley"
#define plugInUniqueID  "_eWorley" //For actions recording

// Image Modes and Enable Info -----------------------------------------
#define plugInSupportedModes doesSupportBitmap, doesSupportGrayScale, doesSupportIndexedColor, \
                             doesSupportRGBColor, doesSupportCMYKColor, \
                             doesSupportHSLColor, doesSupportHSBColor, doesSupportMultichannel, \
                             doesSupportDuotone, doesSupportLABColor
#define plugInEnableInfo "in (PSHOP_ImageMode, RGBMode, GrayScaleMode, CMYKMode, HSLMode, HSBMode, MultichannelMode, DuotoneMode, LabMode, RGB48Mode, Gray16Mode)"

// About Screen Message and Version Numbers ----------------------------
char* AboutMessage(void) {
    //EXTERNC char * AboutMessage(void) {
#if _WIN64
    return "\n\n\n\nVersion " plugInVersion " (64-bit)";
#else
    return "\n\n\n\nVersion " plugInVersion " (32-bit)";
#endif
}

//******************************************************************
//  Global Variables (mostly constants)
//******************************************************************

// Controls (values < 250)
const int CTL_SCANOPACITY = 5;
const int CTL_SCANWIDTH = 1;

// Common Controls (values < 130)
// const int CTL_PROGRESSIMG = 43;
// const int CTL_URLBUTTON = 49;
// const int CTL_PROGRESSBAR = 72;
const int CTL_ZOOMIN = 74;
const int CTL_ZOOMOUT = 75;
const int CTL_ZOOMCOMBO = 76;

//******************************************************************
// Control Definitions (Runs only once, before filter dialog shows)
//******************************************************************

CtlDef{

    // Do we need T-buffers? Only allocate if needed
    need_tbuf = false;
    need_t2buf = false;
    need_t3buf = false;
    need_t4buf = false;


    //**************************************************************
    //  Dialog
    //**************************************************************

    // Set Title
    dialogtitle(plugInName)
    dialogsize(414, 222)
    dialogminmax(414, 222, 0, 0)

    PosClientCENTER

    //**********************************************************
    //  Controls
    //**********************************************************

    ctl(CTL_OK) pushbutton() actionapply() text("OK") pos(359, 205) size(50, 12) anchor(ANCHOR_RIGHT|ANCHOR_BOTTOM) scripting(false)

    ctl(CTL_CANCEL) pushbutton() actioncancel() text("Cancel") pos(300,205) size(50,12) anchor(ANCHOR_RIGHT | ANCHOR_BOTTOM) scripting(false)

    // Trying to modify the default size of the preview
    ctl(CTL_PREVIEW) pos(5,5) size(229,196)

    // Trying to remove / delete standard controls
    ctl(CTL_EDIT) invisible()
    ctl(CTL_LOGO) invisible()
    ctl(CTL_ZOOM) invisible()
    ctl(CTL_PROGRESS) invisible()
    ctl(CTL_FRAME) invisible()

    ctl(CTL_SCANOPACITY) slider()
        text("Distance")
        pos(284,57) size(-1,-1) range(0,100) val(25) defval(25) actionpreview() scripting(true)
    setCustomCtl(0,0);

    ctl(CTL_SCANWIDTH) slider()
        text("Width")
        pos(284,77) range(1,100) val(2) defval(2) actionpreview() scripting(true)
    setCustomCtl(1,0);
    ctl(2) slider() range(1,100)
    pos(284,97)
    
    ctl(CTL_ZOOMIN) pushbutton() text("+") actionnone() size(11,13) pos(179,205) anchor(ANCHOR_LEFT | ANCHOR_BOTTOM) scripting(false)
    ctl(CTL_ZOOMCOMBO) combobox()
        text("3200%\n1600%\n1200%\n800%\n700%\n600%\n500%\n400%\n300%\n200%\n---------------\n100%\n50%\n33%\n25%\n20%\n16%\n14%\n12%\n11%\n10%\n9%\n8%\n7%\n6%\n---------------\nFit")
        val(26) defval(26) actionnone() size(33,120) pos(190,205) anchor(ANCHOR_LEFT | ANCHOR_BOTTOM) scripting(false)
    ctl(CTL_ZOOMOUT) pushbutton() text("-") actionnone() size(11,13) pos(223,205) anchor(ANCHOR_LEFT | ANCHOR_BOTTOM) scripting(false)

    return true;
}

// This needs to go here (after CtlDef, before everything else)
// in order to redefine CTL, val, INVALID_HANDLE_VALUE and NULL.
// Translate FilterMeister language expressions to C
#include "components/include/CL/cl.h"
#include "components/filtermeister/FW_FFP_to_C.h"



//******************************************************************
//  Window Start Handler (executed only once after CtlDef)
//******************************************************************

OnWindowStart {
    /*Nepovtornoto povikuvanje na call_FFP() gi resetira
    goleminite na kontrolnite palci na 1.
    the second parameter must be the value of the thumb image width
    for proper bitmap image display.*/
    setCtlThumbSize(0, 11);
    // setCtlThumbSize(1, 6);
    // setCtlThumbSize(2, 6);
    return true;
}

//******************************************************************
// Control Handlers (executed for UI events)
//******************************************************************

OnCtl(n) {

    if (e == FME_EXITSIZE) {
        if (doingProxy && ctl(CTL_ZOOMCOMBO) == 26) {
            setZoom(-888, 1);
        }
    }

    //////////////////////////////////////////////////////////////
    ///  Keypress Intercept
    //////////////////////////////////////////////////////////////

    if (e == FME_KEYDOWN) {
        if (n == VK_ADD) {
            if (getAsyncKeyStateF(VK_CONTROL) <= -32767) {
                triggerEvent(CTL_ZOOMIN, FME_CLICKED, 0);
                return true;
            }
        }
        else if (n == VK_SUBTRACT) {
            if (getAsyncKeyStateF(VK_CONTROL) <= -32767) {
                triggerEvent(CTL_ZOOMOUT, FME_CLICKED, 0);
                return true;
            }
        }
    }

    //////////////////////////////////////////////////////////////
    ///  Zoom Control Handlers
    //////////////////////////////////////////////////////////////

    if (n == CTL_ZOOMCOMBO) {
        return evalZoomCombo(n, CTL_ZOOMIN, CTL_ZOOMOUT, previous);
    }
    else if ((n == CTL_ZOOMIN || n == CTL_ZOOMOUT)) {
        return evalZoomButtons(n, CTL_ZOOMCOMBO, CTL_ZOOMIN, CTL_ZOOMOUT);
    }

    return false;
}

//*******************************************************************
// Filter Start Handler (Executed before ForEveryTile for every
// preview update and the final rendering)
//******************************************************************

OnFilterStart{
    return true;
}

//******************************************************************
// Self declared function -> Do not use the names of FFP/FML
// functions to avoid problems
//******************************************************************


static cl_context clCtx;
static cl_command_queue clQueue;
static cl_program clProgram;
static cl_kernel clKernel;
static cl_device_id clDevice;
static int opencl_initialized = 0;

unsigned short* gpuOutput = NULL;
int totalcpu = 0;

void InitOpenCL() {
    cl_platform_id platform;
    cl_uint numPlatforms;
    cl_int err;

    clGetPlatformIDs(1, &platform, &numPlatforms);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &clDevice, NULL);
    clCtx = clCreateContext(NULL, 1, &clDevice, NULL, NULL, &err);
    clQueue = clCreateCommandQueue(clCtx, clDevice, 0, &err);

    const char* src =
"__kernel void worleyNoise(__global ushort* out, int w, int h, uint seed, int cellSize) {\n"
"    int x = get_global_id(0);\n"
"    int y = get_global_id(1);\n"
"    if (x >= w || y >= h) return;\n"
"\n"
"    int cellsX = (w + cellSize - 1) / cellSize;\n"
"    int cellsY = (h + cellSize - 1) / cellSize;\n"
"\n"
"    float minDist = 1e9f;\n"
"\n"
"    for (int dy = -1; dy <= 1; dy++) {\n"
"        for (int dx = -1; dx <= 1; dx++) {\n"
"            int cx = (x / cellSize) + dx;\n"
"            int cy = (y / cellSize) + dy;\n"
"\n"
"            if (cx < 0 || cx >= cellsX || cy < 0 || cy >= cellsY) continue;\n"
"\n"
"            uint hash = (uint)((cx + seed) * 374761393u + (cy + seed) * 668265263u);\n"
"            hash = (hash ^ (hash >> 13)) * 1274126177u;\n"
"            float fx = (float)(cx * cellSize + (hash % cellSize));\n"
"            float fy = (float)(cy * cellSize + ((hash / cellSize) % cellSize));\n"
"\n"
"            float dx = (float)x - fx;\n"
"            float dy = (float)y - fy;\n"
"            float dist = dx*dx + dy*dy;\n"
"            if (dist < minDist) minDist = dist;\n"
"        }\n"
"    }\n"
"\n"
"    float maxDist = (float)(cellSize * cellSize * 2);\n"
"    ushort val = (ushort)((1.0f - (minDist / maxDist)) * 0x7FFF);\n"
"\n"
"    int idx = (y * w + x) * 3;\n"
"    out[idx + 0] = val;\n"
"    out[idx + 1] = val;\n"
"    out[idx + 2] = val;\n"
"}\n";

    clProgram = clCreateProgramWithSource(clCtx, 1, &src, NULL, &err);
    clBuildProgram(clProgram, 0, NULL, NULL, NULL, NULL);
    clKernel = clCreateKernel(clProgram, "worleyNoise", &err);

    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    totalcpu = sysinfo.dwNumberOfProcessors;
    if (totalcpu < 1) totalcpu = 1;

    opencl_initialized = 1;
}

//******************************************************************
// Filter Rendering with multilple threads (Executed from
// ForEveryTile with doMultiThreading())
//******************************************************************

unsigned __stdcall threadProc(void* arg) {
    FMcontext* const fmc = gFmcp;
    int thread_id = *(int*)arg;
    int width = x_end - x_start;
    int height = y_end - y_start;

    int rows_per_thread = height / totalcpu;
    int _y0 = y_start + thread_id * rows_per_thread;
    int _y1 = (thread_id == totalcpu - 1) ? y_end : _y0 + rows_per_thread;

    for (int y = _y0; y < _y1; y++) {
        for (int x = x_start; x < x_end; x++) {
            int idx = ((y - y_start) * width + (x - x_start)) * 3;
            if (idx >= 0 && idx + 2 < width * height * 3) {
                pset(x, y, 0, gpuOutput[idx + 0]); // R
                pset(x, y, 1, gpuOutput[idx + 1]); // G
                pset(x, y, 2, gpuOutput[idx + 2]); // B
            }
        }
    }

    _endthreadex(0);
    return 0;
}

void RunOpenCL_GPU_and_CPU() {
    FMcontext* const fmc = gFmcp;

    if (!opencl_initialized) InitOpenCL();

    int width = x_end - x_start;
    int height = y_end - y_start;
    size_t global_size[2] = { width, height };
    cl_int err;

    cl_mem outBuffer = clCreateBuffer(clCtx, CL_MEM_WRITE_ONLY, width * height * 3 * sizeof(unsigned short), NULL, &err);
    if (err != CL_SUCCESS || !outBuffer) return;

    UINT seed = (UINT)time(NULL);
    int cellSize = ctl(5); // You can make this user-controlled later
    if (cellSize < 1) cellSize = 1; // Prevent divide-by-zero

    clSetKernelArg(clKernel, 0, sizeof(cl_mem), &outBuffer);
    clSetKernelArg(clKernel, 1, sizeof(int), &width);
    clSetKernelArg(clKernel, 2, sizeof(int), &height);
    clSetKernelArg(clKernel, 3, sizeof(UINT), &seed);
    clSetKernelArg(clKernel, 4, sizeof(int), &cellSize);


    clEnqueueNDRangeKernel(clQueue, clKernel, 2, NULL, global_size, NULL, 0, NULL, NULL);
    clFinish(clQueue);

    if (gpuOutput) free(gpuOutput);
    gpuOutput = malloc(width * height * 3 * sizeof(unsigned short));
    if (!gpuOutput) return;

    clEnqueueReadBuffer(clQueue, outBuffer, CL_TRUE, 0, width * height * 3 * sizeof(unsigned short), gpuOutput, 0, NULL, NULL);
    clReleaseMemObject(outBuffer);

    HANDLE* threads = (HANDLE*)malloc(sizeof(HANDLE) * totalcpu);
    int* ids = (int*)malloc(sizeof(int) * totalcpu);

    for (int i = 0; i < totalcpu; i++) {
        ids[i] = i;
        threads[i] = (HANDLE)_beginthreadex(NULL, 0, threadProc, &ids[i], 0, NULL);
    }
    WaitForMultipleObjects(totalcpu, threads, TRUE, INFINITE);
    for (int i = 0; i < totalcpu; i++) CloseHandle(threads[i]);

    free(threads);
    free(ids);
}

//*******************************************************************
// ForEveryTile Handler (executed for every preview update and the final rendering)
//*******************************************************************

ForEveryTile{

    //Do any memory or array allocation here and not in threadProc();
    //...
RunOpenCL_GPU_and_CPU();
//Free memory or arrays here ...
//...

return true;

}




//******************************************************************
// OnFilterEnd Handler (Executed after ForEveryTile for every preview
// update and the final rendering)
//******************************************************************

OnFilterEnd {
    // Reset controls
    updateProgress(0, 100);      // Reset the progress bar.
    return false;
}



//******************************************************************
// OnFilterExit Handler (Executed after Cancel or after full rendering with OK)
//******************************************************************

OnFilterExit {
    if (gpuOutput) { free(gpuOutput); gpuOutput = NULL; }
    if (opencl_initialized) {
        clReleaseKernel(clKernel);
        clReleaseProgram(clProgram);
        clReleaseCommandQueue(clQueue);
        clReleaseContext(clCtx);
        opencl_initialized = 0;
    }
    return;
}

