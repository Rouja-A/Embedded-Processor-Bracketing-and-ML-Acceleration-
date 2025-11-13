#pragma once
#include <systemc.h>

class SegmentPutInterface : public virtual sc_interface {
public:
    virtual void put(sc_lv<8> segment[9]) = 0;
};

class SegmentGetInterface : public virtual sc_interface {
public:
    virtual void get(sc_lv<8> segment[9]) = 0;
};
