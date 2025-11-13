.data
inputs:     .byte 12, 23, 34
weights:    .byte 1,4,7,10,13,16,19,22,2,5,8,11,14,17,20,23, 6,9,12,15,18,21,24   # weights[2][i]
newline:    .string "\n"

.text
.globl main
main:
    la t0, inputs      # t0 = base addr of inputs
    la t1, weights     # t1 = base addr of weights
    li t2, 0           # t2 = i (column index)

col_loop:
    li t3, 0           # t3 = sum = 0
    li t4, 0           # t4 = j (row index)

row_loop:
    add t5, t0, t4     # t5 = &inputs[j]
    lb t6, 0(t5)       # t6 = inputs[j]

    li t5, 8
    mul t5, t4, t5     # t5 = j * 8
    add t5, t5, t2     # t5 = j*8 + i
    add t5, t5, t1     # t5 = &weights[j][i]
    lb t5, 0(t5)       # t5 = weights[j][i]

    mul t5, t5, t6     # t5 = inputs[j] * weights[j][i]
    add t3, t3, t5     # sum += product

    addi t4, t4, 1     # j++
    li t6, 3
    blt t4, t6, row_loop

    mv a0, t3          # move result to a0
    li a7, 1
    ecall              # print int

    la a0, newline
    li a7, 4
    ecall              # print newline

    addi t2, t2, 1     # i++
    li t6, 8
    blt t2, t6, col_loop

    li a7, 10
    ecall              # exit
