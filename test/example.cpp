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

    // std::vector<uint32_t> in = {1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11,
    // 12,
    //                             13, 14, 15, 16, 17, 18, 19, 20, 23, 25, 32};
    // std::vector<uint32_t> in = {3,  4,  5,  6,  7,  8,  9,  10, 11, 12,
    //                             13, 14, 15, 21, 25, 36, 38, 54, 62};

    std::vector<uint32_t> in = {3, 4, 7, 13, 14, 15, 21, 25, 36, 38, 54, 62};

    // std::vector<uint32_t> in = {0, 1, 2, 3, 4, 5, 21, 25, 36, 38, 54, 62};

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