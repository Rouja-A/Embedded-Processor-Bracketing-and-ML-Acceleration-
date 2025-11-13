#pragma once
#include "systemc.h"
#include <fstream>
#include <vector>
#include <sstream>
#include "SegmentInterfaces.h"

SC_MODULE(MLPLayer) {
    sc_in<sc_logic> clk;
    sc_in<sc_logic> start;
    sc_out<sc_logic> ready;

    sc_out<sc_logic> startMMA;
    sc_in<sc_logic> intrMMA;
    sc_port<SegmentPutInterface> put_port;
    sc_port<SegmentGetInterface> get_port;

    std::vector<uint8_t> inputs;
    std::vector<std::vector<uint8_t>> weights;
    std::vector<uint32_t> outputs;

    int total_cycles = 0;

    int get_total_cycles() const {
        return total_cycles;
    }

    void process_layer() {
        ready.write(SC_LOGIC_0);
        startMMA.write(SC_LOGIC_0);

        while (true) {
            wait();
            total_cycles++;

            if (start.read() == SC_LOGIC_1) {
                load_inputs("inputs.txt");
                load_weights("weights.txt");
                outputs.resize(8, 0);

                for (int out_idx = 0; out_idx < 8; out_idx += 2) {
                    sc_lv<8> segmentA[9], segmentB[9];

                    for (int i = 0; i < 3; ++i)
                        segmentA[i] = sc_lv<8>(inputs[i]);

                    int idx = 3;
                    for (int i = 0; i < 3; ++i)
                        for (int j = 0; j < 2; ++j) {
                            int col = out_idx + j;
                            segmentA[idx++] = (col < 8) ? sc_lv<8>(weights[i][col]) : "00000000";
                        }

                    for (int i = 0; i < 3; ++i)
                        segmentB[i] = sc_lv<8>(inputs[i]);

                    idx = 3;
                    for (int i = 0; i < 3; ++i)
                        for (int j = 0; j < 2; ++j) {
                            int col = out_idx + j + 2;
                            segmentB[idx++] = (col < 8) ? sc_lv<8>(weights[i][col]) : "00000000";
                        }

                    put_port->put(segmentA);
                    put_port->put(segmentB);

                    std::cout << "[MLP] Put Segment A for columns " << out_idx << " and " << (out_idx + 1) << "\n";
                    std::cout << "[MLP] Put Segment B for columns " << (out_idx + 2) << " and " << (out_idx + 3) << "\n";

                    startMMA.write(SC_LOGIC_1);
                    wait();
                    total_cycles++;
                    startMMA.write(SC_LOGIC_0);

                    do {
                        wait();
                        total_cycles++;
                    } while (intrMMA.read() != SC_LOGIC_1);

                    sc_lv<8> result_segment[9];
                    get_port->get(result_segment);

                    outputs[out_idx] = unpack_result(result_segment, 0).to_uint();
                    outputs[out_idx + 1] = unpack_result(result_segment, 3).to_uint();
                }
                
                save_outputs("outputs.txt");
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
        std::cout << "[MLP] Loaded inputs: ";
        for (auto val : inputs) std::cout << (int)val << " ";
        std::cout << std::endl;
    }

    void load_weights(const std::string& filename) {
        weights.clear();
        weights.resize(3, std::vector<uint8_t>(8, 0));
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
        std::cout << "[MLP] Loaded weights: " << std::endl;
        for (int i = 0; i < 3; ++i) {
            std::cout << "Row " << i << ": ";
            for (int j = 0; j < 8; ++j)
                std::cout << (int)weights[i][j] << " ";
            std::cout << std::endl;
        }
    }

    void save_outputs(const std::string& filename) {
        std::ofstream fout(filename);
        for (int i = 0; i < outputs.size(); ++i) {
            fout << outputs[i];
            if (i != outputs.size() - 1) fout << ",";
        }
        fout << std::endl;
    }

    sc_uint<18> unpack_result(sc_lv<8> seg[], int idx) {
        sc_uint<18> res;
        res.range(7, 0) = seg[idx].to_uint();
        res.range(15, 8) = seg[idx + 1].to_uint();
        res[16] = seg[idx + 2].to_uint() & 1;
        return res;
    }


    SC_CTOR(MLPLayer) {
        SC_THREAD(process_layer);
        sensitive << clk.pos();
    }
};
