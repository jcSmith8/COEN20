//Cooper Smith
//2/26/2020
//Lab 7b Making Change

.syntax unified
.cpu cortex-m4
.text

//void Bills(uint32_t dollars, BILLS *paper);
	.global		Bills
	.thumb_func
	
Bills:
	
	LDR R3, =214748365 		//R3 <- 2^32 / 20
	SMMUL R2, R0, R3
	STR R2, [R1] 			//paper[twenties] <- quotient
	LSL R3, R2, 4			//R3 <- 16 * count
	ADD R3, R3, R2, LSL 2	//R3 <- 16count + 4count = 20count
	SUB R0, R0, R3 			//R0 <- remainder
	
	B TensFivesOnes

//void Coins(uint32_t cents, COINS *coins);
	.global		Coins
	.thumb_func
	
Coins:
	
	LDR R3, =171798691 		//R3 <- 2^32 / 25
	SMMUL R2, R0, R3
	STR R2, [R1] 			//coins[quarters] <- quotient
	LSL R3, R2, 4			//R3 <- 16 * count
	ADD R12, R3, R2			//R3 <- 16count + count = 17count
	ADD R3, R12, R2, LSL 3	//R3 <- 17count + 8count = 25count
	SUB R0, R0, R3 			//R0 <- remainder

TensFivesOnes:
	LDR R3, =429496629		//R3 <- 2^32 / 10
	SMMUL R2, R0, R3
	STR R2, [R1, 4]
	LSL R3, R2, 3			//R3 <- 8 * count
	ADD R3, R3, R2, LSL 1	//R3 <- 8count + 2count = 10count
	SUB R0, R0, R3
	
	LDR R3, =858993459		//R3 <- 2^32 / 5
	SMMUL R2, R0, R3
	STR R2, [R1, 8]
	LSL R3, R2, 2			//R3 <- 4 * count
	ADD R3, R3, R2			//R3 <- 4count + count = 5count
	SUB R0, R0, R3
	
	STR R0, [R1, 12]		// R1[dollars] <- (remainder dollars) 
	
	BX	LR

	.end
	