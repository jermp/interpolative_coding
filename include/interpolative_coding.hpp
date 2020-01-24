#pragma once

#include <vector>
#include <cassert>

namespace bic {

inline uint32_t msb(uint32_t x) {
    assert(x > 0);
    return 31 - __builtin_clz(x);
}

struct output {
    output() : m_size(0), m_cur_word(nullptr) {}

    void append(uint32_t bits, uint32_t len) {
        if (!len) return;
        uint32_t pos_in_word = m_size % 32;
        m_size += len;
        if (pos_in_word == 0) {
            m_bits.push_back(bits);
        } else {
            *m_cur_word |= bits << pos_in_word;
            if (len > 32 - pos_in_word) {
                m_bits.push_back(bits >> (32 - pos_in_word));
            }
        }
        m_cur_word = &m_bits.back();
    }

    void reserve(size_t bytes) {
        m_bits.reserve((bytes + sizeof(m_bits.front()) - 1) /
                       sizeof(m_bits.front()));
    }

    size_t num_bits() const {
        return m_size;
    }

    std::vector<uint32_t> const& bits() const {
        return m_bits;
    }

private:
    std::vector<uint32_t> m_bits;
    size_t m_size;
    uint32_t* m_cur_word;
};

struct input {
    input() {}

    input(uint32_t const* in) {
        init(in);
    }

    void init(uint32_t const* in) {
        m_in = in;
        m_avail = 0;
        m_buf = 0;
    }

    uint32_t take(uint32_t len) {
        if (!len) return 0;
        if (m_avail < len) {
            m_buf |= uint64_t(*m_in++) << m_avail;
            m_avail += 32;
        }
        uint32_t val = m_buf & ((uint64_t(1) << len) - 1);
        m_buf >>= len;
        m_avail -= len;
        return val;
    }

private:
    uint32_t const* m_in;
    uint32_t m_avail;
    uint64_t m_buf;
};

struct binary {
    struct writer : output {
        void write(uint32_t x, uint32_t r) {
#if RUNAWARE
            assert(r > 0);
#else
            if (!r) return;
#endif
            assert(x <= r);
            uint32_t b = msb(r) + 1;
            append(x, b);
        }
    };

    struct reader : input {
        reader() {}
        reader(uint32_t const* encoded) : input(encoded) {}

        uint32_t read(uint32_t r) {
#if RUNAWARE
            assert(r > 0);
#else
            if (!r) return 0;
#endif
            uint32_t b = msb(r) + 1;
            uint32_t x = take(b);
            assert(x <= r);
            return x;
        }
    };
};

struct leftmost_minimal {
    struct writer : output {
        void write(uint32_t x, uint32_t r) {
#if RUNAWARE
            assert(r > 0);
#else
            if (!r) return;
#endif
            assert(x <= r);
            uint32_t b = msb(r);
            uint32_t hi = (uint64_t(1) << (b + 1)) - r - 1;
            if (x < hi) {
                append(x, b);
            } else {
                x += hi;
                append(x >> 1, b);
                append(x & 1, 1);
            }
        }
    };

    struct reader : input {
        reader() {}
        reader(uint32_t const* encoded) : input(encoded) {}

        uint32_t read(uint32_t r) {
#if RUNAWARE
            assert(r > 0);
#else
            if (!r) return 0;
#endif
            uint32_t b = msb(r);
            uint32_t hi = (uint64_t(1) << (b + 1)) - r - 1;
            uint32_t x = take(b);
            if (x >= hi) x = (x << 1) + take(1) - hi;
            assert(x <= r);
            return x;
        }
    };
};

struct centered_minimal {
    struct writer : output {
        void write(uint32_t x, uint32_t r) {
#if RUNAWARE
            assert(r > 0);
#else
            if (!r) return;
#endif
            uint32_t b = msb(r);
            uint32_t c = (uint64_t(1) << (b + 1)) - r - 1;
            int64_t half_c = c / 2;
            int64_t half_r = r / 2;
            int64_t lo, hi;
            lo = half_r - half_c;
            hi = half_r + half_c + 1;
            if (r % 2 == 0) lo -= 1;
            if (x > lo and x < hi) {
                append(x, b);
            } else {
                append(x, b + 1);
            }
        }
    };

