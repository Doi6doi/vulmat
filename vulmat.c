#include "vulmat.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "vulmat_comp.h"

#define REALLOC( p, type, n ) (type *)realloc( p, (n)*sizeof(type) )
#define MAIN "main"
#define TICK 1000
#define GDC( x ) (((x)+UGR-1)/UGR)
#define TASK( name, nstor, conf ) \
   static VcpTask vmt_##name() { \
      if ( ! vmtVulmat.name ) { \
		 vmtVulmat.name = vcp_task_create( vmtVulmat.vulcomp, \
		    name##_spv, name##_spv_len, MAIN, nstor, sizeof( conf )); \
	  } \
      return vmtVulmat.name; \
   }

#include "ident.inc"
#include "copy.inc"
#include "add.inc"
#include "transpose.inc"
#include "gauss.inc"
#include "rows.inc"
#include "mult.inc"

int vmtResult = VMT_SUCCESS;

typedef struct VmtGaussPars * VmtGaussPars;
typedef struct VmtGaussData * VmtGaussData;

struct VmtVulmat {
   bool started;
   VcpVulcomp vulcomp;
   uint32_t nmat;
   VmtMatrix * mats;
   uint32_t ngpars;
   VmtGaussPars gpars;
   VcpStorage temp;
   VcpStorage temp2;

   VcpTask ident;
   VcpTask copy;
   VcpTask add;
   VcpTask transpose;
   VcpTask gauss;
   VcpTask rows;
   VcpTask mult;
};

struct VmtVulmat vmtVulmat = {
   .started = false
};

struct VmtMatrix {
   struct VmtDims d;
   VcpStorage stor;
};

TASK( ident, 1, struct VmtDims )
TASK( copy, 2, struct VmtCopyPars )
TASK( mult, 3, struct VmtMultPars )
TASK( add, 2, struct VmtDims )
TASK( transpose, 2, struct VmtDims )
TASK( rows, 3, struct VmtDims )
TASK( gauss, 3, struct VmtGaussPars )

void vmt_init( VcpVulcomp v ) {
   vmtResult = VMT_ALREADY;
   if ( vmtVulmat.started )
      return;
   vmtVulmat.vulcomp = v;
   vmtVulmat.nmat = 0;
   vmtVulmat.mats = NULL;
   vmtVulmat.ngpars = 0;
   vmtVulmat.gpars = NULL;
   vmtVulmat.started = true;
   vmtVulmat.temp = NULL;
   vmtVulmat.temp2 = NULL;

   vmtVulmat.ident = NULL;
   vmtVulmat.copy = NULL;
   vmtVulmat.add = NULL;
   vmtVulmat.transpose = NULL;
   vmtVulmat.gauss = NULL;
   vmtVulmat.rows = NULL;
   vmtVulmat.mult = NULL;

   vmtResult = VMT_SUCCESS;
}

void vmt_check_fail() {
   if ( VMT_SUCCESS != vmtResult ) {
      fprintf( stderr, "Error: vulmat %d\n", vmtResult );
      exit(1);
   }
}

/// task felszámolása
static void vmt_task_done( VcpTask t ) {
   if ( NULL == t ) return;
   vcp_task_free( t );
}

void vmt_done() {
   vmtResult = VMT_SUCCESS;
   if ( ! vmtVulmat.started )
      return;
   vmt_task_done( vmtVulmat.ident );
   vmt_task_done( vmtVulmat.copy );
   vmt_task_done( vmtVulmat.add );
   vmt_task_done( vmtVulmat.transpose );
   vmt_task_done( vmtVulmat.gauss );
   vmt_task_done( vmtVulmat.rows );
   vmt_task_done( vmtVulmat.mult );

   vmtVulmat.gpars = REALLOC( vmtVulmat.gpars, struct VmtGaussPars, 0 );
   vmtVulmat.started = false;
   
   vcp_storage_free( vmtVulmat.temp );
   vcp_storage_free( vmtVulmat.temp2 );
}

/// ideiglenes memória növelése, ha szükséges
static bool vmt_temp_grow( uint32_t count ) {
   uint64_t sz = sizeof(VmtFloat)*count;
   if ( vmtVulmat.temp 
         && sz <= vcp_storage_size( vmtVulmat.temp ))
      return true;
   if ( vmtVulmat.temp )
      vcp_storage_free( vmtVulmat.temp );
   vmtVulmat.temp = vcp_storage_create( vmtVulmat.vulcomp, sz );
   vmtResult = vcp_error();
   return vmtVulmat.temp;
}
      
/// második ideiglenes memória növelése, ha szükséges
static bool vmt_temp2_grow( uint32_t count ) {
   uint64_t sz = sizeof(VmtFloat)*count;
   if ( vmtVulmat.temp2 
         && sz <= vcp_storage_size( vmtVulmat.temp2 ))
      return true;
   if ( vmtVulmat.temp2 )
      vcp_storage_free( vmtVulmat.temp2 );
   vmtVulmat.temp2 = vcp_storage_create( vmtVulmat.vulcomp, sz );
   vmtResult = vcp_error();
   return vmtVulmat.temp2;
}
      

