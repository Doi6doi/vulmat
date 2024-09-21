#ifndef VULMAT_COMPH
#define VULMAT_COMPH

#define UGR 16

#ifdef VULKAN

#define UINT uint

#else

#define UINT uint32_t

#endif

#define FLOAT float

struct VmtDims {
   UINT width;
   UINT height;
};

struct VmtCopyPars {
   UINT width;
   UINT height;
   UINT sw;
   UINT sx;
   UINT sy;
   UINT dw;
   UINT dx;
   UINT dy;
};

struct VmtMultPars {
   UINT width;
   UINT height;
   UINT axis;
};

struct VmtGaussPars {
   UINT size;
   UINT phase;
   UINT method;
   FLOAT epsilon;
};

struct VmtGaussData {
   UINT quit;
   UINT best;
   FLOAT pivot;
};

#endif // VULMAT_COMPH
