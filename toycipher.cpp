/*
 * Toy 4-bit ARX cipher example
 *
 * State consists of two 4-bit words x[0], x[1].
 * Each round applies the ODD_2 operation:
 *   1) (a) = (a + b) mod 16
 *   2) (a) = ROTL4(a,3) ^ a
 *   3) (b) = (b + a) mod 16
 *
 * Here a=x[0], b=x[1]. We perform 5 rounds per encryption.
 *
 * This program measures the differential behavior by:
 *   - Generating random 4-bit plaintexts x[0] and a fixed nibble key in x[1]
 *   - Toggling one input bit of x[0]
 *   - Running both through 5 rounds of the cipher
 *   - Computing the probability that each output bit differs
 */

#include <bitset>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
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
        (a) = static_cast<u8>(((a) + (b)) & 0x0F);               \
        temp = (a);                                              \
        (a) = static_cast<u8>((ROTATE4(temp, 3) ^ temp) & 0x0F); \
        (b) = static_cast<u8>(((b) + (a)) & 0x0F);               \
    } while (0)

int main(int argc, char *argv[])
{

    std::time_t start_time = std::time(nullptr);
    std::cout << "Started at: "
              << std::put_time(std::localtime(&start_time), "%Y-%m-%d %H:%M:%S")
              << "\n";

    auto hr_start = std::chrono::high_resolution_clock::now();

    u8 x[2], key, temp, total_rounds{5};

    // plaintext and key : k3k2k1k0||p3p2p1p0
    //            x1   ||   x0

    x[0] = 12;
    key = 9;
    x[1] = key;
    temp = (x[1] << 4) | x[0];
    std::cout << "The plaintext is " << std::bitset<8>(temp) << "\n";
    // The plaintext is 0b10011100

    // encryption
    for (u16 rounds{0}; rounds < total_rounds; ++rounds)
    {
        ODD_2(x[0], x[1]);
        temp = (x[1] << 4) | x[0];
        std::cout << "\nThe ciphertxt is " << std::bitset<8>(temp) << " after " << static_cast<u16>(rounds+1) << " rounds \n";
    }

   
    auto hr_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = hr_end - hr_start;


    std::time_t end_time = std::time(nullptr);
    std::cout << "Ended   at: "
              << std::put_time(std::localtime(&end_time), "%Y-%m-%d %H:%M:%S")
              << "\n"
              << "Elapsed : " << elapsed.count() << " s\n";
}

/*
The plaintext is 10011100

The ciphertxt is 10001111 after 1 rounds

The ciphertxt is 01001100 after 2 rounds

The ciphertxt is 01000000 after 3 rounds

The ciphertxt is 10100110 after 4 rounds

The ciphertxt is 10100000 after 5 rounds*/