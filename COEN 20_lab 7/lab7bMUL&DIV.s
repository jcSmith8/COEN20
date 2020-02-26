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

	LDR R2, =20
	UDIV R3, R0, R2 	// R2 <-- dollars/20 (number of $20 bills)
	STR R3, [R1]		// R1[Twenties] <-- R2
	MUL R2, R2, R3		// R2 <-- 20 * (number of twenties)
	SUB R0, R0, R2		// dollars -= (twenties) * 20
	
	LDR R2, =10
	UDIV R3, R0, R2		// R2 <-- dollars/10 (number of $10 bills)
	STR R3, [R1, 4]		// R1[Tens] <-- R2
	MUL R2, R2, R3		// amount of money taken up by $10 bills
	SUB R0, R0, R2		// update dollars
	
	LDR R2, =5
	UDIV R3, R0, R2
	STR R3, [R1, 8]
	MUL R2, R2, R3
	SUB R0, R0, R2
	
	LDR R2, =1
	UDIV R3, R0, R2
	STR R3, [R1, 12]

	
	BX	LR
	
	
//void Coins(uint32_t cents, COINS *coins);
	.global		Coins
	.thumb_func
	
Coins:
	
	LDR R2, =25
	UDIV R3, R0, R2 	// R2 <-- cents/25 (number of 25 cent coins)
	STR R3, [R1]		// R1[quarters] <-- R2
	MUL R2, R2, R3
	SUB R0, R0, R2		// cents -= (quarters) * 25
	
	LDR R2, =10
	UDIV R3, R0, R2		// R2 <-- cents/10 (number of dimes)
	STR R3, [R1, 4]		// R1[dimes] <-- R2
	MUL R2, R2, R3		// amount of money taken up by dimes
	SUB R0, R0, R2		// update cents
	
	LDR R2, =5
	UDIV R3, R0, R2
	STR R3, [R1, 8]
	MUL R2, R2, R3
	SUB R0, R0, R2
	
	LDR R2, =1
	UDIV R3, R0, R2
	STR R3, [R1, 12]

	
	BX	LR

	.end
	