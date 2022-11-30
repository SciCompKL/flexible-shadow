#include "flexible-shadow.hpp"
#include "flexible-shadow-defaultstdlib.hpp"
#include <iostream>

struct Leaf { char data[1ul<<24];  void construct(){} void destruct(){} };

int main(){
  using SM = ShadowMap<unsigned long long, Leaf, DefaultStandardLibraryInterface, 20, 20, 24>;
  SM* sm = new SM;
  auto lookup = [&sm](unsigned long long index) -> char* {
    return &(sm->leaf_for_write(index)->data[sm->index(index)]);
  };
  *lookup(0xabcdef0987654321) = '5';
  *lookup(0xabcde55987654321) = '5';
  std::cout << *lookup(0xabcdef0987654321) << std::endl;
  delete sm;
}
