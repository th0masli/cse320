#include <stdio.h>

/*convert binary number to decimal*/
int bin_dec(int bin) {
    int tmp_bit, dec = 0, power = 1; /*current bit; decimal result; bit power*/
    while (bin) {
        tmp_bit = bin & 1;
        dec += tmp_bit * power;
        power *= 2;
        bin = bin >> 1;
    }

    return dec;
}