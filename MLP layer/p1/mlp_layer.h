#include "systemc.h"
#include <fstream>
#include <vector>
#include <sstream>

SC_MODULE(MLPLayer) {
    sc_in<sc_logic> clk;
    sc_in<sc_logic> start;
    sc_out<sc_logic> ready;

    sc_signal<uint8_t> input_s[3];
    sc_signal<uint8_t> weight_s[3][8];
    sc_signal<uint32_t> output_s[8];

    std::vector<uint8_t> inputs;
    std::vector<std::vector<uint8_t>> weights;
    std::vector<uint32_t> outputs;

    void process_layer() {
        ready.write(SC_LOGIC_0);
        while (true) {
            wait();
            if (start.read() == SC_LOGIC_1) {
                load_inputs("inputs.txt");
                for (int i = 0; i < 3; ++i) {
                    input_s[i].write(inputs[i]);
                }
                load_weights("weights.txt");
                for (int i = 0; i < 3; ++i) {
                    for (int j = 0; j < 8; ++j) {
                        weight_s[i][j].write(weights[i][j]);
                    }
                }
                compute_outputs();
                for (int i = 0; i < 8; ++i) {
                    output_s[i].write(outputs[i]);
                }
                save_outputs("outputs.txt");
                wait(499);
                ready.write(SC_LOGIC_1);
            } else {
                ready.write(SC_LOGIC_0);
            }
        }
    }

    void load_inputs(const std::string& filename) {
        inputs.clear();
        std::ifstream fin(filename);
        std::string line;
        std::getline(fin, line);
        std::stringstream ss(line);
        std::string val;
        while (std::getline(ss, val, ',')) {
            inputs.push_back(std::stoi(val));
        }
    }

    void load_weights(const std::string& filename) {
        weights.clear();
        weights.resize(3, std::vector<uint8_t>(8));
        std::ifstream fin(filename);
        std::string line;
        std::getline(fin, line);
        std::stringstream ss(line);
        std::string val;
        int count = 0;
        while (std::getline(ss, val, ',') && count < 24) {
            int row = count % 3;
            int col = count / 3;
            weights[row][col] = std::stoi(val);
            count++;
        }
    }

    void compute_outputs() {
        outputs.clear();
        outputs.resize(8, 0);
        for (int i = 0; i < 8; ++i) {
            uint32_t sum = 0;
            for (int j = 0; j < 3; ++j) {
                sum += inputs[j] * weights[j][i];
            }
            outputs[i] = sum;
        }
    }

    void save_outputs(const std::string& filename) {
        std::ofstream fout(filename);
        for (int i = 0; i < 8; ++i) {
            fout << outputs[i];
            if (i != 7) fout << ",";
        }
    }

    SC_CTOR(MLPLayer) {
        SC_THREAD(process_layer);
        sensitive << clk.pos();
    }
};