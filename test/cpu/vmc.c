
#include "vmc.h"
#include <stdlib.h>
#include <stdio.h>

#define VMC_REALLOC( p, type, n ) (type *)realloc( p, (n)*sizeof(type) )

typedef struct VmcMatrix__ {
   uint32_t width;
   uint32_t height;
   float * data;
} VmcMatrix_;


VmcMatrix vmc_matrix_create( uint32_t w, uint32_t h ) {
   VmcMatrix ret = VMC_REALLOC( NULL, VmcMatrix_, 1 );
   ret->width = w;
   ret->height = h;
   ret->data = VMC_REALLOC( NULL, float, w*h );
   return ret;
}

float * vmc_matrix_address( VmcMatrix m ) {
   return m->data;
}

void vmcg_ident( VmcMatrix m ) {
   float * f = m->data;
   for (int r=0; r < m->width; ++r ) {
      for (int c=0; c < m->height; ++c )
         *(f++) = r==c ? 1.0: 0.0;
   }
}


int vmcg_search( VmcMatrix m, int phase, float epsilon ) {
   int n = m->height;
   int x = phase;
   for (int i=x; i<n; ++i) {
      float p = m->data[i*n+x];
      if (epsilon < abs(p))
         return i;
   }
   return -1;
}



/// swap best row to phaseth and divide
void vmcg_swap( VmcMatrix base, VmcMatrix result, int phase, int best ) {
   int n = base->width;
   int y = phase;
   float pivot = base->data[ best*n+y ];
   for ( int x=phase+1; x<n; ++x ) {
      if ( best == y ) {
         base->data[ y*n+x ] /= pivot;
      } else {
         float save = base->data[y*n+x];
         base->data[y*n+x] = base->data[best*n+x] / pivot;
         base->data[best*n+x] = save;
      }
   }
   for ( int j = 0; j<n; ++j ) {
      if ( best == y ) {
         result->data[ y*n+j ] /= pivot;
      } else {
         float save = result->data[y*n+j];
         result->data[y*n+j] = result->data[best*n+j]/pivot;
         result->data[best*n+j] = save;
      }
   }
}

/// change most elements
void vmcg_change( VmcMatrix base, VmcMatrix result, int phase ) {
   int n = base->height;
   int p = phase;
   for ( int y=0; y<n; ++y) {
      if ( p != y ) {
         float q = base->data[ y*n+p ];
         for ( int x=p+1; x<n; ++x )
            base->data[ y*n+x ] -= base->data[ p*n+x ]*q;
         for ( int j=0; j<n; ++j )
            result->data[ y*n+j ] -= result->data[ p*n+j ]*q;
      }
   }
}

/// change phaseth col elements
void vmcg_changeCol( VmcMatrix base, int phase ) {
   int n = base->height;
   int x = phase;
   for ( int y=0; y < n; ++y )
      base->data[ y*n+x ] = y == x ? 1.0 : 0.0;
}



bool vmc_matrix_gauss( VmcMatrix m, VmcMatrix result, float epsilon ) {
   uint32_t n = m->width;
   if ( n != m->height || n != result->width || n != result->height )
      return false;
   VmcMatrix base = vmc_matrix_clone(m);
   vmcg_ident( result );
   int phase = 0;
   for ( ; phase < n; ++phase ) {
      int best = vmcg_search( base, phase, epsilon );
      if ( 0 > best )
         break;
      vmcg_swap(base, result, phase, best);
      vmcg_change(base, result, phase);
      vmcg_changeCol(base, phase);
   }
   vmc_matrix_free( base );
   return n == phase;
}


void vmc_matrix_dump( VmcMatrix m ) {
   int w = m->width;
   int h = m->height;
   float * f = vmc_matrix_address( m );
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

void vmc_matrix_free( VmcMatrix m ) {
   m->data = VMC_REALLOC( m->data, float, 0 );
   m = VMC_REALLOC( m, VmcMatrix_, 0 );
}

VmcMatrix vmc_matrix_clone( VmcMatrix m ) {
   VmcMatrix ret = vmc_matrix_create( m->width, m->height );
   float *f = vmc_matrix_address( m );
   float *g = vmc_matrix_address( ret );
   int nn = m->width * m->height;
   for ( int i=0; i<nn; ++i)
      *(g++) = *(f++);
   return ret;
}