VmtMatrix vmt_matrix_create( uint32_t width, uint32_t height ) {
   vmtResult = VMT_NOMEM;
   VmtMatrix m = REALLOC( NULL, struct VmtMatrix, 1 );
   if ( ! m ) return NULL;
   m->d.width = width;
   m->d.height = height;
   vmtResult = VMT_STORAGEERR;
   uint64_t sz1 = (uint64_t)width*height*sizeof(VmtFloat);
   if ( ! (m->stor = vcp_storage_create( vmtVulmat.vulcomp, sz1 )))
      return NULL;
   vmtResult = VMT_NOMEM;
   VmtMatrix * mats = REALLOC( vmtVulmat.mats, VmtMatrix, vmtVulmat.nmat+1 );
   if ( ! mats ) return NULL;
   mats[ vmtVulmat.nmat++ ] = m;
   vmtVulmat.mats = mats;
   vmtResult = VMT_SUCCESS;
   return m;
}

/// legalább n db config lefoglalása
static bool vmt_gpars_grow( uint32_t n ) {
   vmtResult = VMT_HOSTMEM;
   if ( vmtVulmat.ngpars < n ) {
	  VmtGaussPars ret = REALLOC( vmtVulmat.gpars, struct VmtGaussPars, n);
	  if ( ! ret ) return false;
	  vmtVulmat.gpars = ret;
   }
   vmtResult = VMT_SUCCESS;
   return true;
}

static void vmt_gpart( VcpPart parts, VmtGaussPars def, 
   uint32_t i, uint32_t countX, uint32_t countY )
{
   VcpPart p = parts+i;
   VmtGaussPars g = vmtVulmat.gpars+i;
   g->size = def->size;
   g->epsilon = def->epsilon;
   if ( 0 == i ) {
	  g->phase = 0;
	  g->method = 0;
   } else {
	  g->phase = (i-1)/4;
	  g->method = i % 4;
   }
   p->countX = countX;
   p->countY = countY;
   p->countZ = 1;
   p->constants = g;
}

/// set gauss parts
static bool vmt_gauss_setup( VcpStorage * ss, uint32_t n, float epsilon ) {
   if ( ! vmt_gpars_grow( 4*n+1 )) return false;   
   vmtResult = VMT_TASKERR;
   VcpTask t = vmt_gauss();
   if ( ! t ) return false;
   vcp_task_setup( t, ss, 0, 0, 0, NULL ); 
   struct VcpPart * p = vcp_task_parts( t, 4*n+1 );
   if ( ! p ) return false;
   struct VmtGaussPars d = { .size = n, .epsilon = epsilon };
   vmt_gpart( p, &d, 0, GDC(n), GDC(n) );
   for ( int i=0; i<n; ++i ) {
      int j = 4*i;
      // search + inc
      vmt_gpart( p, &d, j+1, 1, 1 );
      // swap
      vmt_gpart( p, &d, j+2, GDC(2*n-i), 1 );
      // change
      vmt_gpart( p, &d, j+3, GDC(2*n-i), GDC(n) );
      // changeCol
      vmt_gpart( p, &d, j+4, 1, GDC(n) );
   }
   vmtResult = VMT_SUCCESS;
   return true;
}

/// task futtatása
static bool vmt_run( VcpTask t ) {
   vmtResult  = VMT_TASKERR;
   vcp_task_start( t );
   while ( ! vcp_task_wait( t, TICK ))
      ;
   if ( vcp_error() ) return false;
   vmtResult = VMT_SUCCESS;
   return true;
}	

/// task beállítása és futtatása
static bool vmt_exec( VcpTask t, VcpStorage * stors, uint32_t nx, uint32_t ny,
   void * pars ) 
{
   vmtResult  = VMT_TASKERR;
   if ( ! t ) return false;
   vcp_task_setup( t, stors, GDC(nx), GDC(ny), 1, pars );
   return vmt_run( t );
}

VmtMatrix vmt_matrix_ident( uint32_t size ) {
   vmtResult = VMT_MATERR;
   VmtMatrix m = vmt_matrix_create( size, size );
   if ( ! m ) return NULL;
   if ( ! vmt_exec( vmt_ident(), & m->stor, size, size, &m->d )) return NULL;
   return m;
}

// copy part
void vmt_matrix_copy( VmtMatrix src, uint32_t sx, uint32_t sy,
   uint32_t w, uint32_t h, uint32_t dx, uint32_t dy, VmtMatrix dst )
{
   vmtResult = VMT_DIMENSION;
   if ( src->d.width < sx+w
      || src->d.height < sy+h
      || dst->d.width < dx+w
      || dst->d.height < dy+h
   )
      return;
   struct VmtCopyPars pars = { .width=w, .height=h, .sx=sx, .sy=sy, 
	  .sw=src->d.width, .dx=dx, .dy=dy, .dw=dst->d.width };
   VcpStorage ss[2] = { src->stor, dst->stor };
   vmt_exec( vmt_copy(), ss, w, h, &pars );
}

