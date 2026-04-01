//
// Created by gcaptari on 2/27/26.
//

long klpow(long base, long exp) {
    if (exp < 0) {
        return 0; // For simplicity, we return 0 for negative exponents
    }
    int result = 1;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}