//Cooper Smith
//2/19/2020
//Lab 6 Tetris

.syntax unified
.cpu cortex-m4
.text

.set BITBANDING,1 //comment out if not using bit banding

.ifdef BITBANDING

.global GetBit
.thumb_func
.align
GetBit: //R0 = uint16_t *bits, R1 = uint32_t row, R2 = uint32_t col

		SUB R3,R0,0x20000000
		LSL R3,R3,5
		LDR R12,=4
		MUL R1,R1,R12			//R1 <- R1 * R12
		ADD R1,R1,R2 			//R1 <- R1 (row) + R2 (col)
		ADD R3,R3,R1,LSL 2		//R3 <- R3 + R1*4
		ADD R3,R3,0x22000000	
		LDRH R0,[R3]
		BX LR

.global PutBit
.thumb_func
.align
PutBit: //R0 = BOOL value, R1 = *bits, R2 = row, R3 = col

		SUB R12,R1,0x20000000
		LSL R12,R12,5					//Row = Row * 4
		ADD R2, R3, R2, LSL 2			//R2 <- R2 (row) + R3 (col)
		ADD R12,R12,R2,LSL 2			//R12 <- R12 + (R2 * 4)
		ADD R12,R12,0x22000000			//Add column value into R12
		STRH R0,[R12]
		BX	LR
		
.else //NO bit banding

.global GetBit
.thumb_func
.align
GetBit: //R0 = uint16_t *bits, R1 = uint32_t row, R2 = uint32_t col

		LDRH R0,[R0]		//load 16 bit value into R0
		LDR R3,=4			//load 4 because rows are bytes
		MUL R1,R1,R3		//set value for rows
		ADD R1,R1,R2 		//index = (row*4)+col
		LDR R12,=1
		LSL R12,R12,R1		//single bit value representing row and col
		AND R0,R0,R12
		LSR R0,R0,R1
		BX LR

.global PutBit
.thumb_func
.align
PutBit: //R0 = BOOL value, R1 = *bits, R2 = row, R3 = col

		PUSH {R4,R5,LR}
		LDRH R5,[R1]
		ADD R2, R3, R2, LSL 2
		LDR R4,=1
		LSL R4,R4,R2		//shift the bit in R4 by R2 rows
		CMP R0,0
		ITE EQ
		BICEQ R5,R5,R4		//Bit clear R5 <- R5 & R4
		ORRNE R5, R5, R4 	//Bitwise OR R5 <- R5 | R4
		STRH R5,[R1]
		POP {R4,R5,PC}

.endif
.end