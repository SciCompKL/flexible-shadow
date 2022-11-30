#include <cstdlib>
#include <iostream>

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

};
