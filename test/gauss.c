#include "vulmat.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
   int n = 1000;
   VcpVulcomp v = vcp_init( "vmtgauss", VCP_VALIDATION );
//   VcpVulcomp v = vcp_init( "vmtgauss", 0 );
   vmt_init( v );
   VmtMatrix m = vmt_matrix_ident( n );
   VmtFloat * f = vmt_matrix_address( m );
   for ( int r=0; r < n; ++r ) {
      for ( int c=0; c < n; ++c )
         *(f++) = r <= c ? 1.0 : 0.0;
   }
   VmtMatrix result = vmt_matrix_create(n,n);
   bool ret = vmt_matrix_gauss( m, result, 0 );
   if ( ret ) {
      printf("result: \n");
      vmt_matrix_dump( result );
   } else {
      printf("not invertible\n");
   }
   vmt_done();
   vcp_done(v);
}
