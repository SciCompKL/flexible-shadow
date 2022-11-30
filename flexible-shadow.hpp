/* \file flexible-shadow.hpp
 * Define shadow memory functionality using templates.
 */


/*! \struct ShadowStandardLibraryInterface
 * Standard library-like functions to be used by ShadowMap objects.
 * 
 * The template ShadowMap, by itself, does not use any functions
 * of the standard library. It needs memory allocation functionality,
 * which must be provided by the template-instantiating code
 * through the template parameter StandardLibraryInterface. 
 * 
 * This struct is a mock interface for the purpose of documentation,
 * do not use it in your code.
 * "Normal" C++ code may include flexible-shadow-defaultstdlib.hpp and
 * use DefaultStandardLibraryInterface. 
 * Code linked into a Valgrind tool cannot link against the standard 
 * library, and must use VG_(malloc), VG_(free), e.g. with the
 * ValgrindStandardLibraryInterface in flexible-shadow-valgrindstdlib.hpp.
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

};


/*! \struct ShadowMap
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

  /*! Given a memory address, return a pointer to leaf shadowing it,
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

  /*! Given a memory address, return a pointer to leaf shadowing it,
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
    map->_leaf.construct();
  }

  static void destructAt(This* map){
    map->_leaf.destruct();
  }

  Leaf* leaf(Address addr){
    return &_leaf; 
  }

  Leaf* leaf_for_write(Address addr){
    return &_leaf; 
  }

  static unsigned long long index(Address addr){
    return addr & ((1ul<<dimension0)-1);
  }

  static Address contiguousElements(Address addr){
    return (1ul<<dimension0) - index(addr);
  }
};


