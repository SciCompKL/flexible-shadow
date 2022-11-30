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

struct Leaf { char data[1ul<<24];  void construct(){} void destruct(){} };

int main(){
  using SM = ShadowMap<unsigned long long, Leaf, shadow_malloc, shadow_free, 20, 20, 24>;
  SM* sm = new SM;
  auto lookup = [&sm](unsigned long long index) -> char* {
    return &(sm->leaf_for_write(index)->data[sm->index(index)]);
  };
  *lookup(0xabcdef0987654321) = '5';
  *lookup(0xabcde55987654321) = '5';
  std::cout << *lookup(0xabcdef0987654321) << std::endl;
  delete sm;
}
