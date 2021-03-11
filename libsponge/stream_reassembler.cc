#include "stream_reassembler.hh"
#include <assert.h>
// #define LOG

#ifdef LOG
#include <dbg.h>
#endif

// Dummy implementation of a stream reassembler.

// For Lab 1, please replace with a real implementation that passes the
// automated checks run by `make check_lab1`.

// You will need to add private members to the class declaration in `stream_reassembler.hh`

template <typename... Targs>
void DUMMY_CODE(Targs &&... /* unused */) {}

using namespace std;

StreamReassembler::StreamReassembler(const size_t capacity) : 
_lastIndex(INT8_MIN), _unassembledStrings{}, _unassembledSize(0), _expectedNextByte(0), _output(capacity), _capacity(capacity) {}

//! \details This function accepts a substring (aka a segment) of bytes,
//! possibly out-of-order, from the logical stream, and assembles any newly
//! contiguous substrings and writes them into the output stream in order.
void StreamReassembler::push_substring(const string &data, const size_t index, const bool eof) {
    if(eof) _lastIndex = index + data.size() - 1;
    
    //! 我认为data为空时候
    //! 1.eof为真时, index的合理值应该是当前发来的所有子串中序号的最大值, 此时负责提供一个eof的信号
    //! 2.eof为假时, 那就没什么意义
    //? data 为空时候进入后面逻辑可能不方便处理, 这里直接处理掉
    if(data == ""){
        if(_lastIndex == _expectedNextByte - 1 && _unassembledStrings.empty()){
            _output.end_input();
            return;
        }
        //! 对于第二种情况没做处理
    }

    //! 处理一下data, 使之不与已经排序的子串重叠
    string _data = data;
    size_t _index = index;
    if(index < _expectedNextByte){
        size_t overlappedCount =  _expectedNextByte - index;
        if(_data.size() <= overlappedCount) return;
        _data = _data.substr(overlappedCount);
        _index = index + overlappedCount;
    }

    #ifdef LOG
        dbg("Before function mergeSubstrings");
        dbg(_data);
    #endif
    mergeSubstrings(_data, _index);
    #ifdef LOG
        dbg("After function mergeSubstrings");
        dbg(_unassembledStrings);
        size_t outputSize = _capacity - _output.remaining_capacity();
        dbg(outputSize);
        dbg(outputSize + _unassembledSize);
    #endif

    
    while(!_unassembledStrings.empty()){
        auto front = _unassembledStrings.front();
        if(front.first == _expectedNextByte){
        //! 不必做返回值处理, 因为必定能全部写入
        _expectedNextByte += front.second.size();
        _unassembledStrings.pop_front();
        _unassembledSize -= front.second.size();
        #ifdef LOG
            dbg("After write");
            dbg(_expectedNextByte);
            dbg(_unassembledStrings);
        #endif
        }else break;
    }
    
    if(_lastIndex == _expectedNextByte -1) _output.end_input();
    return;
}

void StreamReassembler::mergeSubstrings(const string& data, const size_t index){
    //! 可以改成二分
    //! 先处理两种特殊情况 1 _unassembledStrings为空  2 index比所有的都大 (都无法找到下一行所要求的条件)
    if(_unassembledStrings.empty() 
                || _unassembledStrings.back().first + _unassembledStrings.back().second.size() - 1 < index){
        //! 直接从尾端插入就可
        // #ifdef LOG
        //     dbg(data);
        //     dbg(_unassembledSize);
        // #endif
        _unassembledStrings.push_back(make_pair(index, data));
        _unassembledSize += data.size();
    }else{
        //! 找到第一个substring使得index小于等于substring的最后一个元素的序号
        //! 有两种可能, 不相交或者相交
        //! 不相交, 直接插入 
        //! xxxx
        //!        xxxxx
        //! 相交
        //! xxxxx         xxxxxx           xxxx     xxx
        //!    xxxxx        xxx       xxxxxx       xxxxxx
        

        for(auto it = _unassembledStrings.begin(); it != _unassembledStrings.end(); it++){
            auto& currentInterval = *it;
            size_t rightIndex = currentInterval.first + currentInterval.second.size() -1; 
            if(index <= rightIndex){
                size_t dataRightIndex = index + data.size() - 1;
                //! 不相交时
                if(dataRightIndex < currentInterval.first){
                    _unassembledStrings.insert(it, make_pair(index, data));
                    _unassembledSize += data.size();
                    break;
                }else{
                    //! 相交
                    size_t newIndex = min(index, currentInterval.first);
                    size_t newRightIndex = max(dataRightIndex, rightIndex);
                    size_t overlapCount = currentInterval.second.size() + data.size() - (newRightIndex - newIndex + 1);
                    //! 不要忘记删除旧的substring
                    _unassembledSize -= currentInterval.second.size();
                    _unassembledStrings.erase(it);
                    //! 构造新的data和index, 递归插入
                    string newData;
                    if(newIndex == index){
                        if(newRightIndex == dataRightIndex) newData = data;
                        else newData = data.substr(0, data.size() - overlapCount) + currentInterval.second;
                    }else {
                        if(newRightIndex == dataRightIndex) newData = currentInterval.second + data.substr(overlapCount);
                        else newData = currentInterval.second;
                    }
                    mergeSubstrings(newData, newIndex);
                    break;
                }
            }    
        }
    }
    //! 此时已经插好了, 应当保证capacity, 因为后面只可能转化为unread, 不会减少size
    size_t outputSize = _capacity - _output.remaining_capacity();
    size_t currentSize = outputSize + _unassembledSize;
    if(currentSize <= _capacity) return;
    #ifdef LOG
        dbg(outputSize);
        dbg(_unassembledSize);
        dbg(currentSize);
    #endif
    
    while(!_unassembledStrings.empty() && currentSize > _capacity){
        auto last = _unassembledStrings.back();
        _unassembledStrings.pop_back();
        _unassembledSize -= last.second.size();
        currentSize -= last.second.size();

        if(currentSize < _capacity){
            _unassembledStrings.push_back(make_pair(last.first, last.second.substr(0, _capacity - currentSize)));
            _unassembledSize += _capacity - currentSize;
            currentSize = _capacity;
        }
    }
    #ifdef LOG
        dbg("before return");
        dbg(_unassembledStrings);
        dbg(currentSize);
        dbg(_unassembledSize);
    #endif
    return;
}

size_t StreamReassembler::unassembled_bytes() const { return _unassembledSize; }

bool StreamReassembler::empty() const { return _unassembledStrings.empty(); }
