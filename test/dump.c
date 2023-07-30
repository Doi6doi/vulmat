#include "vulmat.h"
#include "stdio.h"

void dump( VmtMatrix m ) {
   int w = vmt_matrix_width( m );
   int h = vmt_matrix_height( m );
   VmtFloat * f = vmt_matrix_address( m );
   for ( int r=0; r < h; ++r ) {
      for ( int c=0; c < w; ++c ) {
         if ( 0 < c )
            printf( "\t" );
         printf( "%.4g", *f );
         ++f;
      }
      printf( "\n" );
   }
}
