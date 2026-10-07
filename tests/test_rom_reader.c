/* Host test: gcc -ffunction-sections -fdata-sections tests/test_rom_reader.c
 * nes/games/nes_game.c -Wl,--gc-sections -o /tmp/test_rom_reader && /tmp/test_rom_reader */
#include "../nes/games/nes_game.h"
#include <assert.h>
#include <limits.h>
int main(void) {
    const unsigned char data[] = {1,2,3,4};
    unsigned char out[4] = {0};
    const nesGameFile file = {"test", data, data + sizeof data};
    nesGame game = {&file, 0};
    assert(nesReadFile(out, 2, 2, &game) == 0);
    assert(game.index == 4 && memcmp(out, data, 4) == 0);
    assert(nesReadFile(out, 1, 1, &game) == -1 && game.index == 4);
    game.index = 0;
    assert(nesReadFile(out, UINT_MAX, 2, &game) == -1 && game.index == 0);
    assert(nesReadFile(out, 5, 1, &game) == -1);
    game.index = 5;
    assert(nesReadFile(out, 0, 0, &game) == -1);
    assert(nesReadFile(out, 1, 1, 0) == -1);
    return 0;
}
