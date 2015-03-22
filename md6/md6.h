/* File:    md6.h 
** Author:  Ronald L. Rivest
** Address: Room 32G-692 Stata Center 
**          32 Vassar Street 
**          Cambridge, MA 02139
** Email:   rivest@mit.edu
** Date:    1/15/2009        (see changelog below)
**
** (The following license is known as "The MIT License")
** 
** Copyright (c) 2008 Ronald L. Rivest
** 
** Permission is hereby granted, free of charge, to any person obtaining a copy
** of this software and associated documentation files (the "Software"), to deal
** in the Software without restriction, including without limitation the rights
** to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
** copies of the Software, and to permit persons to whom the Software is
** furnished to do so, subject to the following conditions:
** 
** The above copyright notice and this permission notice shall be included in
** all copies or substantial portions of the Software.
** 
** THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
** IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
** FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
** AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
** LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
** OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
** THE SOFTWARE.
**
** (end of license)
**
** This file is part of the definition of the MD6 hash function.
** The files defining the MD6 hash function are:
**    md6.h
**    md6_compress.c
**    md6_mode.
** (Note that md6.h includes inttypes.h, which includes stdint.h; 
** versions of these header files compatible with MS Visual Studio
** are available, as noted below.)
**
** The files defining the interface between MD6 and the NIST SHA-3
** API are:
**    md6_nist.h
**    md6_nist.c
** The NIST SHA-3 API is defined in:
**    http://www.csrc.nist.gov/groups/ST/hash/documents/SHA3-C-API.pdf
**
** See  http://groups.csail.mit.edu/cis/md6  for more information.
**
** These files define the ``standard'' version of MD6.  However,
** they are written in such a way that one may relatively easily
** define and experiment with ``variant'' versions.
**
** Changelog:
**   1/15/2009: length of md6_state.hashval doubled (to 128 bytes)
**              to avoid buffer overflow issue found by Fortify.
**              (Change does not affect hash values computed, though.)
*/

/* Prevent multiple includes
** Matching endif is at end of md6.h
*/

#ifndef BZF_MD6_H
#define BZF_MD6_H
 
#include <cstring>
#include <iostream>
 
 
// a small class for calculating MD6 hashes of strings or byte arrays
// it is not meant to be fast or secure
//
// usage: 1) feed it blocks of uchars with update()
//      2) finalize()
//      3) get hexdigest() string
//      or
//      MD6(std::string).hexdigest()
//
// assumes that char is 8 bit and int is 32 bit
class MD6
{
public:
  typedef unsigned int size_type; // must be 32bit
 
  MD6();
  MD6(const std::string& text);
  void update(const unsigned char *buf, size_type length);
  void update(const char *buf, size_type length);
  MD6& finalize();
  std::string hexdigest() const;
  friend std::ostream& operator<<(std::ostream&, MD6 md6);
 
private:
  void init();
  typedef unsigned char uint1; //  8bit
  typedef unsigned int uint4;  // 32bit
  enum {blocksize = 64}; // VC6 won't eat a const static int here
 
  void transform(const uint1 block[blocksize]);
  static void decode(uint4 output[], const uint1 input[], size_type len);
  static void encode(uint1 output[], const uint4 input[], size_type len);
 
  bool finalized;
  uint1 buffer[blocksize]; // bytes that didn't fit in last 64 byte chunk
  uint4 count[2];   // 64bit counter for number of bits (lo, hi)
  uint4 state[4];   // digest so far
  uint1 digest[16]; // the result
 
  // low level logic operations
  static inline uint4 F(uint4 x, uint4 y, uint4 z);
  static inline uint4 G(uint4 x, uint4 y, uint4 z);
  static inline uint4 H(uint4 x, uint4 y, uint4 z);
  static inline uint4 I(uint4 x, uint4 y, uint4 z);
  static inline uint4 rotate_left(uint4 x, int n);
  static inline void FF(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
  static inline void GG(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
  static inline void HH(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
  static inline void II(uint4 &a, uint4 b, uint4 c, uint4 d, uint4 x, uint4 s, uint4 ac);
};
 
std::string md6(const std::string str);
 
#endif
/* end of md6.h */
