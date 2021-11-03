[![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/jermp/interpolative_coding.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/jermp/interpolative_coding/context:cpp)

Binary Interpolative Coding
===========================

A C++ library implementing the *Binary Interpolative Coding* compression algorithm invented by Alistair Moffat and Lang Stuiver [1].

The algorithm can be used to compress sorted integer sequences (here,
assumed to be increasing).

The implementation comes in different flavours:
it can be specified the use of
simple *binary* codes, *left-most minimal* codes and *centered minimal* codes.
Additionally, the implementation is *run-aware*, i.e.,
it optimizes encoding/decoding of runs of consecutive identifiers.

All details and experiments are provided in the following [technical report](http://pages.di.unipi.it/pibiri/papers/BIC.pdf) [2]

##### Table of contents
* [Compiling the code](#compiling-the-code)
* [Quick Start](#quick-start)
* [Encoding/decoding a collection of sequences](#encoding/decoding-a-collection-of-sequences)
* [Benchmark](#benchmark)
* [Author](#author)
* [References](#references)

Compiling the code
------------------

The code is tested on Linux with `gcc` 7.3.0, 8.3.0, 9.2.1 and on Mac 10.14 with `clang` 10.0.0.
To build the code, [`CMake`](https://cmake.org/) is required.

Clone the repository with

	git clone --recursive https://github.com/jermp/interpolative_coding.git

If you have cloned the repository without `--recursive`, you will need to perform the following commands before
compiling:

    git submodule init
    git submodule update

To compile the code for a release environment *and* best performance (see file `CMakeLists.txt` for the used compilation flags), do:

    mkdir build
    cd build
    cmake .. -DRUNAWARE=On
    make

Hint: Use `make -j4` to compile the library in parallel using, e.g., 4 jobs.

For a testing environment, use the following instead:

    mkdir debug_build
    cd debug_build
    cmake .. -DCMAKE_BUILD_TYPE=Debug -DUSE_SANITIZERS=On
    make

Quick Start
-------

For a quick start, see the source file `test/example.cpp`.
After compilation, run this example with

	./example

A simpler variation is shown below.

```C++
#include <iostream>

#include "interpolative_coding.hpp"
using namespace bic;

template <typename BinaryCode>
void test(std::vector<uint32_t> const& in) {
    std::cout << "to be encoded:\n";
    for (auto x : in) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    uint32_t n = in.size();

    encoder<typename BinaryCode::writer> enc;
    enc.encode(in.data(), n);

    std::vector<uint32_t> out(n);
    decoder<typename BinaryCode::reader> dec;
    uint32_t m = dec.decode(enc.bits().data(), out.data());
    assert(m == n);

    std::cout << "decoded " << m << " values" << std::endl;
    std::cout << "total bits " << enc.num_bits() << std::endl;
    std::cout << static_cast<double>(enc.num_bits()) / m << " bits x key"
              << std::endl;

    std::cout << "decoded:\n";
    for (auto x : out) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << argv[0] << " binary_code_type" << std::endl;
        return 1;
    }

    std::vector<uint32_t> in = {3, 4, 7, 13, 14, 15, 21, 25, 36, 38, 54, 62};

    std::string type(argv[1]);

    if (type == "binary") {
        test<binary>(in);
    } else if (type == "leftmost_minimal") {
        test<leftmost_minimal>(in);
    } else if (type == "centered_minimal") {
        test<centered_minimal>(in);
    } else {
        std::cerr << "unknown type '" << type << "'" << std::endl;
        return 1;
    }

    return 0;
}
```

Encoding/decoding a collection of sequences
----------------------------------

Typically, we want to build all the sequences from
a collection.
In this case, we assume that the input collection
is a binary file with all the sequences being written
as 32-bit integers. In this library, we follow the
input data format of the [`ds2i`](https://github.com/ot/ds2i) library:
each sequence is prefixed by an additional
32-bit integer representing the size of the sequence.
The collection file starts with a singleton sequence
containing the universe of representation of the sequences, i.e., the maximum representable value.

We also assume all sequences are *increasing*.

The file `data/test_collection.docs` represents an example of
such organization.

To encode all the sequences from this file, do:

	./encode leftmost_minimal ../data/test_collection.docs -o test.bin

To decode all the sequences from the encoded file `test.bin`, do:

	./decode leftmost_minimal test.bin

To check correctness of the implementation, use:

	./check leftmost_minimal test.bin ../data/test_collection.docs

which will compare every decoded integer against the input collection.

Benchmark
------
For this benchmark we used the whole Gov2 datasets, containing
5,742,630,292 integers in 35,636,425 sequences.

We report the average number of bits per integer (bpi)
and nanoseconds spent per decoded integer (with and without the
run-aware optimization).

We used two different Intel processors: i7-7700
and i9-9900K, both clocked at 3.6 GHz and having 32K L1 caches for
instructions and data.
Both systems run Linux 4.4.0 and have 64 GB on RAM.
The code was compiled with gcc 7.3.0 on the first
system; with gcc 8.3.0 on the second.
In both cases we used all optimizations
(see also `CMakeLists.txt`).

|**Method**        |**bpi** | **ns/int (run-aware) on i7-7700**  | **ns/int (not run-aware) on i7-7700**| **ns/int (run-aware) on i9-9900K** | **ns/int (not run-aware) on i9-9900K**|
|:-----------------|:------:|:------------------:|:------:|:-----:|:-----:|
|simple            |3.532   | 3.45               | 4.65   | 2.52  | 3.37  |
|left-most minimal |3.362   | 5.78               | 7.07   | 4.18  | 5.28  |
|centered minimal  |3.361   | 5.78               | 7.07   | 4.24  | 5.33  |

Author
------
* [Giulio Ermanno Pibiri](http://pages.di.unipi.it/pibiri/), <giulio.ermanno.pibiri@isti.cnr.it>

References
-------
* [1] Alistair Moffat and Lang Stuiver. 2000. *Binary Interpolative Coding for Effective Index Compression*. Information Retrieval Journal 3, 1 (2000), 25 – 47.
* [2] Giulio Ermanno Pibiri. 2019. *On Implementing the Binary Interpolative Coding Algorithm*. Technical report. [http://pages.di.unipi.it/pibiri/papers/BIC.pdf](http://pages.di.unipi.it/pibiri/papers/BIC.pdf)