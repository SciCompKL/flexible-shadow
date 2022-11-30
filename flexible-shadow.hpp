/* \file flexible-shadow.hpp
 *
 * Versatile C++ template implementation of shadow memory
 * functionality realized by a trie-like data structure,
 * similar to the approach M0 presented in the paper
 * *How to shadow every byte of memory used by a program.*
 * by Nicholas Nethercote and Julian Seward (2007).
 *
 * Data structure
 * --------------
 *
 * The user can specify the address type and a partition 
 * sizeof(address type) = dimension0 + ... + dimensionN.
 *
 * To access the shadow memory for a given address, a 
 * primary ShadowMap with 2^dimension0 entries is queried 
 * with the first dimension0 address bits; this yields a
 * pointer to a secondary ShadowMap with 2^dimension1 
 * entries that is queried with the next dimension1 address
 * bits, and so forth. Finally, the (N+1)-ary ShadowMap
 * is called "leaf" and stores one or many arrays with the
 * shadow data for a contiguous range of 2^dimensionN many 
 * addresses.
 * 
 * Leaves
 * ------
 *
 * The precise data structure for a leaf must be defined by the code instantiating the
 * ShadowMap template. In order to store a plain-old-data (POD) type like `char`
 * for each memory address, you can use a member variable like
 * 
 *     char data[1ul<<dimensionN];
 * 
 * For composite types like a double plus an int, either follow an
 * array-of-structures approach,
 * 
 *     struct Data {double d_1; int d_2;};
 *     Data data[1ul<<dimensionN];
 * 
 * or a structures-of-array approach,
 * 
 *    double data_1[1ul<<dimensionN];
 *    int data_2[1ul<<dimensionN];
 * 
 * To access shadow data, use the memory address to query the ShadowMap for 
 * the leaf and the index within that leaf. Then, access the data in the leaf
 * according to your choice of data structure.
 * 
 * In addition to member variables for data, there must be a static member 
 * variable Leaf::distinguished of type Leaf, initialized with an empty leaf
 * before the first query.
 *
 * Standard library
 * ----------------
 * The template ShadowMap, by itself, does not use any functions
 * of the standard library. It needs memory allocation functionality,
 * which must be provided by the template-instantiating code
 * through the template parameter StandardLibraryInterface:
 * - See MockStandardLibraryInterface in flexible-shadow-mockstdlib.hpp
 *   for documentation, but do not use it.
 * - Use DefaultStandardLibraryInterface in flexible-shadow-defaultstdlib.hpp
 *   for "normal" C++ code that may be linked against the C standard library.
 * - Use ValgrindStandardLibraryInterface in flexible-shadow-valgrindstdlib.hpp
 *   in Valgrind tool code.
 * - Or define your own interface :)
 *
 */


/*! \struct ShadowMap
 *
 * 
 *
 * A ShadowMap of level N=0, stores 2^dimension0 many shadow objects as a "leaf", and 
 * therefore provides shadow memory for an address space of dimension0-many bits.
 * A ShadowMap of level N>0 can store 2^dimension0 many ShadowMap's of level (N-1),
 * and therefore provides shadow memory for an address space of 
 * (dimension0+...+dimensionN)-many bits. The lower-level ShadowMap's are only
 * instantiated if the part of the address space that they cover is actually accessed.
 *
 * To access the shadow memory at address addr, obtain a pointer to the leaf shadowing 
 * addr with ShadowMap::leaf or ShadowMap::leaf_for_write, and access it at the index
 * calculated by ShadowMap::index. This two-stage procedure allows the user to store 
 * complex shadow types either in an array-of-structures or a structure-of-arrays fashion.
 * ShadowMap::leaf returns a nullptr if the memory address is not currently shadowed,
 * while ShadowMap::leaf_for_write allocates 
 *
 * This template, by itself, does not use any functions from the standard library.
 * It needs memory allocation functionality which must be provided by the template-instantiating code
 * through static methods of the template typename parameter StandardLibraryInterface.
 *
 * \tparam Address Type of an address, e.g. unsigned long long with 64 bit.
 * \tparam Leaf Type of shadow data stored on top of each original data byte.
 * \tparam StandardLibraryInterface Class containing static functions safe_malloc, free.
 * \tparam dimension0 Number of address bits processed by highest-level map.
 * \tparam dimensions... Numbers of address bits processed by lower-level maps.
 */
template<typename Address, typename Leaf, typename StandardLibraryInterface, int dimension0, int...dimensions>
struct ShadowMap {
  /*! Type of lower-level map, this map stores pointers to such objects.
   */
  using Lower = ShadowMap<Address,Leaf,StandardLibraryInterface,dimensions...>;
  /*! Type of this class.
   */
  using This = ShadowMap<Address,Leaf,StandardLibraryInterface,dimension0,dimensions...>;