    struct reader : input {
        reader() {}
        reader(uint32_t const* encoded) : input(encoded) {}

        uint32_t read(uint32_t r) {
#if RUNAWARE
            assert(r > 0);
#else
            if (!r) return 0;
#endif
            uint32_t b = msb(r);
            uint32_t c = (uint64_t(1) << (b + 1)) - r - 1;
            int64_t half_c = c / 2;
            int64_t half_r = r / 2;
            int64_t lo;
            lo = half_r - half_c;
            if (r % 2 == 0) lo -= 1;
            uint32_t x = take(b);
            if (x <= lo) x += take(1) << b;
            assert(x <= r);
            return x;
        }
    };
};

template <typename Writer>
struct encoder {
    void encode(uint32_t const* input, uint32_t n) {
        if (!n) return;
        uint32_t universe = input[n - 1];
        write_binary(universe);
        write_binary(n);
        encode(input, n - 1, 0, universe);
    }

    void reserve(size_t bytes) {
        return m_writer.reserve(bytes);
    }

    size_t num_bits() const {
        return m_writer.num_bits();
    }

    auto const& bits() const {
        return m_writer.bits();
    }

private:
    void encode(uint32_t const* input, uint32_t n, uint32_t lo, uint32_t hi) {
        if (!n) return;
#if RUNAWARE
        if (hi - lo + 1 == n) return;  // run
#endif
        assert(lo <= hi);
        assert(hi - lo >= n - 1);
        uint32_t m = n / 2;
        uint32_t x = input[m];
        m_writer.write(x - lo - m, hi - lo - n + 1);
        encode(input, m, lo, x - 1);
        encode(input + m + 1, n - m - 1, x + 1, hi);
    }

    void write_binary(uint32_t x) {
        uint32_t b = 0;
        if (x) b = msb(x);
        assert(b <= 31);
        m_writer.append(b, 5);
        m_writer.append(x, b + 1);
    }

    Writer m_writer;
};

template <typename Reader>
struct decoder {
    decoder() {}
    decoder(uint32_t const* encoded) : m_reader(encoded) {}

    uint32_t decode(uint32_t* out) {
        uint32_t universe = read_binary();
        uint32_t n = read_binary();
        out[n - 1] = universe;
        decode(out, n - 1, 0, universe);
        return n;
    }

    uint32_t decode(uint32_t const* encoded, uint32_t* out) {
        m_reader.init(encoded);
        return decode(out);
    }

private:
    void decode(uint32_t* out, uint32_t n, uint32_t lo, uint32_t hi) {
        if (!n) return;
        assert(lo <= hi);
#if RUNAWARE
        if (hi - lo + 1 == n) {  // run
            for (uint32_t i = 0; i != n; ++i) out[i] = lo++;
            return;
        }
#endif
        uint32_t m = n / 2;
        uint32_t x = m_reader.read(hi - lo - n + 1) + lo + m;
        out[m] = x;
        if (n == 1) return;
        decode(out, m, lo, x - 1);
        decode(out + m + 1, n - m - 1, x + 1, hi);
    }

    uint32_t read_binary() {
        uint32_t b = m_reader.take(5);
        return m_reader.take(b + 1);
    }

    Reader m_reader;
};

bool check(uint32_t const* expected, uint32_t const* got, uint32_t n) {
    for (uint32_t i = 0; i != n; ++i) {
        if (expected[i] != got[i]) {
            std::cerr << "error at " << i << "/" << n << ": ";
            std::cerr << "expected " << expected[i] << " but got " << got[i]
                      << std::endl;
            return false;
        }
    }
    return true;
}

}  // namespace bic
