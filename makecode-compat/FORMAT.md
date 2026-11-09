# AIR Arcade bytecode v0

All multi-byte values are little-endian. The VM executes a bounded instruction list once per frame. Programs are limited to 4096 bytes and 256 instructions per frame.

| Opcode | Name | Operands |
|---:|---|---|
| 0 | END | none |
| 1 | SET_CONST | variable:u8, value:i16 |
| 2 | ADD | destination:u8, source:u8 |
| 3 | READ_BUTTON | destination:u8, button:u8 |
| 4 | CREATE_SPRITE | id:u8, x:i16, y:i16, width:u8, height:u8, color:rgb565:u16 |
| 5 | SET_VELOCITY | id:u8, vx:i16, vy:i16 |
| 6 | JUMP | absolute byte offset:u16 |
| 7 | JUMP_IF_ZERO | variable:u8, absolute byte offset:u16 |
| 8 | COLLIDE | sprite_a:u8, sprite_b:u8, absolute byte offset:u16 |
| 9 | SET_BACKGROUND | rgb565:u16 |
| 10 | SET_SCORE | variable:u8 |
| 11 | DRAW | none |

Button IDs: 0 up, 1 down, 2 left, 3 right, 4 A, 5 B, 6 menu.

This is an internal prototype format, not a MakeCode export format. The importer/compiler will generate it from a documented subset of MakeCode APIs. Unknown APIs must produce an explicit unsupported-feature error.
