"""Run after make: python3 tests/verify_image.py [toolchain-bin-directory].
Checks linked layout/vector contents, not physical hardware behavior.
"""
from pathlib import Path
import sys,struct,re,subprocess,xml.etree.ElementTree as ET
p=Path(__file__).resolve().parents[1]
prefix=str(Path(sys.argv[1])/'arm-none-eabi-') if len(sys.argv)>1 else 'arm-none-eabi-'
elf=p/'Debug/STM32F769_Disco_NES.elf'
nm=subprocess.check_output([prefix+'nm','-n',str(elf)],text=True)
symbols={l.split()[2]:int(l.split()[0],16) for l in nm.splitlines() if len(l.split())==3}
b=(elf.with_suffix('.bin')).read_bytes()
assert struct.unpack_from('<I',b)[0]==0x20020000
assert struct.unpack_from('<I',b,4)[0]==symbols['Reset_Handler']|1
assert 0x2007C000<=symbols['dma_pcm'] and symbols['dma_pcm']+5880<=0x20080000
assert symbols['dma_pcm']%32==0
assert 0x20020000<=symbols['queue']<0x2007C000
assert symbols['__heap_start__']==0x20000000
assert symbols['__heap_end__']<=symbols['__stack_limit__']<symbols['_estack']
assert symbols['_ebss']<=0x2007C000
header=(p/'CMSIS/Device/ST/STM32F7xx/Include/stm32f769xx.h').read_text()
for name in ['DMA2_Stream1','SAI1','DSI','LTDC','LTDC_ER','DMA2D']:
 irq=int(re.search(r'\b'+name+r'_IRQn\s*=\s*(\d+)',header)[1])
 assert struct.unpack_from('<I',b,4*(16+irq))[0]==symbols[name+'_IRQHandler']|1,name
for f in ['.project','.cproject']: ET.parse(p/f)
print('PASS: initial SP/reset, DMA SRAM2/alignment, queue SRAM1, DTCM heap/stack, BSS bounds, six IRQ vectors, project XML')

assert symbols['nes_rom_storage'] == 0xC0400000
assert symbols['SuperDodgeBall_end'] - symbols['SuperDodgeBall'] == 786464
rom = (p/'nes/games/SuperDodgeBall.nes').read_bytes()
offset = symbols['SuperDodgeBall'] - 0x08000000
assert b[offset:offset+len(rom)] == rom
print('PASS: ROM SDRAM arena and complete embedded SuperDodgeBall bytes')
