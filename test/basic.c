#include "dump.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
   VcpVulcomp v = vcp_init( "vmtident", VCP_VALIDATION | VCP_ATOMIC_FLOAT );
   vmt_init( v );
   VmtMatrix m = vmt_matrix_ident( 5 );
   VmtFloat * f = vmt_matrix_address( m );
   for (int i=0; i<5; ++i ) {
      f[i*5+i] = 0;
      f[i*5+(4-i)] = i;
   }
//   VmtMatrix m2 = vmt_matrix_create( 5, 5 );
//   dump( m3 );
//   vmt_matrix_gauss( m, m2 );
   dump( m );
   vmt_done();
   vcp_done(v);
}
