#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include "K6502.h"
#include "InfoNES.h"
#include "InfoNES_System.h"
#include "InfoNES_Mapper.h"
#include "InfoNES_pAPU.h"
#include "nes_board.h"
#include "nes_audio.h"

#include "nes_game.h"

#define SELECT_GAME     NES_GAME_INDEX            




// Palette data
WORD NesPalette[ 64 ] =
{
    0x39ce, 0x1071, 0x0015, 0x2013, 0x440e, 0x5402, 0x5000, 0x3c20,
    0x20a0, 0x0100, 0x0140, 0x00e2, 0x0ceb, 0x0000, 0x0000, 0x0000,
    0x5ef7, 0x01dd, 0x10fd, 0x401e, 0x5c17, 0x700b, 0x6ca0, 0x6521,
    0x45c0, 0x0240, 0x02a0, 0x0247, 0x0211, 0x0000, 0x0000, 0x0000,
    0x7fff, 0x1eff, 0x2e5f, 0x223f, 0x79ff, 0x7dd6, 0x7dcc, 0x7e67,
    0x7ae7, 0x4342, 0x2769, 0x2ff3, 0x03bb, 0x0000, 0x0000, 0x0000,
    0x7fff, 0x579f, 0x635f, 0x6b3f, 0x7f1f, 0x7f1b, 0x7ef6, 0x7f75,
    0x7f94, 0x73f4, 0x57d7, 0x5bf9, 0x4ffe, 0x0000, 0x0000, 0x0000
    
};


void nesStart(void)
{
    WorkFrame = (WORD *)NES_WORK_FRAME_ADDR;
    
    if(0 != InfoNES_Load("embedded")) {

        NES_BoardFatal(5);
    }
    
    InfoNES_Main();
}



/*===================================================================*/
/*                                                                   */
/*                  InfoNES_Menu() : Menu screen                     */
/*                                                                   */
/*===================================================================*/
int InfoNES_Menu()
{
/*
 *  Menu screen
 *
 *  Return values
 *     0 : Normally
 *    -1 : Exit InfoNES
 */

  if ( PAD_PUSH( PAD_System, PAD_SYS_QUIT) )
    return -1;	 	

  // Nothing to do here
  return 0;
}


/*===================================================================*/
/*                                                                   */
/*               InfoNES_ReadRom() : Read ROM image file             */
/*                                                                   */
/*===================================================================*/
/* CPU-only cached SDRAM, initialized only after NES_BoardInit. */
static BYTE nes_rom_storage[1024 * 1024]
    __attribute__((section(".nes_rom"), aligned(32)));

int InfoNES_ReadRom(const char *pszFileName)
{
    (void)pszFileName;
    ROM = NULL;
    VROM = NULL;
    pNesGame game = &nes_game[SELECT_GAME];
    game->index = 0;
    if (nesReadFile(&NesHeader, sizeof NesHeader, 1, game) != 0 ||
        memcmp(NesHeader.byID, "NES\x1a", 4) != 0 ||
        (NesHeader.byInfo2 & 0x0c) == 0x08) return -1; /* NES 2.0 unsupported */
    size_t prg_size = (size_t)NesHeader.byRomSize * 0x4000;
    size_t chr_size = (size_t)NesHeader.byVRomSize * 0x2000;
    if (!prg_size || prg_size + chr_size > sizeof nes_rom_storage) return -1;
    memset(SRAM, 0, SRAM_SIZE);
    if ((NesHeader.byInfo1 & 4) && nesReadFile(&SRAM[0x1000], 512, 1, game) != 0)
        return -1;
    if (nesReadFile(nes_rom_storage, prg_size, 1, game) != 0) return -1;
    if (chr_size && nesReadFile(nes_rom_storage + prg_size, chr_size, 1, game) != 0)
        return -1;
    ROM = nes_rom_storage;
    if (chr_size) VROM = nes_rom_storage + prg_size;
    return 0;
}

void InfoNES_ReleaseRom(void)
{
    /* Statically reserved SDRAM must never be passed to free(). */
    ROM = NULL;
    VROM = NULL;
}

/*===================================================================*/
/*                                                                   */
/*      InfoNES_LoadFrame() :                                        */
/*           Transfer the contents of work frame on the screen       */
/*                                                                   */
/*===================================================================*/
void InfoNES_LoadFrame(void)
{
    NES_BoardPresent(WorkFrame);
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_PadState() : Get a joypad state               */
/*                                                                   */
/*===================================================================*/
void InfoNES_PadState(DWORD *pdwPad1, DWORD *pdwPad2, DWORD *pdwSystem)
{
    *pdwPad1 = NES_BoardReadPad();
    *pdwPad2 = 0;
    *pdwSystem = 0;
}

/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemoryCopy() : memcpy                         */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemoryCopy( void *dest, const void *src, int count )
{
/*
 *  memcpy
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the copied block's destination
 *
 *    const void *src                  (Read)
 *      Points to the starting address of the block of memory to copy
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to copy
 *
 *  Return values
 *    Pointer of destination
 */

  memcpy( dest, src, count );
  return dest;
}


/*===================================================================*/
/*                                                                   */
/*             InfoNES_MemorySet() : memset                          */
/*                                                                   */
/*===================================================================*/
void *InfoNES_MemorySet( void *dest, int c, int count )
{
/*
 *  memset
 *
 *  Parameters
 *    void *dest                       (Write)
 *      Points to the starting address of the block of memory to fill
 *
 *    int c                            (Read)
 *      Specifies the byte value with which to fill the memory block
 *
 *    int count                        (Read)
 *      Specifies the size, in bytes, of the block of memory to fill
 *
 *  Return values
 *    Pointer of destination
 */

  memset( dest, c, count);  
  return dest;
}


/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundInit() : Sound Emulation Initialize           */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundInit( void ) 
{
NES_AudioReset();
}



/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundOpen() : Sound Open                           */
/*                                                                   */
/*===================================================================*/
int InfoNES_SoundOpen(int samples_per_sync, int sample_rate)
{
    return NES_AudioOpen(samples_per_sync, sample_rate);
}

/*===================================================================*/
/*                                                                   */
/*        InfoNES_SoundClose() : Sound Close                         */
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundClose(void)
{
    NES_AudioClose();
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_SoundOutput() : Sound Output 5 Waves           */           
/*                                                                   */
/*===================================================================*/
void InfoNES_SoundOutput(int samples, BYTE *wave1, BYTE *wave2, BYTE *wave3, BYTE *wave4, BYTE *wave5)
{
    NES_AudioOutput(samples, wave1, wave2, wave3, wave4, wave5);
}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_Wait() : Wait Emulation if required            */
/*                                                                   */
/*===================================================================*/
void InfoNES_Wait() {}

/*===================================================================*/
/*                                                                   */
/*            InfoNES_MessageBox() : Print System Message            */
/*                                                                   */
/*===================================================================*/
void InfoNES_MessageBox( char *pszMsg, ... )
{
//    va_list args;
//    va_start( args, pszMsg );
//    printf( pszMsg, args );	
//    va_end( args );
}


