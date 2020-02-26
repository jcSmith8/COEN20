// Cooper Smith
// 1/29/2020
// COEN 20 Lab 3: Copying Data Quickly

	.syntax		unified
	.cpu		cortex-m4
	.text
	
	//int32_t Discriminant(int32_t a, int32_t b, int32_t c);
		.global		Discriminant
		.thumb_func
	Discriminant:
	
		MULS R1,R1,R1 		// R1 <- b^2
		LSL R3, R0, 2 		// R3 <- 4a
		MLS R0, R2, R3, R1	// R0 <- b^2 - (4a * c) 
		BX	LR
		
	
	//int32_t Root1(int32_t a, int32_t b, int32_t c);
		.global		Root1
		.thumb_func
	Root1:
	
		PUSH {R4, R5, LR}
		NEG R5, R1			//stores -b into R5
		LSLS R4, R0, 1 		//stores 2a into R4
		BL Discriminant
		BL SquareRoot
		ADDS R0, R0, R5		// R0 <- SquareRoot(Discriminant) - b
		SDIV R0, R0, R4		// R0 <- ( SquareRoot(Disc) - b ) / 2a
		POP {R4, R5, PC}
		
	
	//int32_t Root2(int32_t a, int32_t b, int32_t c);
		.global		Root2
		.thumb_func
	Root2:
	
		PUSH {R4, R5, LR}
		NEG R5, R1			//stores -b into R5
		LSLS R4, R0, 1		//stores 2a into R4
		BL Discriminant
		BL SquareRoot
		SUBS R0, R5, R0		// R0 <- (-b) - (SquareRoot(Discriminant))
		SDIV R0, R0, R4		// R0 <- (-b-SquareRoot(Discriminant)) / 2a
		POP {R4, R5, PC}
	
	
	// int32_t Quadratic(int32_t x, int32_t a, int32_t b, int32_t c);
		.global		Quadratic
		.thumb_func
	Quadratic:
		
		
		MLA R2, R0, R2, R3		// R2 <- bx + c
		MUL R0, R0, R0			// R0 <- x^2
		MUL R0, R0, R1			// R0 <- x^2 * a
		ADDS R0, R0, R2
		BX	LR
		
	
	
		.end