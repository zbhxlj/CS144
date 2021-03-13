#include "tcp_receiver.hh"
// #define LOG

#ifdef LOG
#include <dbg.h>
#endif
// Dummy implementation of a TCP receiver

// For Lab 2, please replace with a real implementation that passes the
// automated checks run by `make check_lab2`.

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

void TCPReceiver::segment_received(const TCPSegment &seg) {
    auto header = seg.header();

    if(header.syn){
        SYNIsSet = true;
        SYN = header.seqno;
    }

    auto payload = seg.payload();
    if(SYNIsSet){
        auto data = payload.copy();
        
        //! 此时还没有 SYN 的影响, push_substring 的第二个参数是bytestream中下标, 不计算SYN 和 FIN
        if(header.syn){
            _reassembler.push_substring(data, unwrap(header.seqno, SYN, checkPoint), header.fin);
            ACK = wrap(_reassembler.stream_out().bytes_written() + 1 + (header.fin ? 1 : 0), SYN);
            return;
        }
        _reassembler.push_substring(data, unwrap(WrappingInt32{header.seqno.raw_value() - 1}, SYN, checkPoint), header.fin);
        //! 没有用上 tcp_segment.length_in_sequence_space(), 我认为有可能乱序会影响
        #ifdef LOG
            dbg(_reassembler.stream_out().bytes_written());
        #endif
        checkPoint = _reassembler.stream_out().bytes_written();
        #ifdef LOG
            dbg(checkPoint);
        #endif
        //! 加上FIN
        if(_reassembler.stream_out().input_ended()) checkPoint += 1;
        #ifdef LOG
            dbg(checkPoint);
        #endif
        ACK = wrap(checkPoint + 1, SYN);
        #ifdef LOG
            dbg(ACK);
        #endif
    }
}

optional<WrappingInt32> TCPReceiver::ackno() const { 
    if(!SYNIsSet)
        return {};
    else return WrappingInt32{ACK};
}

size_t TCPReceiver::window_size() const { return _capacity - _reassembler.stream_out().buffer_size();}
