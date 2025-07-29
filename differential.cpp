/*
Differential search*/

#include <chrono>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <random>


using u8 = std::uint8_t; // 8-bit unsigned
using u16 = std::uint16_t; // 16-bit unsigned
using u32 = std::uint32_t; // 32-bit unsigned
#define toggle_bit(word, bit) ((word) ^= (1u << (bit)))

// 4-bit rotate-left
enum
{
    bitsize = 4
};

static inline u8 ROTATE4(u8 x, unsigned n)
{
    x &= 0x0F;
    return static_cast<u8>(((x << n) & 0x0F) | (x >> (4 - n)));
}

// Random generator
static thread_local std::mt19937 gen{std::random_device{}()};
bool GenerateRandomBoolean()
{
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    return dist(gen) > 0.5;
}
static u8 Generate4BitRandom()
{
    u8 r = 0;
    for (int i = 0; i < bitsize; ++i)
        r = (r << 1) | GenerateRandomBoolean();
    return r;
}

// One ARX-inspired round
#define ODD_2(a, b)                                              \
    do                                                           \
    {                                                            \
        u8 temp;                                                 \
        (a) = static_cast<u8>(((a) + (b)) & 0x0F);               \
        temp = (a);                                              \
        (a) = static_cast<u8>((ROTATE4(temp, 3) ^ temp) & 0x0F); \
        (b) = static_cast<u8>(((b) + (a)) & 0x0F);               \
    } while (0)

int main()
{
    std::time_t start_time = std::time(nullptr);
    std::cout << "Started at: "
              << std::put_time(std::localtime(&start_time), "%Y-%m-%d %H:%M:%S")
              << "\n";

    auto t_start = std::chrono::high_resolution_clock::now();

    u8 x[2], dx[2], key, diff[2];
    u32 samples = 1U << 20;

    for (u8 id = 0; id < bitsize; ++id)
    {
        std::cout << "ID BIT: " << int(id) << "\n";
        for (u8 bit = 0; bit < 8; ++bit)
        {
            u32 count = 0;
            for (u32 i = 0; i < samples; ++i)
            {
                x[0] = Generate4BitRandom();
                key = Generate4BitRandom();
                x[1] = key;
                dx[0] = x[0];
                dx[1] = x[1];
                toggle_bit(dx[0], id);
                for (int r = 0; r < 5; ++r)
                {
                    ODD_2(x[0], x[1]);
                    ODD_2(dx[0], dx[1]);
                }
                diff[0] = x[0] ^ dx[0];
                diff[1] = x[1] ^ dx[1];
                u8 t = (diff[1] << 4) | diff[0];
                if ((t >> bit) & 1)
                    count++;
            }
            std::cout << "OD BIT " << int(bit)
                      << " → Prob = " << double(count) / samples << "\n";
        }
    }


    auto t_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = t_end - t_start;


    std::time_t end_time = std::time(nullptr);
    std::cout << "Ended   at: "
              << std::put_time(std::localtime(&end_time), "%Y-%m-%d %H:%M:%S")
              << "\n"
              << "Elapsed : " << elapsed.count() << " s\n";
    return 0;
}