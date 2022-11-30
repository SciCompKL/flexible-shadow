#ifndef FLEXIBLE_SHADOW_MOCKSTDLIB_HPP
#define FLEXIBLE_SHADOW_MOCKSTDLIB_HPP

#error "flexible-shadow-mockstdlib.hpp is only meant for documentation purposes, do not #include it!"

/*! \struct MockStandardLibraryInterface
 * Sample collection of standard library-like functions 
 * used by ShadowMap objects, for documentation purposes.
 * Do not use this class or header in your code!
 */
struct MockStandardLibraryInterface {
  /*! Memory allocation.
   *
   * If memory allocation fails, this function must terminate the
   * program. 
   * 
   * \param size Number of bytes to be allocated.
   * \returns Pointer to allocated block.
   */
  static void* safe_malloc(unsigned long long size) { return nullptr; }

  /*! Memory deallocation.
   * \param ptr Pointer to block of memory allocated by safe_malloc.
   */
  static void free(void* ptr) {}
  
  /*! Memory copy.
   * \param dst Destination, memory to copy to.
   * \param src Source, memory to copy from.
   * \param size Number of bytes to be copied.
   * \returns Destination.
   */
  static void* memcpy(void* dst, const void* src, unsigned long long size){}

};

#endif
