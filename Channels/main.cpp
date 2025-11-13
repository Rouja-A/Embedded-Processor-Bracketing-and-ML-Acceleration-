#include <systemc.h>
#include <fstream>
#include "SegmentInterfaces.h"
#include "SegmentChannel.h"
#include "MMA.h"

SC_MODULE(Testbench) {
    sc_signal<sc_logic> clk;
    sc_signal<sc_logic> startMMA, intrMMA;

    SegmentChannel inputChannel{"InputChannel"};
    SegmentChannel outputChannel{"OutputChannel"};

    MatrixMultiplierAccelerator mma{"MMA"};

    SC_CTOR(Testbench) {
        mma.clk(clk);
        mma.startMMA(startMMA);
        mma.intrMMA(intrMMA);
        mma.input_channel(inputChannel);
        mma.output_channel(outputChannel);

        SC_THREAD(clock_gen);
        SC_THREAD(run);
    }

    void clock_gen() {
        while (true) {
            clk.write(SC_LOGIC_0);
            wait(5, SC_NS);
            clk.write(SC_LOGIC_1);
            wait(5, SC_NS);
        }
    }

    sc_uint<18> reconstruct(sc_lv<8> segment[], int start_idx) {
    sc_uint<18> val = 0;
    val.range(7, 0) = segment[start_idx].to_uint();
    val.range(15, 8) = segment[start_idx + 1].to_uint();
    val[16] = segment[start_idx + 2].to_uint() & 1;
    return val;
}


    void run() {
        wait(10, SC_NS); 

        std::ifstream fin("input.txt");
        if (!fin) {
            std::cerr << "Failed to open input.txt\n";
            sc_stop();
            return;
        }

        sc_lv<8> segment[9];
        for (int i = 0; i < 9; ++i) {
            int val;
            fin >> val;
            segment[i] = sc_lv<8>(val);
        }
        fin.close();
        sc_lv<8> segmentA[9], segmentB[9];
        for (int i = 0; i < 3; ++i) {
            segmentA[i] = segment[i];
            segmentB[i] = "00000000";
        }
        for (int i = 0; i < 6; ++i) {
            segmentB[i] = segment[i + 3];
        }
        for (int i = 3; i < 9; ++i) {
            segmentA[i] = "00000000";
        }
        for (int i = 6; i < 9; ++i) {
            segmentB[i] = "00000000";
        }

        // Two consecutive puts
        std::cout << "[TB] Sending segmentA...\n";
        inputChannel.put(segmentA);
        std::cout << "[TB] Sending segmentB...\n";
        inputChannel.put(segmentB);



        startMMA.write(SC_LOGIC_1);
        wait(10, SC_NS);
        startMMA.write(SC_LOGIC_0);

        while (true) {
        wait(10, SC_NS);
        std::cout << "[TB] intrMMA = " << intrMMA.read() << "\n";
        if (intrMMA.read() == SC_LOGIC_1)
            break;
}

        sc_lv<8> result_segment[9];
        outputChannel.get(result_segment);
        sc_uint<18> res0 = reconstruct(result_segment, 0);
        sc_uint<18> res1 = reconstruct(result_segment, 3);

        std::cout << "Result[0] = " << res0.to_uint() << "\n";
        std::cout << "Result[1] = " << res1.to_uint() << "\n";

        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {
    Testbench tb("tb");

    sc_trace_file* tf = sc_create_vcd_trace_file("mma_test");
    sc_trace(tf, tb.clk, "clk");
    sc_trace(tf, tb.startMMA, "startMMA");
    sc_trace(tf, tb.intrMMA, "intrMMA");
    
    for (int i = 0; i < 3; ++i)
        sc_trace(tf, tb.mma.A[i], "A_" + std::to_string(i));
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 2; ++j)
            sc_trace(tf, tb.mma.B[i][j], "B_" + std::to_string(i) + "_" + std::to_string(j));
    for (int i = 0; i < 2; ++i)
        sc_trace(tf, tb.mma.result[i], "result_" + std::to_string(i));

    sc_start();
    sc_close_vcd_trace_file(tf);
    return 0;
}
