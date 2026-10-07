#include "nes_game.h"
#include <stdint.h>

const nesGameFile gameFileList[GAME_FILE_NUM] = {
    {"SuperMario", SuperMario, SuperMario_end},
    {"yingzichuanshuo", yingzichuanshuo, yingzichuanshuo_end},
    {"SuperDodgeBall", SuperDodgeBall, SuperDodgeBall_end},
};
nesGame nes_game[GAME_FILE_NUM] = {
    {&gameFileList[0], 0}, {&gameFileList[1], 0}, {&gameFileList[2], 0},
};
int nesReadFile(void *buf, unsigned int len, unsigned short num, pNesGame png)
{
    if (!png || !png->gameFile || !buf) return -1;
    const nesGameFile *file = png->gameFile;
    size_t size = (uintptr_t)file->gameFileEnd - (uintptr_t)file->gameFileSrc;
    if (png->index > size || (num && len > (size - png->index) / num)) return -1;
    size_t bytes = (size_t)len * num;
    memcpy(buf, file->gameFileSrc + png->index, bytes);
    png->index += bytes;
    return 0;
}
