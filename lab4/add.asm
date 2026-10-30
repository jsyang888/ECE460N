.ORIG x3000
    .FILL xA000
    LEA R0, initialize
    LDW R0, R0, #0
    LDW R1, R0, #0
    ADD R1, R1, #1
    STW R1, R0, #0 ; add one to x4000, initialized

    ; now do the summing
    LEA R0, start
    AND R3, R3, #0 ; counter
    ADD R3, R3, #10
    ADD R3, R3, #10
    AND R4, R4, #0 ; store in R4
    LDW R0, R0, #0;
Loop    
    LDB R1, R0, #0
    ADD R4, R4, R1 ; add to R4
    ADD R0, R0, #1 ; increment ptr
    ADD R3, R3, #-1 ; decrement R3
    BRp Loop

    ; at this point, done with adding

    LEA R0, sum
    LDW R0, R0, #0
    STW R4, R0, #0 ; store, done

HALT



start .FILL xC000
sum .FILL xC014
initialize .FILL x0001