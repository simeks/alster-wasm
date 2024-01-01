#include <iostream>

#ifdef EMSCRIPTEN
#  include <emscripten.h>
#endif

int main() {
  std::cout << "Hello World!\n";
  return 0;
}
