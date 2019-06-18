Memory-mapped files
-------------------
A self-contained, header-only, implementation of memory-mapped files in C++
for both reading and writing.
This is meant to offer the same capability of [`boost::mapped_file`](https://www.boost.org/doc/libs/1_69_0/libs/iostreams/doc/classes/mapped_file.html) (`source` and `sink`) but avoiding
the dependance from `boost`.

Example usage.

```C++
#include <iostream>

#include "../include/mm_file/mm_file.hpp"

int main() {
    std::string filename("./tmp.bin");
    static const size_t n = 13;

    {
        // write n uint32_t integers
        mm::file_sink<uint32_t> fout(filename, n);
        std::cout << "mapped " << fout.bytes() << " bytes "
                  << "for " << fout.size() << " integers" << std::endl;

        auto* data = fout.data();
        for (uint32_t i = 0; i != fout.size(); ++i) {
            data[i] = i;
            std::cout << "written " << data[i] << std::endl;
        }

        fout.close();
    }

    {
        // instruct the kernel that we will read the content
        // of the file sequentially
        int advice = mm::advice::sequential;

        // read the stream as uint16_t integers
        mm::file_source<uint16_t> fin(filename, advice);
        std::cout << "mapped " << fin.bytes() << " bytes "
                  << "for " << fin.size() << " integers" << std::endl;

        auto const* data = fin.data();
        for (uint32_t i = 0; i != fin.size(); ++i) {
            std::cout << "read " << data[i] << std::endl;
        }

        fin.close();
    }

    std::remove(filename.c_str());

    return 0;
}
```

See also the example in `test/test.cpp`. To compile that, just type the following commands from the parent directory.

    $ cd test
    $ mkdir build
    $ cd build
    $ cmake ..
    $ make