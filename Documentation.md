# Vulmat documentation

Vulmat is a simple C library to do basic linear algebra calculations with matrices using GPU. 
Works much faster on large matrices than a CPU solution.

- [Important types](#important-types)
- [Important functions](#important-functions)
- [Less important functions](#less-important-functions)
- [Error codes](#error-codes)
- [Example code](#example-code)

## Important types

- `VmtFloat`: synonym of float. Vulmat uses floats in matrices.
- `VmtMatrix`: opaque type handle for a GPU matrix

## Important functions

```c
    int vmt_error()
```
Returns [error code](#error-codes) of last call. 0 means no error.

---
```c
    void vmt_init( VcpVulcomp v ) 
```
Initialize vulmat system.
- `v`: already initialized vulcomp instance
---
```c
void vmt_done()
```
End of vulmat usage. Also frees all created matrices.

---
```c
VmtMatrix vmt_matrix_create( uint32_t width, uint32_t height )
```
Create new matrix.
- `width`: matrix width
- `height`: matrix height
- *returns* handle to new matrix or NULL if an error occurs

---
```c
VmtFloat * vmt_matrix_address( VmtMatrix m );
```
Pointer to matrix elements. A matrix has `width`*`height` elements.
- `m`: matrix handle
- *returns* pointer to elements

---
```c
uint32_t vmt_matrix_width( VmtMatrix m );
```
Width of matrix
- `m`: matrix handle
- *returns* width of matrix

---
```c
uint32_t vmt_matrix_height( VmtMatrix )
```
Height of matrix
- `m`: matrix handle
- *returns* height of matrix

---
```c
void vmt_matrix_copy( VmtMatrix src, uint32_t sx, uint32_t sy, uint32_t w, uint32_t h, uint32_t dx, uint32_t dy, VmtMatrix dst )
```
Copy matrix part to other matrix. Source area must fit in source matrix. Destination area must fit in destination matrix. 
If source and destination is the same, the areas must not overlap.
- `src`: source matrix handle
- `sx`: source area x corrdinate
- `sy`: source area y corrdinate
- `w`: width of area to copy
- `h`: height of area to copy
- `dx`: destination area x coordinate
- `dy`: destination area y coordinate
- `dst`: destination matrix handle

---
```c
void vmt_matrix_add( VmtMatrix base, VmtMatrix delta )
```
Matrix addition. Matrix dimensions must match.
- `base`: base matrix. It will change with `delta`
- `delta`: the matrix which will be added to `base`

---
```c
void vmt_matrix_mult( VmtMatrix left, VmtMatrix right, VmtMatrix result )
```
Matrix multiplication. The following values must be equal:
    `left.width` = `right.height`
    `left.height` = `result.height`
    `right.width` = `result.width`
- `left`: left matrix of multiplication
- `right`: right matrix of multiplication
- `result`: result matrix

---
```c
bool vmt_matrix_gauss( VmtMatrix m, VmtMatrix result, float epsilon )
```
Matrix inversion with gauss elimination. 
- `m`: matrix to find inverse for
- `result`: inverse matrix if found
- `epsilon`: float precision can result in false inverses. A positive epsilon (for comparision to zero) can exclude those.
- *returns* true if inverse is found

## Less important functions

```c
void vmt_check_fail()
```
Checks last error code (*vmt_error*) and terminates program with an error message if it was not *VMT_SUCCESS*.

---
```c
void vmt_matrix_free( VmtMatrix m )
```
Frees resources used by `m`. You usually don't need to call this as vulmat frees up everything on `vmt_done`. It is only needed if you wish to save memory earlier.
- `m`: matrix to dispose

---
```c
VmtMatrix vmt_matrix_clone( VmtMatrix m )
```
Create a clone of `m`.
- `m`: matric to clone
- *returns* clone of `m`

---
```c
VmtMatrix vmt_matrix_ident( uint32_t size )
```
Create an identitiy matrix.
- `size`: width and height of matrix
- *returns* identity matrix

---
```c
void vmt_matrix_transpose( VmtMatrix m, VmtMatrix result )
```
Transposes a matrix.
- `m`: original matrix
- `result`: transposed matrix

---
```c
void vmt_matrix_rows( VmtMatrix m, uint32_t * indices, VmtMatrix result )
```
Permutate rows of matrix
- `m`: original matrix
- `indices`: indices of matrix rows (with m.height items)
- `result`: matrix with permutated rows

---
```c
void vmt_matrix_dump( VmtMatrix m )
```
Prints matrix to standard output
- `m`: matrix to print

## Error codes

- `VMT_SUCCESS`: last operation terminated successfully.
- `VMT_HOSTMEM`: host memory allocation error
- `VMT_ALREADY`: vulmat already initialized with `vmt_init`
- `VMT_TASKERR`: error during vulcmp task setup
- `VMT_STORAGEERR`: error during vulcmp storage setup
- `VMT_MATERR`: error during matrix creation
- `VMT_EXECERR`: error during vulcmp task execution
- `VMT_NOMEM`: not enough memory for operation
- `VMT_DIMENSION`: matrix size mismatch in operation

## Example code

A simple vulmat program example:

```c
// initialize GPU
VcpVulcomp v = vcp_init( "vmttest", VCP_VALIDATION );
// initialize vulmat
vmt_init( v );
// create identity matrix
VmtMatrix m = vmt_matrix_ident( 5 );
// get elements address
VmtFloat * f = vmt_matrix_address( m );
// set an element
f[ 2*5+1 ] = 2.0;
// dump matrix
vmt_matrix_dump( m );
// finalize
vmt_done();
vcp_done(v);
```

