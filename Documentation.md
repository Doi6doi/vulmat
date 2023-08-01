# Vulmat documentation

Vulmat is a simple C library to do basic linear algebra calculations with matrices using GPU. 
Works much faster on large matrices than a CPU solution.

- [Important types](#important-types)
- [Important functions](#important-functions)
- [Less important functions](#less-important-functions)
- [Error codes](#error-codes)
- [Example code](#example-code)

## Important types

- `VmtFloat`: synonim of float. Vulmat uses floats in matrices.
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
void vmt_matrix_sub( VmtMatrix base, VmtMatrix delta )
```
Matrix subtraction. Matrix dimensions must match.
- `base`: base matrix. It will change with `delta`
- `delta`: the matrix which will be subtracted from `base`

---
```c
void vmt_matrix_mult( VmtMatrix left, VmtMatrix right, VmtMatrix result )
```
Matrix multiplication. Matrix dimensions must match.
- `base`: base matrix. It will change with `delta`
- `delta`: the matrix which will be subtracted from `base`



```
Wait for task to terminate.
- `t`: task handle
- `timeoutMsec`: maximum number of milliseconds to wait. If 0, returns immediately
- *returns* true is task ends in `timeoutMsec` or if an error occured

## Less important types

- `VcpStr`: shorthand for const char *
- `VcpFlag`: [initialization flag](#initialization-flags)
- `VcpPart`: struct used to run task for given parts of space
    - `baseX`,`baseY`,`baseZ`: coorindates of starting group
    - `countX`,`countY`,`countZ`: sizes of area to run task on
- `VcpScorer`: function which returns a score for an object to help vulcmp select the best. 
 Larger value is better, less than zero value means object is not suitable, so it wont be selected

## Less important functions

```c
void vcp_check_fail()
```
Checks last error code (*vcp_error*) and terminates program with an error message if it was not *VCP_SUCCESS* or *VCP_TIMEOUT*.

---
```c
void vcp_task_parts( VcpTask t, uint32_t nparts, VcpPart * parts );
```
Setup task to run in parts. Parts will run after each other, with memory barrier between them.
- `t`: task handle
- `nparts`: number of parts to run (count of `parts` array)
- `parts`: the parts to run in given order
---
```c
VcpTask vcp_task_create_file( VcpVulcomp v, VcpStr filename, VcpStr entry, uint32_t nstorage )
```
Create task directly from a file. Calls *vcp_task_create* after reading whole file to memory.
- `v`: GPU handle
- `filename`: filename to load
- `entry`: task entry point
- `nstorage`: number of storage buffers accessed by the task
- `returns` task handle

---
```c
void vcp_select_physical( VcpVulcomp v, VcpScorer s )
```
Selects best physical device for computation.
- `v`: GPU handle
- `s`: scorer function for a device (object will be a pointer to [VkPhysicalDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html))

---
```c
void vcp_select_queue( VcpVulcomp v, VcpScorer s)
```
Selects best queue family for computation.
- `v`: GPU handle
- `s`: scorer function for a queue family (object will be a pointer to [VkQueueFamilyProperties](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFamilyProperties.html))

---
```c
int vcp_physical_score( void * p )
```
Default score function for a physical device. Prefers discrete, intergated, virtual and cpu types in that order.
- `p`: the device to be scored. Pointer to [VkPhysicalDevice](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkPhysicalDevice.html)
- *returns* score of physical device. 

---
```c
int vcp_family_score( void * f )
```
Default score function for a queue family. Accepts only compute queues.
- `f`: the queue family to be scored. Pointer to a [VkQueueFamilyProperties](https://registry.khronos.org/vulkan/specs/1.3-extensions/man/html/VkQueueFamilyProperties.html) structure
- *returns* score of queue famly. 

---
```c
void vcp_storage_free( VcpStorage s )
```
Frees up resources used by `s`. You usually don't need to call this as vulcomp frees up everything on `vcp_done`. It is only needed if you wish to save memory earlier.
- `s`: storage to dispose

---
```c
void vcp_task_free( VcpTask t )
```
Frees up resources used by `t`. You usually don't need to call this as vulcomp frees up everything on `vcp_done`. It is only needed if you wish to save memory earlier.
- `t`: task to dispose

## Initialization flags

The following flags can be used in *vcp_init*

- `VCP_VALIDATION`: program will use [Vulkan validation layer](https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Validation_layers) and show validation errors.
- `VCP_ATOMIC_FLOAT`: request support for atomic float operations

## Error codes

- `VMT_SUCCESS`: last operation terminated successfully.
- `VMT_ALREADY`: vulmat already initialized with `vmt_init`
- `VMT_TASKERR`: error during vulcmp task setup
- `VMT_STORAGEERR`: error during vulcmp storage setup
- `VMT_MATERR`: error during matrix creation
- `VMT_EXECERR`: error during vulcmp task execution
- `VMT_NOMEM`: not enough memory for operation
- `VMT_DIMENSION`: matrix size mismatch in operation

## Example code
   
A vulcmp program more or less would have the following structure:

```c
// initialize GPU
VcpVulcomp v = vcp_init( "vcptest", VCP_VALIDATION );
// allocate GPU memory
VcpStorage s = vcp_storage_create( v, 320*200*4 );
// create task
VcpTask t = vcp_task_create_file( v, "example.spv", "main", 1 );
// initialize task
vcp_task_setup( t, &s, 320, 200, 1 );
// start task
vcp_task_start( t );
// check if all went well
vcp_check_fail();
// wait for termination
vcp_task_wait( t, 1000*60 );
// use result
void * data = vcp_storage_address( s );
consume( data );
// clean up
vcp_done( v );
```
    
