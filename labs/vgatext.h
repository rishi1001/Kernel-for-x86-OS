#include "util/io.h"

namespace vgatext{

   static inline void writechar(int loc, uint8_t c, uint8_t bg, uint8_t fg, addr_t base){
     //your code goes here
     uint16_t combined = (c) | (bg << 12) | (fg << 8);
     mmio::write16(base, loc*2, combined);
   }

}//namespace vgatext
