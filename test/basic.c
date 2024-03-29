#include "vulmat.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
   VcpVulcomp v = vcp_init( "vmtident", VCP_VALIDATION );
   vmt_init( v );
   VmtFloat *f;
   VmtMatrix m = vmt_matrix_ident( 5 );
   f = vmt_matrix_address( m );
   for (int i=0; i<5; ++i ) {
      f[i*5+i] = 0;
      f[i*5+(4-i)] = i;
   }
   VmtMatrix n = vmt_matrix_create( 2, 5 );
   f = vmt_matrix_address( n );
   for (int r=0; r<5; ++r ) {
      for (int c=0; c<2; ++c )
         f[r*2+c] = (r+c) % 2;
   }
   VmtMatrix o = vmt_matrix_create( 2, 5 );
   vmt_matrix_mult( m, n, o );
   VmtMatrix p = vmt_matrix_create( 5, 5 );
   uint32_t indices [] = { 0,1,4,4,3 };
   vmt_matrix_rows( m, indices, p );
   vmt_matrix_dump( m );
   vmt_matrix_dump( n );
   vmt_matrix_dump( o );
   vmt_matrix_dump( p );
   vmt_done();
   vcp_done(v);
}
