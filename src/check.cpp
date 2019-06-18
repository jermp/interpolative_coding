#include <iostream>
#include <chrono>

#include "../external/mm_file/include/mm_file/mm_file.hpp"
#include "interpolative_coding.hpp"

using namespace bic;

template <typename Decoder>
void check(char const* binary_filename, char const* collection_filename) {
    int advice = mm::advice::sequential;
    mm::file_source<uint32_t> input_collection(collection_filename, advice);
    uint32_t const* input = input_collection.data();
    input += 2;

    mm::file_source<uint32_t> encoded(binary_filename, advice);
    uint32_t const* data = encoded.data();
    uint32_t universe = data[0];
    uint32_t sequences = data[1];
    Decoder dec(data + 2);
    std::vector<uint32_t> decoded(universe);

    std::cout << "checking " << sequences << " sequences..." << std::endl;
    size_t decoded_ints = 0;
    bool all_good = true;

    for (uint32_t i = 0; i != sequences; ++i) {
        uint32_t n = dec.decode(decoded.data());
        decoded_ints += n;
        if (n != input[0]) {
            std::cerr << "decoded " << n << " integers but expected "
                      << input[0] << std::endl;
            return;
        }
        all_good &= check(input + 1, decoded.data(), n);
        input += n + 1;

        if (i and i % 100000 == 0) {
            std::cout << "  checked " << i << " sequences" << std::endl;
        }
    }

    std::cout << "DONE" << std::endl;
    if (all_good) {
        std::cout << "everything good" << std::endl;
    }
}

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << argv[0]
                  << " decoder_type binary_filename collection_filename"
                  << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    char const* binary_filename = argv[2];
    char const* collection_filename = argv[3];

    if (type == "binary") {
        check<decoder<binary::reader>>(binary_filename, collection_filename);
    } else if (type == "leftmost_minimal") {
        check<decoder<leftmost_minimal::reader>>(binary_filename,
                                                 collection_filename);
    } else if (type == "centered_minimal") {
        check<decoder<centered_minimal::reader>>(binary_filename,
                                                 collection_filename);
    } else {
        std::cerr << "unknown type '" << type << "'" << std::endl;
        return 1;
    }

    return 0;
}
