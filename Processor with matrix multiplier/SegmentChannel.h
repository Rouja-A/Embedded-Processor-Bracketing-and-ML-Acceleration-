#pragma once
#include <systemc.h>
#include <queue>
#include <array>
#include "SegmentInterfaces.h"

class SegmentChannel : public sc_module, public SegmentPutInterface, public SegmentGetInterface {
public:
    sc_event data_available;
    std::queue<std::array<sc_lv<8>, 9>> queue;
    const size_t max_depth = 4;

    void put(sc_lv<8> segment[9]) override {
        while (queue.size() >= max_depth) wait(data_available);
        std::array<sc_lv<8>, 9> temp;
        for (int i = 0; i < 9; ++i) temp[i] = segment[i];
        queue.push(temp);
        data_available.notify(SC_ZERO_TIME);
    }

    void get(sc_lv<8> segment[9]) override {
        while (queue.empty()) wait(data_available);
        std::array<sc_lv<8>, 9> temp = queue.front(); queue.pop();
        for (int i = 0; i < 9; ++i) segment[i] = temp[i];
        data_available.notify(SC_ZERO_TIME);
    }

    SC_CTOR(SegmentChannel) {}
};
