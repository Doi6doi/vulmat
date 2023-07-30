#ifndef VMCH
#define VMCH

#include <stdbool.h>
#include <stdint.h>

typedef struct VmcMatrix__ * VmcMatrix;

VmcMatrix vmc_matrix_create( uint32_t w, uint32_t h );

float * vmc_matrix_address( VmcMatrix );

VmcMatrix vmc_matrix_clone( VmcMatrix m );

bool vmc_matrix_gauss( VmcMatrix m, VmcMatrix result, float epsilon );

void vmc_matrix_dump( VmcMatrix );

void vmc_matrix_free( VmcMatrix );

#endif // VMCH
