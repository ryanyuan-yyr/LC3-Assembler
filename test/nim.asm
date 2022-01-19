             .ORIG x3000

; Usage of registers
; * R1 The current player
; * R3 The address of data
; * R5 The current total number of stones
main          AND R1, R1, #0
              AND R5, R5, #0
              LEA R3, data
              ADD R0, R5, #3
              ; Initialize R5
    accum     BRz rnd_begin
              LDR R2, R3, #0
              ADD R5, R2, R5
              ADD R3, R3, #1
              ADD R0, R0, #-1
              BRnzp accum

              ; Round begin
    rnd_begin JSR cur_cond
              JSR get_in

              ; Refreshes R5 and determins if a winner appears
              NOT R2, R2
              ADD R2, R2, #1
              ADD R5, R5, R2
              BRz win

              ; Refreshes the number of stones
              LDR R4, R3, #0
              ADD R4, R2, R4
              STR R4, R3, #0

              ; Flips player
              ADD R1, R1, #1
              AND R1, R1, #1
              
              LD R0, newLine
              TRAP x21
              BRnzp rnd_begin
    win       JSR winner
              HALT

; Output the current condition
cur_cond      ST R1, Save0
              ST R2, Save1
              AND R1,R1,  #0

              ; R1 stores the index of current row
    rowBegin  ADD R1, R1, #-3
              BRz exit_cc
              ADD R1, R1, #3

              ; print 'ROW_X'
              LEA R0, s_row
              TRAP	x22
              LD R0, A
              ADD R0, R0, R1
              TRAP   x21
              LEA R0, colon
              TRAP   x22
              
              ; print 'o's
              LD R0, o
              LEA R2, data
              ADD R2, R2, R1
              LDR R2, R2, #0
    out_o     BRz nextRow
              TRAP  x21
              ADD R2, R2, #-1
              BRnzp out_o

              ; The current row is finished
    nextRow   ADD R1, R1, #1
              LD R0, newLine
              TRAP	x21
              BRnzp rowBegin

    exit_cc   LD R1, Save0
              LD R2, Save1
              RET

; @breif Get the selected row index and number of stoned to be removed.  
; @para None
; @return 
;      R2  The number of stones to be removed
;      R3  The location where the number of stones in the selected row is stored
get_in        ST R4, Save0
              ST R5, Save1

              ; show prompt
              LEA R0, prompt1
              TRAP    x22
              LD R0, one
              ADD R0, R0, R1
              TRAP    x21
              LEA R0, prompt2
              TRAP    x22
              
              ; Reading chars
              TRAP	  x20
              TRAP    x21
              AND R4, R0, #-1

              TRAP    x20
              TRAP    x21
              AND R2, R0, #-1

              LD R0, newLine
              TRAP    x21
              
              ; Examinates if less than `A`
              LD R3, A
              NOT R3, R3
              ADD R3, R3, #1
              ADD R4, R4, R3
              BRn fail

              ; Examinates if greater than `C`
              ADD R3, R4, #-3
              BRzp fail

              ; Examinates if less than `0`
              LD R3, zero
              NOT R3, R3
              ADD R3, R3, #1
              ADD R2, R3, R2
              BRn fail

              ; Examinates if greater than the number of stones in the chosen row
              LEA R3, data   ; Load the number of stones
              ADD R3, R3, R4
              LDR R5, R3, #0
              NOT R5, R5     ; Comparison
              ADD R5, R5, #1
              ADD R5, R5, R2
              BRp fail

              BRnzp succ

    fail      LEA R0, invalid_arg
              TRAP	x22
              BRnzp get_in
    succ      LD R4, Save0
              LD R5, Save1
              RET

winner        LD R0, newLine
              TRAP x21
              LEA R0, prompt1
              TRAP  x22

              ; Flips player
              ADD R1, R1, #1
              AND R1, R1, #1

              LD R2, one
              ADD R0, R1, R2
              TRAP  x21
              LEA R0, win_msg
              TRAP  x22
              RET

data         .FILL      #3
             .FILL      #5
             .FILL      #8
o            .FILL      x6F
A            .FILL      x41
one          .FILL      x31
zero         .FILL      x30
newLine      .FILL      x0A
s_row        .STRINGZ	"ROW "
colon        .STRINGZ   ": "
prompt1      .STRINGZ	"Player "
prompt2      .STRINGZ	", choose a row and number of rocks: "
invalid_arg  .STRINGZ	"Invalid move. Try again.\n"
win_msg      .STRINGZ	" Wins. \n"
Save0        .BLKW      #1
Save1        .BLKW      #1

             .END