#pragma once
#include <systemc.h>
#include "SegmentInterfaces.h"

SC_MODULE(MatrixMultiplierAccelerator) {
    sc_in<sc_logic> clk;
    sc_in<sc_logic> startMMA;
    sc_out<sc_logic> intrMMA;

    sc_port<SegmentGetInterface> input_channel;
    sc_port<SegmentPutInterface> output_channel;

    sc_signal<sc_uint<8>> A[3];
    sc_signal<sc_uint<8>> B[3][2];
    sc_signal<sc_uint<18>> result[2];

    void process() {
        intrMMA.write(SC_LOGIC_0);

        while (true) {
            wait();
            if (startMMA.read() == SC_LOGIC_1) {
                sc_lv<8> segment[9];
                sc_lv<8> segmentA[9], segmentB[9];
                std::cout << "[MMA] Waiting for segmentA...\n";
                input_channel->get(segmentA);
                std::cout << "[MMA] Received segmentA\n";

                std::cout << "[MMA] Waiting for segmentB...\n";
                input_channel->get(segmentB);
                std::cout << "[MMA] Received segmentB\n";


                for (int i = 0; i < 3; ++i){
                    A[i].write(segmentA[i].to_uint());
                    //wait();
                }
                wait();

                int idx = 0;
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 2; ++j) {
                        B[i][j].write(segmentB[idx++].to_uint());
                        //wait();
                    }
                }
                wait();

                for (int j = 0; j < 2; ++j) {
                    sc_uint<18> sum = 0;
                    for (int k = 0; k < 3; ++k)
                        sum += A[k].read() * B[k][j].read();
                    result[j].write(sum);
                    wait();
                }

                sc_lv<8> out_segment[9];
                out_segment[0] = result[0].read().range(7, 0);
                out_segment[1] = result[0].read().range(15, 8);
                out_segment[2] = sc_lv<8>(result[0].read()[16] ? 1 : 0);

                out_segment[3] = result[1].read().range(7, 0);
                out_segment[4] = result[1].read().range(15, 8);
                out_segment[5] = sc_lv<8>(result[1].read()[16] ? 1 : 0);

                for (int i = 6; i < 9; ++i)
                    out_segment[i] = "00000000";

                output_channel->put(out_segment);
                intrMMA.write(SC_LOGIC_1);
                wait();

            } else {
                intrMMA.write(SC_LOGIC_0);
            }
        }
    }


    SC_CTOR(MatrixMultiplierAccelerator) {
        SC_THREAD(process);
        sensitive << clk.pos();
    }
};
