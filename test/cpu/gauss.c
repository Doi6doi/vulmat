#include "vmc.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
   int n = 1500;
   VmcMatrix m = vmc_matrix_create( n, n );
   float * f = vmc_matrix_address( m );
   for ( int c=0; c < n; ++c )
      for ( int r=0; r < n; ++r ) {
         *(f++) = r <= c ? 1.0 : 0.0;
   }
   VmcMatrix result = vmc_matrix_create( n, n );
   vmc_matrix_gauss( m, result, 0 );
   printf("result: \n");
   vmc_matrix_dump( result );
   vmc_matrix_free( result );
   vmc_matrix_free( m );
}
