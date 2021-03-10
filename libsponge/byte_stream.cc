#include "byte_stream.hh"
#include <iostream>
using namespace std;
// Dummy implementation of a flow-controlled in-memory byte stream.

// For Lab 0, please replace with a real implementation that passes the
// automated checks run by `make check_lab0`.

// You will need to add private members to the class declaration in `byte_stream.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

ByteStream::ByteStream(const size_t capacity) : stream(string()),
    _capacity(capacity), isEnded(false), writtenBytes(0), readBytes(0) {}

size_t ByteStream::write(const string &data) {
    size_t left = _capacity - stream.size();
    if(left >= data.size()){
        stream += data;
        writtenBytes += data.size();
        return data.size();
    }else{
        stream += data.substr(0, left);
        writtenBytes += left;
        return left;
    }
}

//! \param[in] len bytes will be copied from the output side of the buffer
string ByteStream::peek_output(const size_t len) const {
    if(len <= stream.size()){
        return stream.substr(0, len);
    }else {
        return stream;
    }
}

//! \param[in] len bytes will be removed from the output side of the buffer
void ByteStream::pop_output(const size_t len) { 
    if(len <= stream.size()){
        readBytes += len;
        stream.erase(0, len);
    }else {
        readBytes += stream.size();
        stream.clear();
    }
}

//! Read (i.e., copy and then pop) the next "len" bytes of the stream
//! \param[in] len bytes will be popped and returned
//! \returns a string
std::string ByteStream::read(const size_t len) {
    auto s = peek_output(len);
    pop_output(len);
    return s;
}

void ByteStream::end_input() { isEnded = true; }

bool ByteStream::input_ended() const { return isEnded; }

size_t ByteStream::buffer_size() const { return stream.size(); }

bool ByteStream::buffer_empty() const { return stream.empty(); }

bool ByteStream::eof() const { return stream.empty() && isEnded; }

size_t ByteStream::bytes_written() const { return writtenBytes; }

size_t ByteStream::bytes_read() const { return readBytes; }

size_t ByteStream::remaining_capacity() const { return _capacity - stream.size(); }
