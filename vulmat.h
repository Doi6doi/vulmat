#ifndef VULMATH
#define VULMATH

#include "vulcmp.h"

typedef float VmtFloat;
typedef struct VmtMatrix * VmtMatrix;

#define VMT_SUCCESS         0
#define VMT_HOSTMEM    VCP_HOSTMEM
#define VMT_ALREADY    -20001
#define VMT_TASKERR    -20002
#define VMT_STORAGEERR -20003
#define VMT_MATERR     -20004
#define VMT_EXECERR    -20005
#define VMT_NOMEM      -20006
#define VMT_DIMENSION  -20007

/// vmt last error code
int vmt_error();
/// check result and fail if not ok
void vmt_check_fail();
/// initialize matrix operations
void vmt_init( VcpVulcomp v );
/// terminate matrix operations
void vmt_done();
/// create new matrix
VmtMatrix vmt_matrix_create( uint32_t width, uint32_t height );
/// destroy matrix
void vmt_matrix_free( VmtMatrix );
/// matrix clone
VmtMatrix vmt_matrix_clone( VmtMatrix m );
/// identity matrix
VmtMatrix vmt_matrix_ident( uint32_t size );
/// matrix element address
VmtFloat * vmt_matrix_address( VmtMatrix );
/// matrix width
uint32_t vmt_matrix_width( VmtMatrix );
/// matrix height
uint32_t vmt_matrix_height( VmtMatrix );
/// copy part
void vmt_matrix_copy( VmtMatrix src, uint32_t sx, uint32_t sy,
   uint32_t w, uint32_t h, uint32_t dx, uint32_t dy, VmtMatrix dst );
/// addition
void vmt_matrix_add( VmtMatrix base, VmtMatrix delta );
/// transpose
void vmt_matrix_transpose( VmtMatrix m, VmtMatrix result );
/// subtraction
void vmt_matrix_sub( VmtMatrix base, VmtMatrix delta );
/// multiplication
void vmt_matrix_mult( VmtMatrix left, VmtMatrix right, VmtMatrix result );
/// permutate rows
void vmt_matrix_rows( VmtMatrix m, uint32_t * indices, VmtMatrix result );
/// gauss inversion
bool vmt_matrix_gauss( VmtMatrix m, VmtMatrix result, float epsilon );
/// dump data
void vmt_matrix_dump( VmtMatrix m );


#endif // VULMATH
