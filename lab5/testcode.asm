.ORIG x3000
    LEA R0, Target
    LDW R0, R0, #0
    LDW R1, R0, #0
Target .FILL x4000 ; change target to test different exceptions
.END