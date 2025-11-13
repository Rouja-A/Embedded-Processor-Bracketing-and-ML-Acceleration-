#pragma once
#include <systemc.h>

class MMAInputInterface : public virtual sc_interface {
public:
    virtual void read_blocks(sc_lv<8> A[3], sc_lv<8> B[3][2]) = 0;
};
