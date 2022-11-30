#ifndef FLEXIBLE_SHADOW_DEFAULTSTDLIB_HPP
#define FLEXIBLE_SHADOW_DEFAULTSTDLIB_HPP

#include <cstdlib> // malloc, free
#include <cstring> // memcpy
#include <iostream> // std::cerr, std::endl

struct DefaultStandardLibraryInterface {

  static void* safe_malloc(unsigned long long size){
    void* ptr = malloc(size);
    if(!ptr){
      std::cerr << "flexible-shadow: Error allocating memory in safe_malloc." << std::endl;
      exit(1);
    } 
    return ptr;
  }

  static void free(void* ptr){
    ::free(ptr);
  }

  static void* memcpy(void* dst, const void* src, unsigned long long size){
    return ::memcpy(dst, src, size);
  }

};

#endif
