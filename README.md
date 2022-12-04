flexible-shadow
===============

A versatile shadow memory library based on the paper 
[*How to shadow every byte of memory used by a program*](https://valgrind.org/docs/shadow-memory2007.pdf)
by Nicholas Nethercote and Julian Seward (2007).

Shadow memory can be thought of as a big associative array
indexed by memory addresses. It allows to store additional 
"shadow" data characterizing, but not interfering with, the 
original data at any given address.

For instance, Valgrind-Memcheck uses a shadow memory tool to track 
addressability and validity bits in order to detect memory errors.
The flexible-shadow tool has been developed for the automatic 
differentiation tool Derivgrind, which needs to keep track of dot 
values or variable indices.

Basic Usage
-----------
See [test.cpp](test.cpp).

Advanced Usage
--------------
- Users can configure the number of shadow map layers and how many address
  bits each of them resolves.
- Users can configure the functions used to allocate, deallocate, and copy
  memory. For instance, the alternative standard library interface defined in
  [flexible-shadow-valgrindstdlib.hpp](flexible-shadow-valgrindstdlib.hpp)
  allows to use flexible-shadow in a Valgrind tool.
