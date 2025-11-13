#pragma once
#include <systemc.h>
#include "MMAInterface.h"

SC_MODULE(MatrixMultiplierAccelerator) {
    sc_in<sc_logic> clk;
    sc_in<sc_logic> startMMA;
    sc_out<sc_logic> intrMMA;

    sc_out<sc_lv<18>> result[2];
    sc_port<MMAInputInterface> input_port;

    sc_signal<sc_lv<8>> A_internal[3];
    sc_signal<sc_lv<8>> B_internal[3][2];
    sc_signal<sc_lv<18>> computed_result[2];

    enum State { IDLE, COMPUTE, OUTPUT };
    State state = IDLE;

    void process() {
        intrMMA.write(SC_LOGIC_0);

        while (true) {
            wait();

            switch (state) {
                case IDLE:
                    if (startMMA.read() == SC_LOGIC_1) {
                        sc_lv<8> A[3];
                        sc_lv<8> B[3][2];
                        input_port->read_blocks(A, B);

                        for (int i = 0; i < 3; ++i)
                            A_internal[i].write(A[i]);
                        for (int i = 0; i < 3; ++i)
                            for (int j = 0; j < 2; ++j)
                                B_internal[i][j].write(B[i][j]);

                        for (int j = 0; j < 2; ++j) {
                            sc_uint<18> sum = 0;
                            for (int k = 0; k < 3; ++k) {
                                sc_uint<8> a = A[k].to_uint();
                                sc_uint<8> b = B[k][j].to_uint();
                                sum += a * b;
                            }
                            computed_result[j].write(sc_lv<18>(sum));
                        }

                        state = OUTPUT;
                    }
                    break;

                case OUTPUT:
                    for (int j = 0; j < 2; ++j)
                        result[j].write(computed_result[j].read());

                    intrMMA.write(SC_LOGIC_1);
                    state = IDLE;
                    break;
            }
        }
    }

    SC_CTOR(MatrixMultiplierAccelerator) {
        SC_THREAD(process);
        sensitive << clk.pos();
    }
};
