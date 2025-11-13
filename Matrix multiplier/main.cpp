#include <systemc.h>
#include "MMA.h"
#include "MMAInterface.h"
#include "MMAInputChannel.h"

SC_MODULE(Testbench) {
    sc_out<sc_logic> clk;
    sc_out<sc_logic> startMMA;
    sc_in<sc_logic> intrMMA;

    void generate_clock() {
        while (true) {
            clk.write(SC_LOGIC_0); wait(5, SC_NS);
            clk.write(SC_LOGIC_1); wait(5, SC_NS);
        }
    }

    void drive_inputs() {
        wait(10, SC_NS);
        startMMA.write(SC_LOGIC_1);
        wait(10, SC_NS);
        startMMA.write(SC_LOGIC_0);

        while (intrMMA.read() != SC_LOGIC_1) wait(10, SC_NS);
        sc_stop();
    }

    SC_CTOR(Testbench) {
        SC_THREAD(generate_clock);
        SC_THREAD(drive_inputs);
    }
};

int sc_main(int argc, char* argv[]) {
    sc_signal<sc_logic> clk, startMMA, intrMMA;
    sc_signal<sc_lv<18>> result[2];
    Testbench tb("TB");
    tb.clk(clk);
    tb.startMMA(startMMA);
    tb.intrMMA(intrMMA);

    MMAInputChannel input_channel("InputChannel");
    MatrixMultiplierAccelerator accelerator("Accelerator");

    accelerator.clk(clk);
    accelerator.startMMA(startMMA);
    accelerator.intrMMA(intrMMA);
    for (int i = 0; i < 2; ++i)
        accelerator.result[i](result[i]);
    accelerator.input_port(input_channel);

    sc_trace_file* tf = sc_create_vcd_trace_file("mma_trace");
    tf->set_time_unit(1, SC_NS);

    sc_trace(tf, clk, "clk");
    sc_trace(tf, startMMA, "startMMA");
    sc_trace(tf, intrMMA, "intrMMA");

    for (int i = 0; i < 2; ++i) {
        std::string name = "result_" + std::to_string(i);
        sc_trace(tf, result[i], name.c_str());
    }

    for (int i = 0; i < 3; ++i) {
        std::string name = "mma_A_" + std::to_string(i);
        sc_trace(tf, accelerator.A_internal[i], name);
    }

    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 2; ++j) {
            std::string name = "mma_B_" + std::to_string(i) + "_" + std::to_string(j);
            sc_trace(tf, accelerator.B_internal[i][j], name);
        }
    }

    sc_start();
    sc_close_vcd_trace_file(tf);
    return 0;
}
