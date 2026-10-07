#ifndef __NES_GAME_H
#define __NES_GAME_H

#include <string.h>
//#include <stdlib.h>

#define GAME_FILE_NAME_MAXLEN       20
#define GAME_FILE_NUM               3

typedef struct {
    char gameName[GAME_FILE_NAME_MAXLEN];
    const unsigned char *gameFileSrc;
    const unsigned char *gameFileEnd;
} nesGameFile;


typedef struct {
    const nesGameFile  *gameFile;
    unsigned int  index;
} nesGame, *pNesGame;


typedef struct {
    pNesGame *pGameList;
    pNesGame  pCurrentGame;
} nesGameFIL, pNesGameFIL;


extern nesGame nes_game[];
extern const unsigned char 		SuperMario[];
extern const unsigned char 		yingzichuanshuo[];

extern const unsigned char SuperDodgeBall[];
extern const unsigned char SuperMario_end[], yingzichuanshuo_end[], SuperDodgeBall_end[];

int nesReadFile(void *buf, unsigned int len, unsigned short num, pNesGame png);



#endif
