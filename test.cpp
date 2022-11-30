#include "flexible-shadow.hpp"
#include <iostream>

void* shadow_malloc(unsigned long long size){
  std::cout << "allocating " << size << std::endl;
  return new char[size];
}

void shadow_free(void* ptr){
  delete[] (char*)ptr;
}

void shadow_out_of_memory(){
  std::cerr << "Memory allocation failed." << std::endl;
}

int main(){
  using SM = ShadowMap<unsigned long long, char, shadow_malloc, shadow_free, 20, 20, 24>;
  SM* sm = new SM;
  *(sm->template data<true>(0xabcdef0987654321)) = '5';
  *(sm->template data<true>(0xabcde50987654321)) = 'j';
  std::cout << *(sm->template data<false>(0xabcdef0987654321)) << std::endl;
  delete sm;
}
