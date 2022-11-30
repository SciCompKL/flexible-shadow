#include "flexible-shadow.hpp"
#include "flexible-shadow-defaultstdlib.hpp"
#include <iostream>


struct Leaf { 
  char data[1ul<<24]; 
  static Leaf distinguished;
};

Leaf Leaf::distinguished;


int main(){
  for(int i=0; i<(1ul<<24); i++){
    Leaf::distinguished.data[i]='u';
  }
  using SM = ShadowMap<unsigned long long, Leaf, DefaultStandardLibraryInterface, 20, 20, 24>;
  SM* sm = new SM;
  auto lookup_for_read = [&sm](unsigned long long index) -> char* {
    return &(sm->leaf_for_read(index)->data[sm->index(index)]);
  };
  auto lookup_for_write = [&sm](unsigned long long index) -> char* {
    return &(sm->leaf_for_write(index)->data[sm->index(index)]);
  };
  *lookup_for_write(0xabcdef0987654321) = '5';
  *lookup_for_write(0xabcde55987654321) = '5';
  std::cout << *lookup_for_read(0xabcdef0987654321) << std::endl;
  delete sm;
}
