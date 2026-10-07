.syntax unified
.section .rodata.GameFiles,"a",%progbits
.balign 4
.global SuperMario
.type SuperMario,%object
SuperMario:
.incbin "nes/games/SuperMarioBros.nes"
SuperMario_end:
.size SuperMario, .-SuperMario
.balign 4
.global SuperMarioSize
SuperMarioSize:
.word SuperMario_end-SuperMario
.balign 4
.global yingzichuanshuo
.type yingzichuanshuo,%object
yingzichuanshuo:
.incbin "nes/games/yingzichuanshuo.nes"
yingzichuanshuo_end:
.size yingzichuanshuo, .-yingzichuanshuo
.balign 4
.global yingzichuanshuoSize
yingzichuanshuoSize:
.word yingzichuanshuo_end-yingzichuanshuo
 .balign 4
.global SuperDodgeBall
.type SuperDodgeBall,%object
SuperDodgeBall:
.incbin "nes/games/SuperDodgeBall.nes"
SuperDodgeBall_end:
.size SuperDodgeBall, .-SuperDodgeBall
.global SuperMario_end, yingzichuanshuo_end, SuperDodgeBall_end
.section .note.GNU-stack,"",%progbits
