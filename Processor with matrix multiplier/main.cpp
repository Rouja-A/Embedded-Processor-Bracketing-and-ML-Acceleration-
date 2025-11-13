#include <systemc.h>
#include "SegmentChannel.h"
#include "MMA.h"
#include "MLPlayer.h"

SC_MODULE(Testbench) {
    sc_signal<sc_logic> clk, start, ready;
    sc_signal<sc_logic> startMMA, intrMMA;

    SegmentChannel input_channel{"InputChannel"};
    SegmentChannel output_channel{"OutputChannel"};

    MLPLayer mlp{"MLP"};
    MatrixMultiplierAccelerator mma{"MMA"};

    SC_CTOR(Testbench) {

        mlp.clk(clk);
        mlp.start(start);
        mlp.ready(ready);
        mlp.put_port(input_channel);
        mlp.get_port(output_channel);
        mlp.startMMA(startMMA);
        mlp.intrMMA(intrMMA);

        mma.clk(clk);
        mma.startMMA(startMMA);
        mma.intrMMA(intrMMA);
        mma.input_channel(input_channel);
        mma.output_channel(output_channel);

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

    void run() {
        wait(10, SC_NS);

        std::cout << "[TB] Triggering MLP...\n";
        start.write(SC_LOGIC_1);
        wait(10, SC_NS);
        start.write(SC_LOGIC_0);

        while (ready.read() != SC_LOGIC_1) {
            wait(10, SC_NS);
        }

        std::cout << "[TB] MLP processing complete.\n";
        std::cout << "[TB] Total MLP cycles: " << mlp.get_total_cycles() << "\n";
        std::cout << "[TB] Total MMA cycles: " << mma.get_total_cycles() << "\n";
        std::cout << "[TB] Combined total cycles: " << mma.get_total_cycles() << "\n";

        sc_stop();
    }
};

int sc_main(int argc, char* argv[]) {
    Testbench tb("tb");

    sc_trace_file* tf = sc_create_vcd_trace_file("mlp_mma_test");
    sc_trace(tf, tb.clk, "clk");
    sc_trace(tf, tb.start, "start");
    sc_trace(tf, tb.ready, "ready");
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
