#include <pub_tool_mallocfree.h> // VG_(malloc), VG_(free)
#include <pub_tool_libcprint.h> // VG_(printf)
#include <pub_tool_libcassert> // tl_assert

struct ValgrindStandardLibraryInterface {

  static void* safe_malloc(unsigned long long size){
    void* ptr = VG_(malloc)("Memory allocation by flexible-shadow", size);
    if(!ptr){
      VG_(printf)("flexible-shadow: Error allocating memory in safe_malloc.\n");
      tl_assert(0);
    } 
    return ptr;
  }

  static void free(void* ptr){
    VG_(free)(ptr);
  }

};
