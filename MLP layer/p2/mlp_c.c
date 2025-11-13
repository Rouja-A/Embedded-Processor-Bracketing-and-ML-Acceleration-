#include <stdint.h>

uint8_t inputs[3] = {12, 23, 34};
uint8_t weights[3][8] = {
    {1, 4, 7, 10, 13, 16, 19, 22},
    {2, 5, 8, 11, 14, 17, 20, 23},
    {3, 6, 9, 12, 15, 18, 21, 24}
};
uint32_t outputs[8];

int main() {
    for (int i = 0; i < 8; ++i) {
        outputs[i] = 0;
        for (int j = 0; j < 3; ++j) {
            outputs[i] += inputs[j] * weights[j][i];
        }
    }
    return 0;
}
