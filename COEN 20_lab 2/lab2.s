// Cooper Smith
// 1/22/2019
// COEN 20 Lab 2A: Functions and Parameters

	.syntax		unified
	.cpu		cortex-m4
	.text
	
	// int32_t Add(int32_t a, int32_t b;
		.global		Add
		.thumb_func
		
	Add:
		ADD R0, R0, R1	//R0 <-- (a+b)
		BX	LR
	
	
	// int32_t Less1(int32_t a);
		.global		Less1
		.thumb_func
		
	Less1:
		SUB R0, R0, 1	//R0 <-- (a - 1))
		BX	LR
		
	// int32_t Square2x(int32_t x);
		.global		Square2x
		.thumb_func
		
	Square2x:
		ADD R0, R0, R0	//R0 <-- (x+x)
		B Square		//Call square using R0 (2x)
		
	
	// int32_t Last(int32_t x);
		.global		Last
		.thumb_func
		
	Last:
		PUSH {R4, LR}	//push R4 onto stack so we can preserve original value
		MOV R4, R0		//x value goes into R4
		BL SquareRoot	//call squareroot and return to R0
		ADD R0, R0, R4	//add (x + squareroot(x)) and put into Reg0
		POP {R4, PC}	//pop original R4 value off stack
	
	
		.end
	