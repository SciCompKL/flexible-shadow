/*! \file test.cpp
 * Regression test and example for the usage of the
 * shadow memory tool.
 */

// Include flexible-shadow.hpp and one of the standard 
// library wrapper headers.
#include "flexible-shadow.hpp" 
#include "flexible-shadow-defaultstdlib.hpp" 

// Define a leaf class storing ...
struct Leaf { 
  // ... the shadow data for dimensionN (here, 24) contiguous memory addresses.
  char data_1[1ul<<24];
  double data_2[1ul<<24];
  // The distinguished leaf ...
  static Leaf distinguished;
};
// ... declared here ...
Leaf Leaf::distinguished;


int main(){
  // ... must be initialized with "empty" shadow data before the first access.
  for(int i=0; i<(1ul<<24); i++){
    Leaf::distinguished.data_1[i]='u';
    Leaf::distinguished.data_2[i]=0.;
  }
  // Specify the index type, leaf type, standard library interface, and a
  // partition of the address bits (here 64=20+20+24) whose last summand
  // must match the dimensionN of the leaf class.
  using SM = ShadowMap<unsigned long long, Leaf, DefaultStandardLibraryInterface, 20, 20, 24>;
  SM* sm = new SM;
  // This is how to read data:
  auto lookup_char_for_read = [&sm](unsigned long long index) -> char const& {
    return (sm->leaf_for_read(index)->data_1[sm->index(index)]);
  };
  auto lookup_double_for_read = [&sm](unsigned long long index) -> double const& {
    return (sm->leaf_for_read(index)->data_2[sm->index(index)]);
  };
  // This is how to write data:
  auto lookup_char_for_write = [&sm](unsigned long long index) -> char& {
    return (sm->leaf_for_write(index)->data_1[sm->index(index)]);
  };
  auto lookup_double_for_write = [&sm](unsigned long long index) -> double& {
    return (sm->leaf_for_write(index)->data_2[sm->index(index)]);
  };
  // Let's test it:
  bool pass = true;
  lookup_char_for_write(0xfedcba9876543210) = '5';
  pass = pass && (lookup_char_for_read(0x0) == 'u');
  pass = pass && (lookup_char_for_read(0x0123456789abcdef) == 'u');
  pass = pass && (lookup_char_for_read(0xfedcba9876543210) == '5');
  lookup_double_for_write(0xfedcba9876543210) = 3.14;
  lookup_double_for_write(0x0) = 2.72;
  pass = pass && (lookup_double_for_read(0x0) == 2.72);
  pass = pass && (lookup_double_for_read(0x0123456789abcdef) == 0.0);
  pass = pass && (lookup_double_for_read(0xfedcba9876543210) == 3.14);

  delete sm;

  return pass ? 0 : 1;
}
