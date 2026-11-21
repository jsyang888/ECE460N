.ORIG x1200
    ADD R6, R6, #-2
    STW R0, R6, #0
    ADD R6, R6, #-2
    STW R1, R6, #0
    ADD R6, R6, #-2
    STW R2, R6, #0
    ; supervisor stack push, AAPCS

    LEA R0, PT_Location
    LDW R0, R0, #0
    LEA R2, Count
    LDW R2, R2, #0
    ; initialize program variables

LOOP LDW R1, R0, #0
    AND R1, R1, #-2 ; this clears the reference bit
    STW R1, R0, #0
    ADD R0, R0, #2
    ADD R2, R2, #-1
    BRp LOOP ; loop clears all reference bits in 128 PTE

    LDW R2, R6, #0 ; restore register 
    ADD R6, R6, #2
    LDW R1, R6, #0
    ADD R6, R6, #2
    LDW R0, R6, #0
    ADD R6, R6, #2

    RTI ; x121a
PT_Location .FILL x1000
Count .FILL x0080 ; 128 total entries
.END