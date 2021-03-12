#include "wrapping_integers.hh"
// #define LOG
#ifdef LOG
#include<dbg.h>
#endif
// Dummy implementation of a 32-bit wrapping integer

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

//! Transform an "absolute" 64-bit sequence number (zero-indexed) into a WrappingInt32
//! \param n The input absolute 64-bit sequence number
//! \param isn The initial sequence number
WrappingInt32 wrap(uint64_t n, WrappingInt32 isn) {
    const uint64_t mod = 1ULL << 32;

    return WrappingInt32{
        static_cast<uint32_t>((static_cast<uint32_t>(n) + isn.raw_value()) % mod)
    };
}

//! Transform a WrappingInt32 into an "absolute" 64-bit sequence number (zero-indexed)
//! \param n The relative sequence number
//! \param isn The initial sequence number
//! \param checkpoint A recent absolute 64-bit sequence number
//! \returns the 64-bit sequence number that wraps to `n` and is closest to `checkpoint`
//!
//! \note Each of the two streams of the TCP connection has its own ISN. One stream
//! runs from the local TCPSender to the remote TCPReceiver and has one ISN,
//! and the other stream runs from the remote TCPSender to the local TCPReceiver and
//! has a different ISN.
uint64_t unwrap(WrappingInt32 n, WrappingInt32 isn, uint64_t checkpoint) {
    const uint64_t mod = 1ULL << 32;
    uint64_t low = ((n.raw_value() + mod - isn.raw_value()) % mod);
    //! checkpoint 为0 时，不可能向前减去 mod， 所以范围是[0, 2^32 - 1]， 正好就是low的范围
    if(checkpoint == 0) return low;
    
    uint64_t mid = checkpoint & (static_cast<uint64_t>(~(mod - 1)));
    // ! 注意这里的取值范围 [ )
    const __int128_t mod128 = static_cast<__int128_t>(1) << 64;
    uint64_t offset = static_cast<uint64_t>((static_cast<__uint128_t>(mid) + static_cast<__uint128_t>(low) + mod128 - static_cast<__uint128_t>(checkpoint)) % mod128);
    
    if(offset < static_cast<uint64_t>(1ULL << 31) || offset >= static_cast<uint64_t>(mod128 - static_cast<__uint128_t>(1ULL << 31))){
        #ifdef LOG
            dbg(mid + low);
        #endif
        return mid + low;
    }else if(offset > static_cast<uint64_t>(1ULL << 31) && offset < static_cast<uint64_t>(1ULL << 48)){
        #ifdef LOG
            dbg(mid + low - mod);
        #endif
        return mid + low - mod;
    }else{
        #ifdef LOG
            dbg(mid + low + mod);
        #endif
        return mid + low + mod;
    }
}