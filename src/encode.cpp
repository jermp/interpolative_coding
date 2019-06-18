#include <iostream>
#include <fstream>
#include <cassert>

#include "../external/mm_file/include/mm_file/mm_file.hpp"
#include "interpolative_coding.hpp"

using namespace bic;
static const uint64_t GiB = 1024 * 1024 * 1024;

template <typename Encoder>
void encode(char const* input_filename, char const* output_filename) {
    mm::file_source<uint32_t> input(input_filename);
    uint32_t const* data = input.data();
    assert(data[0] == 1);
    uint32_t universe = data[1];

    Encoder enc;
    enc.reserve(10 * GiB);

    std::cout << "encoding data..." << std::endl;
    uint32_t encoded_sequences = 0;
    size_t encoded_ints = 0;
    for (size_t i = 2; i < input.size();) {
        uint32_t n = data[i];
        enc.encode(data + i + 1, n);
        i += n + 1;
        encoded_ints += n;
        encoded_sequences += 1;
        if (encoded_sequences % 100000 == 0) {
            std::cout << "  encoded " << encoded_sequences << " sequences"
                      << std::endl;
        }
    }
    std::cout << "DONE" << std::endl;

    std::cout << "encoded " << encoded_sequences << " sequences" << std::endl;
    std::cout << "encoded " << encoded_ints << " integers" << std::endl;

    // NOTE: slightly larger than enc.num_bits() due to padding
    std::cout << "using "
              << enc.bits().size() * sizeof(enc.bits().front()) * 8.0 /
                     encoded_ints
              << " bits x int" << std::endl;

    if (output_filename) {
        std::ofstream out(output_filename, std::ios::binary);
        if (!out.is_open()) {
            std::cerr << "error in opening binary file" << std::endl;
            return;
        }
        std::cout << "writing encoded data to disk..." << std::endl;

        // save also:
        // - universe (max sequence length)
        // - number of encoded sequences
        out.write(reinterpret_cast<char const*>(&universe), sizeof(universe));
        out.write(reinterpret_cast<char const*>(&encoded_sequences),
                  sizeof(encoded_sequences));

        out.write(reinterpret_cast<char const*>(enc.bits().data()),
                  static_cast<std::streamsize>(sizeof(enc.bits().front()) *
                                               enc.bits().size()));
        std::cout << "DONE" << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << argv[0]
                  << " encoder_type input_filename -o output_filename"
                  << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    char const* input_filename = argv[2];
    char const* output_filename = nullptr;

    if (argc > 3 and argv[3] == std::string("-o")) {
        output_filename = argv[4];
    }

    if (type == "binary") {
        encode<encoder<binary::writer>>(input_filename, output_filename);
    } else if (type == "leftmost_minimal") {
        encode<encoder<leftmost_minimal::writer>>(input_filename,
                                                  output_filename);
    } else if (type == "centered_minimal") {
        encode<encoder<centered_minimal::writer>>(input_filename,
                                                  output_filename);
    } else {
        std::cerr << "unknown type '" << type << "'" << std::endl;
        return 1;
    }

    return 0;
}
