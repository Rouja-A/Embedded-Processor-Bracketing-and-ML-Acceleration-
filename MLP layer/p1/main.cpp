#include "systemc.h"
#include "mlp_layer.h"

SC_MODULE(Testbench) {
    sc_signal<sc_logic> clk;
    sc_signal<sc_logic> start;
    sc_signal<sc_logic> ready;

    sc_trace_file* tf;

    MLPLayer* mlp;

    void clock_gen() {
        while (true) {
            clk.write(SC_LOGIC_0);
            wait(0.001, SC_NS);
            clk.write(SC_LOGIC_1);
            wait(0.001, SC_NS);
        }
    }

    void stimulus() {
        start.write(SC_LOGIC_0);
        wait(10, SC_NS);

        start.write(SC_LOGIC_1);
        wait(10, SC_NS);

        start.write(SC_LOGIC_0);
        wait(100, SC_NS);

        sc_stop();
    }

    SC_CTOR(Testbench) {
        mlp = new MLPLayer("MLP");
        mlp->clk(clk);
        mlp->start(start);
        mlp->ready(ready);

        tf = sc_create_vcd_trace_file("waveform");
        sc_trace(tf, clk, "clk");
        sc_trace(tf, start, "start");
        sc_trace(tf, ready, "ready");
        for (int i = 0; i < 3; ++i) {
            std::string name = "input_" + std::to_string(i);
            sc_trace(tf, mlp->input_s[i], name);
        }
        for (int i = 0; i < 3; ++i) {
            for (int j = 0; j < 8; ++j) {
                std::string name = "weight_" + std::to_string(i) + "_" + std::to_string(j);
                sc_trace(tf, mlp->weight_s[i][j], name);
            }
        }
        for (int i = 0; i < 8; ++i) {
            std::string name = "output_" + std::to_string(i);
            sc_trace(tf, mlp->output_s[i], name);
        }

        SC_THREAD(clock_gen);
        SC_THREAD(stimulus);
    }

    ~Testbench() {
        sc_close_vcd_trace_file(tf);
        delete mlp;
    }
};

int sc_main(int argc, char* argv[]) {
    Testbench tb("tb");
    sc_start();
    return 0;
}