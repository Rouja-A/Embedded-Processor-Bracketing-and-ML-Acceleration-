#pragma once
#include <systemc.h>
#include "MMAInterface.h"

SC_MODULE(MMAInputChannel), public MMAInputInterface {
    sc_lv<8> A_data[3];
    sc_lv<8> B_data[3][2];

    void load_data_from_files() {
        std::ifstream fa("A.txt");
        std::ifstream fb("B.txt");

        for (int i = 0; i < 3; ++i) {
            int val;
            fa >> val;
            A_data[i] = sc_lv<8>(val);
        }

        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 2; ++j) {
                int val;
                fb >> val;
                B_data[i][j] = sc_lv<8>(val);
            }
        }
    }

    void read_blocks(sc_lv<8> A[3], sc_lv<8> B[3][2]) override {
        for (int i = 0; i < 3; ++i)
            A[i] = A_data[i];
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 2; ++j)
                B[i][j] = B_data[i][j];
    }

    SC_CTOR(MMAInputChannel) {
        load_data_from_files();
    }
};
