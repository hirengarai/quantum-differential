/*
 * differential of the toy-cipher
 */

#include <cstdint>
#include <iostream>
#include <random>

using u8 = std::uint8_t;   // positive integer of 8 bits
using u16 = std::uint16_t; // positive integer of 16 bits
using u32 = std::uint32_t; // positive integer of 32 bits
#define toggle_bit(word, bit) ((word) ^= (1u << (bit)))

// 4-bit rotate-left
static inline u8 ROTATE4(u8 x, unsigned n)
{
    x &= 0x0F; // keep only low nibble
    return static_cast<u8>(
        ((x << n) & 0x0F) |
        (x >> (4 - n)));
}

// Thread-local random engine
static thread_local std::mt19937 gen{std::random_device{}()};

// Generate a random boolean (50/50)
bool GenerateRandomBoolean()
{
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return (distribution(gen) > 0.5);
}

// Build a 4-bit random nibble from four coin-flips
static u8 Generate4BitRandom()
{
    u8 result = 0;
    for (int i = 0; i < 4; ++i)
    {
        result = (result << 1) | GenerateRandomBoolean();
    }
    return result;
}

// Single ARX-inspired round function on two 4-bit words
#define ODD_2(a, b)                                              \
    do                                                           \
    {                                                            \
        u8 temp;                                                 \
        /* add mod-16 */                                         \
        (a) = static_cast<u8>(((a) + (b)) & 0x0F);               \
        /* rotate-xor */                                         \
        temp = (a);                                              \
        (a) = static_cast<u8>((ROTATE4(temp, 3) ^ temp) & 0x0F); \
        /* mix b */                                              \
        (b) = static_cast<u8>(((b) + (a)) & 0x0F);               \
    } while (0)

int main(int argc, char *argv[])
{
    u8 x[2], dx[2], key, DiffState[2];
    u32 samples = 1ULL << 20, count;

    for (u16 id{0}; id < 4; ++id)
    {
        std::cout << "ID BIT: " << id << " ===-------------------===\n";
        for (u16 bit{0}; bit < 8; ++bit)
        {
            count = 0;
            std::cout << "Bit position: " << bit << "\n";
            for (u32 i{0}; i < samples; ++i)
            {
                // plaintext and key
                x[0] = Generate4BitRandom();
                key = Generate4BitRandom();
                x[1] = key;

                // prepare differential input
                dx[0] = x[0];
                dx[1] = x[1];
                toggle_bit(dx[0], id);

                // encrypt both
                for (u16 rounds{0}; rounds < 5; ++rounds)
                {
                    ODD_2(x[0], x[1]);
                    ODD_2(dx[0], dx[1]);
                }

                // compute output difference
                DiffState[0] = x[0] ^ dx[0];
                DiffState[1] = x[1] ^ dx[1];
                u8 temp = (DiffState[1] << 4) | DiffState[0];
                if ((temp >> bit) & 1)
                    count++;
            }
            std::cout << "Prob. " << static_cast<double>(count) / samples << "\n";
        }
    }
    return 0;
}