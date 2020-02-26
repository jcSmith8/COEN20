//Cooper Smith
//January 16, 2020
//puzzle.s

		.syntax		unified
		.cpu		cortex-m4
		.text
		
		
		.global		CopyCell
		.thumb_func
	//void CopyCell(RGB_PXL *dst, RGB_PXL *src);
	CopyCell:
				PUSH {R4, R5}		//initialize row and col to 0
				LDR R4, =0			//R4 = row
	outerLoop:	
				LDR R5, =0			//R5 = col
				CMP R4, 60			//if (row >= 60) goto done
				BHS done
	innerLoop:	
				CMP R5, 60			//if (col >= 60) goto innerDone
				BHS innerDone
				LDR R3, [R1,R5,LSL 2]	//R3 <- src[col]
				STR R3, [R0, R5, LSL 2]	//dst[col] <- R3
				ADD R5, R5, 1			//col++
				B innerLoop
	innerDone:	
				ADD R4, R4, 1			//row++
				ADD R0, R0, 960			//dst+=240 (960 = 240 * 4 bytes)
				ADD R1, R1, 960			//dst+=240	""
				B outerLoop
	done:		
				POP {R4, R5}
				BX	LR
				
		.global		FillCell
		.thumb_func
	//void FillCell(RGB_PXL *dst, uint32_t pixel);
	FillCell:	
				PUSH {R4, R5}
				LDR R4, =0			//R4 = row
	fouterLoop: 
				LDR R5, =0			//R5 = col
				CMP R4,60			//if (row >= 60) goto DONE
				BHS fdone
	finnerLoop: 
				CMP R5,60			//if (col >= 60) goto innerDone
				BHS finnerDone
				STR R1,[R0,R5,LSL 2] 		//dst[col] = pixel
				ADD R5,R5,1 				//col++
				B finnerLoop
	finnerDone: 
				ADD R4,R4,1 				//row++
				ADD R0,R0,960				//dst+=240 (960 = 240 * 4 bytes)
				B fouterLoop
	fdone:      
				POP {R4, R5}
				BX	LR
			
			
			.end
		