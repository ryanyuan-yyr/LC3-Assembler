
LOOP     JSR MINUS        
         BRz FINISH       
         BRp POSITIVE     

        .ORIG x3000       ; Wrongly placed .ORIG
                          
         BRznp	NONEXIST  ; the label does not exist
         ; R2 = -R0
         NOT R0, R8       ; R8 is not a register
         ADD R0, R0, xFFF ; The immediate is out of range
LOOP     AND R2, R0, -1  ; Immediate in wrong form; Multiple address for one label

         BRnzp LOOP

POSITIVE AND R1, R0      ; Missing arguments
         ADD R1, R0, R1, #0 ; Too many args
         
         .FILI #0        ; typo

         BRnzp LOOP

FINISH   AND R0, R1, #-1
         HALT
TMPSTORE      .FILL	#0

         .BLKW	xFFF     ; Big bulk. To invalidate labels.  

MINUS    ST R2, TMPSTORE ; Label TMPSTORE is beyond the range
         NOT R2, R2
         ADD R2, R2, #1
         ADD R0, R1, R2
         LD R2, TMPSTORE
         AND R0, R0, #-1
         RET
         ;.END          ; Missing .END