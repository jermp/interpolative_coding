#include <iostream>
#include <chrono>

#include "../external/mm_file/include/mm_file/mm_file.hpp"
#include "interpolative_coding.hpp"

using namespace bic;
typedef std::chrono::high_resolution_clock clock_type;

template <typename Decoder>
void encode(char const* input_filename) {
    int advice = mm::advice::sequential;
    mm::file_source<uint32_t> input(input_filename, advice);
    uint32_t const* data = input.data();
    uint32_t universe = data[0];
    uint32_t sequences = data[1];
    Decoder dec(data + 2);
    std::vector<uint32_t> decoded(universe);

    std::cout << "decoding " << sequences << " sequences..." << std::endl;
    size_t decoded_ints = 0;
    auto start = clock_type::now();
    for (uint32_t i = 0; i != sequences; ++i) {
        uint32_t n = dec.decode(decoded.data());
        decoded_ints += n;
    }
    auto finish = clock_type::now();
    std::cout << "DONE" << std::endl;

    std::chrono::duration<double> elapsed = finish - start;
    std::cout << "decoded " << decoded_ints << " integers in "
              << elapsed.count() << " [sec]" << std::endl;
    std::cout << elapsed.count() * 1000000000 / decoded_ints << " ns/int"
              << std::endl;
    std::cout << "using " << input.bytes() * 8.0 / decoded_ints << " bits x int"
              << std::endl;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        std::cerr << argv[0] << " decoder_type input_filename" << std::endl;
        return 1;
    }

    std::string type(argv[1]);
    char const* input_filename = argv[2];
    std::cout << "type: '" << type << "':\n";

    if (type == "binary") {
        encode<decoder<binary::reader>>(input_filename);
    } else if (type == "leftmost_minimal") {
        encode<decoder<leftmost_minimal::reader>>(input_filename);
    } else if (type == "centered_minimal") {
        encode<decoder<centered_minimal::reader>>(input_filename);
    } else {
        std::cerr << "unknown type '" << type << "'" << std::endl;
        return 1;
    }

    return 0;
}
