.ORIG x3000 
    LEA R0, Access
    LDW R0, R0, #0
    LDB R1, R0, #0 ; x3100
    LDB R2, R0, #1 ; x3101
    AND R3, R3, #0 ; product
    AND R4, R4, #0 ; R4 will track overflow status

    ADD R1, R1, #0 ; reset CC bits
    BRz LoopOver

Loop ADD R3, R3, R2 ; R3 has product
     ADD R1, R1, #-1
     BRp Loop


LoopOver LEA R5, Negative256
         LDW R5, R5, #0
         ADD R5, R3, R5
         BRn Result ; this means product is under 256, not overflowed
         ADD R4, R4, #1 ; set the overflow flag


Result  STB R4, R0, #3 ; 1 -> MEM[x3103] 
        STB R3, R0, #2 ; Result -> MEM[x3102]

HALT

Access      .FILL x3100       ; x3026
Negative256 .FILL xFF00       ; x3028


.END