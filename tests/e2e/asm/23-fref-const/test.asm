.extern ext_data
.section text
jmp my_const
.word ext_data
.equ my_const, 0x5
.end
