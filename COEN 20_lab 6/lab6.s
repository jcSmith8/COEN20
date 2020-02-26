//Cooper Smith
//Feb 19 2020
//tetris.s

		.syntax		unified
		.cpu		cortex-m4
		.text
		
	// BOOL GetBit(uint16_t *bits, uint32_t row, uint32_t col);
		.global		GetBit
		.thumb_func
		.set		BITBANDING, 1
		.ifdef		BITBANDING
		
	GetBit:	//WITH bit banding
		
		SUB R3, R0, 0x20000000 		//R3 <- R0 - 0x22000000
		LSL R3, R3, 5 				//R3 <- R3 * 2^5
		LDR R12, =4 				//R12 <- 4
		MUL R1, R1, R12 			//R1 <- R1 * R12
		ADD R1, R1, R2 				//R1 <- R1 (row) + R2 (col)
		ADD R3, R3, R1, LSL 2 		//R3 <- R3 + R1*4
		ADD R3, R3, 0x22000000
		LDRH R0, [R3] 				//R0 <- [R3]
		BX LR
		
		.else
	
	GetBit:	//no bit banding

		LDRH R0, [R0] 			//R0 <- [R0]
		LDR R3, =4 				//R3 <- 4
		MUL R1, R1, R3 			//R1 (row) <- R1 (row) * R3 (4)
		ADD R1, R1, R2 			//R1 <- R1 (row*4) + R2 (col)
		LDR R12, =1 			//R12 <- 1
		LSL R12, R12, R1 		//R12 <- R12 * R1
		AND R0, R0, R12 		//R0 <- R0 AND R12
		LSR R0, R0, R1 			//R0 <- R0 / R1
		BX LR
		.endif
	
	//void PutBit(BOOL value, uint16_t *bits, uint32_t row, uint32_t col);
		.global		PutBit
		.thumb_func
		.ifdef		BITBANDING
	
	PutBit:	//WITH bit banding

		PUSH {R4}
		SUB R12, R1, 0x20000000 	//R12 <- R1 - 0x20000000
		LSL R12, R12, 5 			//R12 <- R12 * 2^R5
		LDR R4, =4 					//R4 <- 4
		MUL R2, R2, R4 				//R2 <- R2 * R4
		ADD R2, R2, R3 				//R2 <- R2 + R3
		ADD R12, R12, R2, LSL 2 	//R12 <- R12 + R2 * 4
		ADD R12, R12, 0x22000000
		STRH R0, [R4]
		POP {R4}
		BX LR
		
		.else
		
	PutBit:	// no bit banding

		PUSH {R4,R5}
		LDRH R5, [R1]		//R5 <- [R1]
		LDR R12, =4 		//R12 <- 4
		MUL R2, R2, R12 	//R2 <- R2 * R12
		ADD R2, R2, R3 		//R2 <- R2 * R3
		LDR R4, =1 			//R4 <- 1
		LSL R4, R4, R2 		//R4 <- R4 * 2^R2
		CMP R0, 0 			//If R0 is 0
		ITE EQ
		BICEQ R5, R5, R4 	//Bit clear R5 <- R5 & R4
		ORRNE R5, R5, R4 	//Bitwise OR R5 <- R5 | R4
		STRH R5, [R1]
		POP {R4,R5,PC}
		
		.endif
	
		.end