  /*! Number of address bits covered by this map and all lower-level maps.
   */
  static constexpr int dimensions_sum = dimension0 + Lower::dimensions_sum;

  /*! Pointers to lower-level maps.
   */
  Lower* _pointers[1ul<<dimension0];

  /*! Constructor for the user.
   */
  ShadowMap(){
    constructAt(this);
  }
  /*! Destructor for the user.
   */
  ~ShadowMap(){
    destructAt(this);
  }

  /*! Internal "constructor" initializing the map on plain allocated memory.
   * \param map Pointer to allocated memory where the ShadowMap to be "constructed".
   */
  static void constructAt(This* map){
    for(unsigned long long i=0; i<(1ul<<dimension0); i++){
      map->_pointers[i] = nullptr;
    }
  }
  /*! Internal "destructor" recursively freeing all pointers to lower-level maps.
   * \param map Pointer to ShadowMap to be "destructed".
   */
  static void destructAt(This* map){
    for(unsigned long long i=0; i<(1ul<<dimension0); i++){
      if(map->_pointers[i]){
        Lower::destructAt(map->_pointers[i]);
        StandardLibraryInterface::free(map->_pointers[i]);
      }
    }
  }

  /*! Given a memory address, return a pointer to the leaf shadowing it,
   * or nullptr if the memory address is not shadowed.
   *
   * \param addr Memory address.
   */
  Leaf* leaf(Address addr){
    unsigned long long index = (addr>>Lower::dimensions_sum) & ((1ul<<dimension0)-1);
    Lower* pointer = _pointers[ index ];
    if(pointer==nullptr){
      return nullptr;
    } else {
      return pointer->leaf(addr);
    }
  }

  /*! Given a memory address, return a pointer to the leaf shadowing it,
   * allocating it if the memory address is not shadowed yet.
   *
   * \param addr Memory address.
   */
  Leaf* leaf_for_write(Address addr){
    unsigned long long index = (addr>>Lower::dimensions_sum) & ((1ul<<dimension0)-1);
    Lower* pointer = _pointers[ index ];
    if(pointer==nullptr){
      _pointers[index] = (Lower*)StandardLibraryInterface::safe_malloc(sizeof(Lower));
      Lower::constructAt(_pointers[index]);
      return _pointers[index]->leaf_for_write(addr);
    } else {
      return pointer->leaf_for_write(addr);
    }
  }

  /*! Given a memory address, return a pointer to the leaf shadowing it,
   * or to the distinguished leaf if the memory address is not shadowed yet.
   *
   * \param addr Memory address.
   */
  Leaf* leaf_for_read(Address addr){
    unsigned long long index = (addr>>Lower::dimensions_sum) & ((1ul<<dimension0)-1);
    Lower* pointer = _pointers[ index ];
    if(pointer==nullptr){
      return &Leaf::distinguished;
    } else {
      return pointer->leaf_for_read(addr);
    }
  }

  /*! Return the index of a memory address withing a leaf shadowing it.
   *
   * The leaf does not need to be allocated for this calculation.
   * \param addr Memory address.
   */
  static unsigned long long index(Address addr){
    return Lower::index(addr);
  }

  /*! Return the maximal N such that the shadow objects for the addresses 
   *  addr, addr+1, ..., addr+N-1 are contiguously stored at data(addr).
   */
  static Address contiguousElements(Address addr){
    return Lower::contiguousElements(addr);
  }

};


template<typename Address, typename Leaf, typename StandardLibraryInterface, int dimension0>
struct ShadowMap<Address,Leaf,StandardLibraryInterface,dimension0>{

  using This = ShadowMap<Address,Leaf,StandardLibraryInterface,dimension0>;
  static constexpr int dimensions_sum = dimension0;

  Leaf _leaf;

  ShadowMap(){
    constructAt(this);
  }
  ~ShadowMap(){
    destructAt(this);
  }

  static void constructAt(This* map){
    StandardLibraryInterface::memcpy(&map->_leaf, &Leaf::distinguished, sizeof(_leaf));
  }

  static void destructAt(This* map){
  }

  Leaf* leaf(Address addr){
    return &_leaf; 
  }

  Leaf* leaf_for_write(Address addr){
    return &_leaf; 
  }

  Leaf* leaf_for_read(Address addr){
    return &_leaf; 
  }

  static unsigned long long index(Address addr){
    return addr & ((1ul<<dimension0)-1);
  }

  static Address contiguousElements(Address addr){
    return (1ul<<dimension0) - index(addr);
  }
};


