// Cooper Smith
// 1/29/2020
// COEN 20 Lab 3: Copying Data Quickly

	.syntax		unified
	.cpu		cortex-m4
	.text
	
	.global UseLDRB
	.thumb_func
	
	// void UseLDRB (void *dst, void *src);
	UseLDRB:
		
		.rept 512			//repeat 512 times for 512 byes
		LDRB R2, [R1], 1	//increment by 1 byte each time
		STRB R2, [R0], 1
		.endr
		BX	LR
		
		
		.global		UseLDRH
		.thumb_func
		
	//void UseLDRH (void *dsst, void *src);
	UseLDRH:
		
		.rept 256			//repeat 256 times
		LDRH R2, [R1], 2	//increment by 2 bytes each time
		STRH R2, [R0], 2
		.endr
		BX	LR
	
		.global		UseLDR
		.thumb_func
		
	//void UseLDR(void *dst, void *src)
	UseLDR:
	
		.rept 128			//repeat 128 times
		LDR R2, [R1], 4 	//increment by 4 bytes each time
		STR R2, [R0], 4
		.endr
		BX	LR
		
		.global		UseLDRD
		.thumb_func
		
	//void UseLDRD(void *dst, void *src)
	UseLDRD:
	
		.rept 64				//repeat 64 times
		LDRD R2, R3, [R1], 8	//increment by 8 bytes each iteration
		STRD R2, R3, [R0], 8	//stores R2 and R3 into destination pointer
		.endr
		BX	LR
		
		.global		UseLDM
		.thumb_func
		
	//void UseLDM(void *dst, void *src)
	UseLDM:
		
		PUSH {R4-R9}		//push extra registers because we are dealing with large amt of bytes
		.rept 16
		LDMIA R1!, {R2-R9}	//load registers 2-9 and iterate with !
		STMIA R0!, {R2-R9}	//store values from reg 2-9 into destination pointer
		.endr
		POP {R4-R9}
		BX	LR

	.end