void vmt_matrix_add( VmtMatrix base, VmtMatrix delta ) {
   vmtResult = VMT_DIMENSION;
   if ( base->d.width != delta->d.width
      || base->d.height != delta->d.height )
      return;
   VcpStorage ss[2] = { base->stor, delta->stor };
   vmt_exec( vmt_add(), ss, base->d.width, base->d.height, &base->d );
}

void vmt_matrix_mult( VmtMatrix left, VmtMatrix right, VmtMatrix result ) {
   vmtResult = VMT_DIMENSION;
   if ( left->d.width != right->d.height
      || left->d.height != result->d.height
      || right->d.width != result->d.width )
      return;
   struct VmtMultPars pars = { .width = result->d.width,
	  .height=result->d.height, .axis = left->d.width };
   VcpStorage ss[3] = { left->stor, right->stor, result->stor };
   vmt_exec( vmt_mult(), ss, pars.width, pars.height, &pars );
}



VmtMatrix vmt_matrix_clone( VmtMatrix m ) {
   VmtMatrix ret = vmt_matrix_create( m->d.width, m->d.height );
   if ( ! ret ) return NULL;
   uint32_t sz = m->d.width * m->d.height * sizeof(VmtFloat);
   vcp_storage_copy( m->stor, ret->stor, 0, 0, sz );
   return ret;
}

void vmt_matrix_transpose( VmtMatrix m, VmtMatrix result ) {
   vmtResult = VMT_DIMENSION;
   if ( m->d.width != result->d.height
      || m->d.height != result->d.width )
      return;
   VcpStorage ss[2] = { m->stor, result->stor };
   vmt_exec( vmt_transpose(), ss, m->d.width, m->d.height, &m->d );
}

void vmt_matrix_dump( VmtMatrix m ) {
   float *g = (float*)vcp_storage_address( m->stor );
   for (int r = 0; r < m->d.height; ++r ) {
      for ( int c = 0; c < m->d.width; ++c )
         printf("%.4g\t", g[ r*m->d.width+c ] );
      printf("\n" );
   }
   printf("\n");
}

void vmt_matrix_rows( VmtMatrix m, uint32_t * indices, VmtMatrix result ) {
   vmtResult = VMT_DIMENSION;
   uint32_t mw = m->d.width;
   uint32_t mh = m->d.height;
   uint32_t rh = result->d.height;
   if ( mw != result->d.width ) return;
   for ( int i=0; i < rh; ++i )
      if ( mh <= indices[i] ) return; 
   if ( ! vmt_temp_grow( rh )) return;
   memcpy( vcp_storage_address( vmtVulmat.temp ), indices, rh*sizeof(uint32_t));
   VcpStorage ss[3] = { vmtVulmat.temp, m->stor, result->stor };
   vmt_exec( vmt_rows(), ss, result->d.width, result->d.height, &result->d );
}


bool vmt_matrix_gauss( VmtMatrix m, VmtMatrix result, float epsilon ) {
   vmtResult = VMT_DIMENSION;
   uint32_t n = m->d.width;
   if ( n != m->d.height || n != result->d.width || n != result->d.height )
      return false;
   if ( ! vmt_temp_grow( n*n )) return false;
   vcp_storage_copy( m->stor, vmtVulmat.temp, 0, 0, n*n*sizeof(VmtFloat) );
   if ( ! vmt_temp2_grow( sizeof( struct VmtGaussData )/4 ) ) return false;
   VmtGaussData data = vcp_storage_address( vmtVulmat.temp2 );
   data->quit = 0;
   VcpStorage ss[3] = { vmtVulmat.temp2, vmtVulmat.temp, result->stor }; 
   if ( ! vmt_gauss_setup( ss, n, epsilon )) return false;
   if ( ! vmt_run( vmt_gauss() )) return false;
   data = vcp_storage_address( vmtVulmat.temp2 );
   return ! data->quit;
}

VmtFloat * vmt_matrix_address( VmtMatrix m ) {
   return (VmtFloat *)vcp_storage_address( m->stor );
}

/// remove matrix from list
static void vmt_matrix_remove( VmtMatrix m ) {
   VmtMatrix * mm = vmtVulmat.mats;
   int n = vmtVulmat.nmat;
   for ( int i = n-1; 0 <= i; --i ) {
      if (mm[i] == m ) {
         mm[i] = mm[n-1];
         vmtVulmat.mats = REALLOC( mm, VmtMatrix, n-1 );
         -- vmtVulmat.nmat;
         return;
      }
   }
}

void vmt_matrix_free( VmtMatrix m ) {
   vmt_matrix_remove( m );
   vcp_storage_free( m->stor );
   m = REALLOC( m, struct VmtMatrix, 0 );
   vmtResult = VMT_SUCCESS;
}


uint32_t vmt_matrix_width( VmtMatrix m ) {
   return m->d.width;
}

uint32_t vmt_matrix_height( VmtMatrix m ) {
   return m->d.height;
}


