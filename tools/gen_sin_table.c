#include <stdio.h>
#include <math.h>
#include <stdint.h>
#define ENTRIES 256

int main(void) {
    printf("/* GENERATED FILE - do not edit by hand */\n");
    printf("/* sin(x) for x = 0..90 degrees, Q16.16 */\n\n");
    printf("#ifndef SIN_TABLE_H\n#define SIN_TABLE_H\n\n");
    printf("#include <stdint.h>\n\n");
    printf("#define SIN_TABLE_ENTRIES %d\n\n", ENTRIES);
    printf("static const int32_t sin_table[%d] = {\n", ENTRIES + 1);

    for (int i = 0; i <= ENTRIES; i++) {
        double degrees = 90.0 * i / ENTRIES;
        double radians = degrees * M_PI / 180.0;
        double value   = sin(radians);
        int32_t fixed  = (int32_t)(value * 65536.0 + 0.5);

        printf("    %10d,   /* %7.3f deg */\n", fixed, degrees);
    }

    printf("};\n\n#endif\n");
    return 0;
}
