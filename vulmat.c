#include "vulmat.h"
#include "stdlib.h"
#include "stdio.h"

#define VMT_REALLOC( p, type, n ) (type *)realloc( p, (n)*sizeof(type) )
#define VMT_MAIN "main"
#define VMT_TICK 1000
#define VMT_GROUP 16
#define VMT_GDF( x ) ((x)/VMT_GROUP)
#define VMT_GDC( x ) (((x)+VMT_GROUP-1)/VMT_GROUP)
#define VMT_TASK( s, n ) \
	tdd[ vt_##s ] = s##_spv; \
        tdl[ vt_##s ] = s##_spv_len; \
        tds[ vt_##s ] = n;

typedef enum Vmt_Task { vt_ident, vt_copy, vt_add, vt_clone, vt_transpose,
   vt_gauss, vt_rows } VmtTask;

#define vt_first vt_ident
#define vt_last vt_rows

unsigned char * tdd[ vt_last+1 ];
uint32_t tdl[ vt_last+1 ];
uint32_t tds[ vt_last+1 ];

#include "ident.inc"
#include "copy.inc"
#include "add.inc"
#include "clone.inc"
#include "transpose.inc"
#include "gauss.inc"
#include "rows.inc"

void vmt_taskdata_init() {
   VMT_TASK( ident, 2 );
   VMT_TASK( copy, 3 );
   VMT_TASK( add, 3 );
   VMT_TASK( clone, 3 );
   VMT_TASK( transpose, 3 );
   VMT_TASK( gauss, 3 );
   VMT_TASK( rows, 3 );
}

int vmtResult = VMT_SUCCESS;

typedef struct Vmt__Vulmat {
   bool started;
   VcpVulcomp vulcomp;
   uint32_t nmat;
   VmtMatrix * mats;
   VcpTask tasks[ vt_last+1 ];
} Vmt_Vulmat;


Vmt_Vulmat vmtVulmat = {
   .started = false
};

typedef struct Vmt__Matrix {
   uint32_t width, height;
   VcpStorage sDims;
   VcpStorage sVals;
} Vmt_Matrix;


void vmt_init( VcpVulcomp v ) {
   vmtResult = VMT_ALREADY;
   if ( vmtVulmat.started )
      return;
   vmt_taskdata_init();
   vmtVulmat.vulcomp = v;
   vmtVulmat.nmat = 0;
   vmtVulmat.mats = NULL;
   vmtVulmat.started = true;
   vmtResult = VMT_SUCCESS;
}

void vmt_check_fail() {
   if ( VMT_SUCCESS != vmtResult ) {
      fprintf( stderr, "Error: vulmat %d\n", vmtResult );
      exit(1);
   }
}

void vmt_done() {
   vmtResult = VMT_SUCCESS;
   if ( ! vmtVulmat.started )
      return;
   for ( VmtTask vt = vt_first; vt <= vt_last; ++vt ) {
      VcpTask t = vmtVulmat.tasks[vt];
      if ( t )
         vcp_task_free( t );
      vmtVulmat.tasks[vt] = NULL;
   }
   vmtVulmat.started = false;
}

VmtMatrix vmt_matrix_create( uint32_t width, uint32_t height ) {
   vmtResult = VMT_NOMEM;
   VmtMatrix m = VMT_REALLOC( NULL, Vmt_Matrix, 1 );
   if ( ! m ) return NULL;
   uint64_t sz0 = 2*sizeof(uint32_t);
   vmtResult = VMT_STORAGEERR;
   VcpStorage s0 = vcp_storage_create( vmtVulmat.vulcomp, sz0 );
   if ( ! s0 ) return NULL;
   m->sDims = s0;
   uint32_t * pu = (uint32_t *)vcp_storage_address( s0 );
   pu[0] = m->width = width;
   pu[1] = m->height = height;
   uint64_t sz1 = (uint64_t)width*height*sizeof(VmtFloat);
   if ( ! (m->sVals = vcp_storage_create( vmtVulmat.vulcomp, sz1 )))
      return NULL;
   vmtResult = VMT_NOMEM;
   VmtMatrix * mats = VMT_REALLOC( vmtVulmat.mats, VmtMatrix, vmtVulmat.nmat+1 );
   if ( ! mats ) return NULL;
   mats[ vmtVulmat.nmat++ ] = m;
   vmtVulmat.mats = mats;
   vmtResult = VMT_SUCCESS;
   return m;
}

/// set gauss parts
bool vmt_gauss_parts( VcpTask t, uint32_t n ) {
   vmtResult = VMT_NOMEM;
   VcpPart * p = VMT_REALLOC( NULL, VcpPart, 4*n+1 );
   if ( ! p ) return false;
   VcpPart pid = { VMT_GDF(n),0,0, VMT_GDC(n)+1,n,1 };
   VcpPart psr = { 0,0,1, 1,1,1 };
   VcpPart psw = { 0,0,2, VMT_GDC(2*n), 1, 1};
   VcpPart pch = { 0,0,3, VMT_GDC(2*n), n, 1};
   VcpPart pcc = { 0,0,4, VMT_GDC(n), 1, 1};
   p[0] = pid;
   for ( int i=0; i<n; ++i ) {
      int j = 4*i;
      // search + inc
      p[j+1] = psr;
      // swap
      p[j+2] = psw;
      // change
      p[j+3] = pch;
      // changeCol
      p[j+4] = pcc;
      // not all coordinates needed
      p[j+2].baseX = p[j+3].baseX = p[j+4].baseX = VMT_GDF( i );
      p[j+2].countX = p[j+3].countX = VMT_GDC( 2*n-i )+1;
   }
   vmtResult = VMT_EXECERR;
   vcp_task_parts( t, 4*n+1, p );
   p = VMT_REALLOC( p, VcpPart, 0 );
   if ( VCP_SUCCESS != vcp_error() )
      return false;
   vmtResult = VMT_SUCCESS;
   return true;
}


/// setup tasks
bool vmt_setup( VmtTask vt, VcpTask t, VcpStorage * ss, uint32_t w, uint32_t h ) {
   vcp_task_setup( t, ss, VMT_GDC(w), VMT_GDC(h), 1 );
   if ( VCP_SUCCESS != vcp_error() )
      return false;
   if ( vt_gauss == vt )
      return vmt_gauss_parts( t, w );
   return true;
}

/// execute a task
void vmt_exec( VmtTask vt, VcpStorage * ss, uint32_t w, uint32_t h ) {
   vmtResult = VMT_TASKERR;
   VcpTask t = vmtVulmat.tasks[vt];
   if ( ! t ) {
      t = vcp_task_create( vmtVulmat.vulcomp, tdd[vt], tdl[vt], VMT_MAIN, tds[vt] );
      if ( ! t ) return;
      vmtVulmat.tasks[vt] = t;
   }
   if ( ! vmt_setup( vt, t, ss, w, h )) return;
   vmtResult = VMT_EXECERR;
   vcp_task_start( t );
   while ( ! vcp_task_wait( t, VMT_TICK ))
      ;
   if ( ! vcp_error() )
      vmtResult = VMT_SUCCESS;
}


VmtMatrix vmt_matrix_ident( uint32_t size ) {
   vmtResult = VMT_MATERR;
   VmtMatrix m = vmt_matrix_create( size, size );
   if ( ! m )
      return NULL;
   VcpStorage ss[2] = { m->sDims, m->sVals };
   vmt_exec( vt_ident, ss, size, size );
   return m;
}

// copy part
void vmt_matrix_copy( VmtMatrix src, uint32_t sx, uint32_t sy,
   uint32_t w, uint32_t h, uint32_t dx, uint32_t dy, VmtMatrix dst )
{
   vmtResult = VMT_DIMENSION;
   if ( sx+w > src->width
      || sy+h > src->height
      || dx+w > dst->width
      || dy+h > dst->height
   )
      return;
   VcpStorage s = vcp_storage_create( vmtVulmat.vulcomp,
      sizeof( uint32_t)*10 );
   uint32_t * f = vcp_storage_address( s );
   f[0] = src->width;
   f[1] = src->height;
   f[2] = sx;
   f[3] = sy;
   f[4] = w;
   f[5] = h;
   f[6] = dst->width,
   f[7] = dst->height,
   f[8] = dx;
   f[9] = dy;
   VcpStorage ss[3] = { s, src->sVals, dst->sVals };
   vmt_exec( vt_copy, ss, w, h );
   vcp_storage_free( s );
}


void vmt_matrix_add( VmtMatrix base, VmtMatrix delta ) {
   vmtResult = VMT_DIMENSION;
   if ( base->width != delta->width
      || base->height != delta->height )
      return;
   VcpStorage ss[3] = { base->sDims, base->sVals, delta->sVals };
   vmt_exec( vt_add, ss, base->width, base->height );
}


VmtMatrix vmt_matrix_clone( VmtMatrix m ) {
   VmtMatrix ret = vmt_matrix_create( m->width, m->height );
   if ( ! ret )
      return NULL;
   VcpStorage ss[3] = { m->sDims, m->sVals, ret->sVals };
   vmt_exec( vt_clone, ss, m->width, m->height );
   return ret;
}

void vmt_matrix_transpose( VmtMatrix m, VmtMatrix result ) {
   vmtResult = VMT_DIMENSION;
   if ( m->width != result->height
      || m->height != result->width )
      return;
   VcpStorage ss[3] = { m->sDims, m->sVals, result->sVals };
   vmt_exec( vt_transpose, ss, m->width, m->height );
}

void vmt_matrix_dump( VmtMatrix m ) {
   float *g = (float*)vcp_storage_address( m->sVals );
   for (int r = 0; r < m->height; ++r ) {
      for ( int c = 0; c < m->width; ++c )
         printf("%.4g\t", g[ r*m->width+c ] );
      printf("\n" );
   }
   printf("\n");
}

void vmt_matrix_rows( VmtMatrix m, uint32_t * indices, VmtMatrix result ) {
   vmtResult = VMT_DIMENSION;
   uint32_t mw = m->width;
   uint32_t mh = m->height;
   uint32_t rh = result->height;
   if ( mw != result->width )
      return;
   for ( int i=0; i < rh; ++i ) {
      if ( indices[i] >= mh )
         return;
   }
   VcpStorage s = vcp_storage_create( vmtVulmat.vulcomp,
      sizeof(uint32_t)*(rh+2) );
   uint32_t * f = vcp_storage_address( s );
   f[0] = mw;
   f[1] = rh;
   VcpStorage ss[3] = { s, m->sVals, result->sVals };
   vmt_exec( vt_rows, ss, mw, rh );
}


bool vmt_matrix_gauss( VmtMatrix m, VmtMatrix result, float epsilon ) {
   vmtResult = VMT_DIMENSION;
   uint32_t n = m->width;
   if ( n != m->height || n != result->width || n != result->height )
      return false;
   VmtMatrix h = vmt_matrix_clone(m);
   VcpStorage sh = vcp_storage_create( vmtVulmat.vulcomp,
      sizeof(uint32_t)*4+sizeof(float)*2 );
   uint32_t * f = vcp_storage_address( sh );
   f[0] = n;
   f[1] = 0;
   f[2] = -1;
   f[3] = -1;
   *(float *)(f+4) = epsilon;
   *(float *)(f+8) = 0.0;
   VcpStorage ss[3] = { sh, h->sVals, result->sVals };
   vmt_exec( vt_gauss, ss, n, n );
   f = vcp_storage_address( sh );
   vmt_matrix_free( h );
   bool ret = ! f[1];
   vcp_storage_free( sh );
   return ret;
}


VmtFloat * vmt_matrix_address( VmtMatrix m ) {
   return (VmtFloat *)vcp_storage_address( m->sVals );
}


/// remove matrix from list
static void vmt_matrix_remove( VmtMatrix m ) {
   VmtMatrix * mm = vmtVulmat.mats;
   int n = vmtVulmat.nmat;
   for ( int i = n-1; 0 <= i; --i ) {
      if (mm[i] == m ) {
         mm[i] = mm[n-1];
         vmtVulmat.mats = VMT_REALLOC( mm, VmtMatrix, n-1 );
         -- vmtVulmat.nmat;
         return;
      }
   }
}

void vmt_matrix_free( VmtMatrix m ) {
   vmt_matrix_remove( m );
   vcp_storage_free( m->sDims );
   vcp_storage_free( m->sVals );
   m = VMT_REALLOC( m, Vmt_Matrix, 0 );
   vmtResult = VMT_SUCCESS;
}


uint32_t vmt_matrix_width( VmtMatrix m ) {
   return m->width;
}

uint32_t vmt_matrix_height( VmtMatrix m ) {
   return m->height;
}


