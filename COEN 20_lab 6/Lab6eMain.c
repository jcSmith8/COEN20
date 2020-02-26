/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

// Adapted from code posted at https://codereview.stackexchange.com/questions/136406/tetris-in-c-in-200-lines

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "library.h"
#include "graphics.h"

typedef int BOOL ;
#define TRUE	1
#define FALSE	0

#define	ENTRIES(a)				(sizeof(a)/sizeof(a[0]))

#define	CPU_SPEED_MHZ			168		// Also speed of DWT_CYCCNT used by GetClockCycleCount()
#define	CPU_SPEED_HZ			(CPU_SPEED_MHZ * 1E6)

#define	ROW_OFFSET				49		// y-pixel start position of usable display area
#define	COL_OFFSET				0		// x-pixel start position of usable display area

// Functions to be implemented in ARM assembly
extern BOOL GetBit(uint16_t *bits, uint32_t row, uint32_t col) ;
extern void PutBit(BOOL value, uint16_t *bits, uint32_t row, uint32_t col) ;

// Game parameters
#define ROWS					17		// Number of vertical cells in the playing field
#define COLS					16		// Number of horizontal cells in the playing field

#define	MSEC_PER_TILT			150		// How often the game checks for left or right tilt
#define	MSEC_PER_DOWN			500		// How often the game moves a shape down one row

#define	CYCLES_PER_TILT			(MSEC_PER_TILT*CPU_SPEED_MHZ*1000)
#define	CYCLES_PER_DOWN			(MSEC_PER_DOWN*CPU_SPEED_MHZ*1000)

#define	CELL_WIDTH				15		// horizontal pixels per cell
#define	CELL_HEIGHT				15		// vertical pixels per cell

uint32_t	Table[ROWS][COLS] ;			// keeps track of cells in the playing field
BOOL		GameOn = TRUE ;				// Chenaged to FALSE when game is finished

typedef struct
	{
	uint16_t	array ;		// An array of four nibbles - one per row of the Shape
    uint32_t	size ;		// shapes are square, so this is the number of rows and cols
	uint32_t	color ;		// Color of the shape
	uint32_t	row ;		// current row position of upper-left-hand corner of shape
	uint32_t	col ;		// current col position of upper-left-hand corner of shape
	} SHAPE ;

SHAPE current, temp ;

SHAPE ShapesArray[] =
	{
    {0x0063, 3},	// S shape
    {0x0036, 3},	// Z shape
    {0x0072, 3},	// T shape
    {0x0017, 3},	// L shape
    {0x0074, 3},	// J shape
    {0x0033, 2},	// O shape
    {0x00F0, 4}		// I shape
	} ;

// Gyro operating parameters
#define	ZERO_RATE_SAMPLES		300		// Number of gyro samples in running average
#define	SENSITIVITY				0.07	// Converts integer samples to degrees/sec

// Library function prototypes for Gyro sensor (not included in library.h)
extern void GYRO_IO_Init(void) ;
extern void GYRO_IO_Write(uint8_t* data, uint8_t port, uint16_t bytes) ;
extern void GYRO_IO_Read(uint8_t* data, uint8_t port, uint16_t bytes) ;

const int GYRO_CTRL_REG1 = 0x20 ;
#define	GYRO_DR1_FLAG			(1 << 7)
#define	GYRO_DR0_FLAG			(1 << 6)
#define	GYRO_BW1_FLAG			(1 << 5)
#define	GYRO_BW0_FLAG			(1 << 4)
#define	GYRO_PD_FLAG			(1 << 3)
#define	GYRO_ZEN_FLAG			(1 << 2)
#define	GYRO_YEN_FLAG			(1 << 1)
#define	GYRO_XEN_FLAG			(1 << 0)

const int GYRO_CTRL_REG4 = 0x23 ;
#define	GYRO_BDU_FLAG			(1 << 7)
#define	GYRO_FS1_FLAG			(1 << 5)
#define	GYRO_FS0_FLAG			(1 << 4)

const int GYRO_CTRL_REG5 = 0x24 ;
#define	GYRO_OSEL1_FLAG			(1 << 1)
#define	GYRO_HPEN_FLAG			(1 << 4)

const int GYRO_STAT_REG	= 0x27 ;
#define	GYRO_ZYXDA_FLAG			(1 << 3)

const int GYRO_DATA_REG	= 0x28 ;

#define	SHAPE_DOWN				0
#define	SHAPE_ROTATE			1
#define	SHAPE_RIGHT				2
#define	SHAPE_LEFT				3
#define	SHAPE_DROP				4

static BOOL		Conflict(SHAPE *shape) ;
static void		GetNewShape(void) ;
static void		GetVelocity(float degrees_per_sec[]) ;
static void		IncreaseScore(unsigned points) ;
static void		InitializeGyroscope(void) ;
static void		LEDs(int grn_on, int red_on) ;
static BOOL		MoveThisShape(int action) ;
static void		PaintCell(int row, int col, int color) ;
static void		PaintShape(int color) ;
static void		RotateShape(SHAPE *shape) ;
static int		SanityChecksOK(void) ;
static void		CollapseOneRow(void) ;
static void		WriteToTable(void) ;

int main()
	{
	float degr_roll, degr_ptch, prev_ptch_dps, prev_roll_dps, deltaT, dps[3] ;
	uint32_t curr_time,  down_timeout, tilt_timeout, curr_cycles, prev_cycles ;
	int row, col ;
	uint8_t sts ;

	InitializeHardware(HEADER, "Lab 6e: Tetris & Gyros") ;
	if (!SanityChecksOK()) return 255 ;
	InitializeGyroscope() ;

	for (row = 0; row < ROWS; row++)
		{
		for (col = 0; col < COLS; col++)
			{
			Table[row][col] = COLOR_WHITE ;
			}
		}

 	curr_cycles  = curr_time = GetClockCycleCount() ;
	down_timeout = curr_time + CYCLES_PER_DOWN ;
	tilt_timeout = curr_time + CYCLES_PER_TILT ;

	prev_ptch_dps = prev_roll_dps = 0.0 ;
	degr_roll = degr_ptch = 0.0 ;
	GetNewShape() ;
	PaintShape(current.color) ;
    while (GameOn)
		{
		if (PushButtonPressed())
			{
			WaitForPushButton() ;
			MoveThisShape(SHAPE_ROTATE) ;
			degr_ptch = degr_roll = 0.0 ;
			}


		// Check to see if new gyro data is available
		GYRO_IO_Read(&sts, GYRO_STAT_REG, sizeof(sts)) ;
		if ((sts & GYRO_ZYXDA_FLAG) != 0)
			{
			prev_cycles = curr_cycles ;
			curr_cycles = GetClockCycleCount() ;
			deltaT = (curr_cycles - prev_cycles) / CPU_SPEED_HZ ;
			GetVelocity(dps) ;

			// Perform trapezoidal integration to get position
			degr_ptch += deltaT * (dps[0] + prev_ptch_dps) / 2 ;
			degr_roll += deltaT * (dps[1] + prev_roll_dps) / 2 ;
			prev_ptch_dps = dps[0] ;
			prev_roll_dps = dps[1] ;
			}

		curr_time = GetClockCycleCount() ;

		if ((int) (tilt_timeout - curr_time) < 0)
			{
			if (degr_roll < -10.0)		MoveThisShape(SHAPE_LEFT) ;
			else if (degr_roll > +10.0)	MoveThisShape(SHAPE_RIGHT) ;
			if (degr_ptch > 20.0)		MoveThisShape(SHAPE_DROP) ;
			degr_ptch = 0.0 ;
			tilt_timeout += CYCLES_PER_TILT ;
			}

		if ((int) (down_timeout - curr_time) < 0)
			{
    	    if (MoveThisShape(SHAPE_DOWN))
				{
				IncreaseScore(1) ;
				degr_ptch = degr_roll = 0.0 ;
				}
			down_timeout += CYCLES_PER_DOWN ;
			}
		}

    return 0 ;
	}

static void IncreaseScore(unsigned points)
	{
	static unsigned score = 0 ;
	char footer[100] ;

	score += points ;
	sprintf(footer, "Lab 6e: Tetris & Gyros (%d pts)", score) ;
	DisplayFooter(footer) ;
	}

static BOOL Conflict(SHAPE *shape)
	{
	int row = shape->row ;
    for (int r = 0; r < shape->size; r++, row++)
		{
		int col = shape->col ;
        for (int c = 0; c < shape->size; c++, col++)
			{
			if (GetBit(&shape->array, r, c) == 0) continue ;

			// Is this cell of the table already occupied?
			if (Table[row][col] != COLOR_WHITE) return TRUE ;

            if (col < 0 || col >= COLS || row >= ROWS) return TRUE ;
			}
		}

	return FALSE ;
	}

static void GetNewShape(void)
	{
	static int colors[] = {COLOR_RED, COLOR_BLUE, COLOR_ORANGE, COLOR_YELLOW, COLOR_MAGENTA, COLOR_CYAN, COLOR_GREEN} ;
	static int prev_shape = 0 ;
	static int prev_color = 0 ;
	int shape, color ;

	do shape = GetRandomNumber() % ENTRIES(ShapesArray) ;
	while (shape == prev_shape) ;
	prev_shape = shape ;
    current = ShapesArray[shape] ;

	do color = GetRandomNumber() % ENTRIES(colors) ;
	while (color == prev_color) ;
	prev_color = color ;
	current.color = colors[color] ;

    current.col = GetRandomNumber() % (COLS - current.size + 1) ;
   	current.row = 0 ;

    if (Conflict(&current)) GameOn = FALSE ;
	}

static void RotateShape(SHAPE *shape) //rotates clockwise
	{
    temp = *shape ;
    for (int r = 0; r < shape->size; r++)
		{
		int col = shape->size - 1 ;
        for (int c = 0; c < shape->size ; c++, col--)
			{
			BOOL value = GetBit(&temp.array, col, r) ;
			PutBit(value, &shape->array, r, c) ;
			}
		}
	}

static void WriteToTable(void)
	{
	int row = current.row ;
    for (int r = 0; r < current.size; r++, row++)
		{
		int col = current.col ;
        for (int c = 0; c < current.size; c++, col++)
			{
			if (GetBit(&current.array, r, c) != 0)
				{
				Table[row][col] = current.color ;
				}
			}
		}
	}

static void CollapseOneRow(void)
	{
    for (int row = 0; row < ROWS; row++)
		{
        int filled = 0 ;

        for (int col = 0; col < COLS; col++)
			{
			if (Table[row][col] != COLOR_WHITE) filled++ ;
			}

		// Do nothing if any col of this row is empty
        if (filled < COLS) continue ;

		// Remove full row and shift above rows down
		for (int r = row; r > 0; r--)
			{
			for (int col = 0; col < COLS; col++)
				{
				Table[r][col] = Table[r-1][col] ;
				PaintCell(r, col, Table[r-1][col]) ;
				}
			}

		// Clear the top row
		for (int col = 0; col < COLS; col++)
			{
			Table[0][col] = COLOR_WHITE ;
			PaintCell(0, col, COLOR_WHITE) ;
			}

		IncreaseScore(100) ;
		}
	}

static void PaintCell(int row, int col, int color)
	{
	unsigned xpos = COL_OFFSET + CELL_WIDTH*col ;
	unsigned ypos = ROW_OFFSET + CELL_HEIGHT*row ;

	SetColor(color) ;
	FillRect(xpos, ypos, CELL_WIDTH, CELL_HEIGHT) ;
	if (color != COLOR_WHITE)
		{
		SetColor(COLOR_BLACK) ;
		DrawRect(xpos, ypos, CELL_WIDTH-1, CELL_HEIGHT-1) ;
		}
	}

static void PaintShape(int color)
	{
	int row = current.row ;
    for (int r = 0; r < current.size; r++, row++)
		{
		int col = current.col ;
        for (int c = 0; c < current.size; c++, col++)
			{
			if (GetBit(&current.array, r, c) != 0) PaintCell(row, col, color) ;
			}
		}
	}

static BOOL MoveThisShape(int action)
	{
    temp = current ;
	BOOL done = FALSE ;

	PaintShape(COLOR_WHITE) ;
    switch (action)
		{
		case SHAPE_DROP:
			do temp.row++ ; 
			while (!Conflict(&temp)) ;
			current.row = temp.row - 1 ;
			break ;

        case SHAPE_DOWN:
            temp.row++ ;
            if (!Conflict(&temp))
				{
				current.row++ ;
				break ;
				}

			PaintShape(current.color) ;
			WriteToTable() ;
			CollapseOneRow() ;
			GetNewShape() ;
			done = TRUE ;
            break ;

        case SHAPE_RIGHT:
            temp.col++ ;
            if (!Conflict(&temp)) current.col++ ;
            break ;

        case SHAPE_LEFT:
            temp.col-- ;
            if (!Conflict(&temp)) current.col-- ;
            break ;

        case SHAPE_ROTATE:
            RotateShape(&temp) ;
            if (!Conflict(&temp)) RotateShape(&current) ;
            break ;
		}

    if (GameOn) PaintShape(current.color) ;
	return done ;
	}

static void InitializeGyroscope(void)
	{
	uint8_t cmd ;

	GYRO_IO_Init() ;

	// Enable Block Data Update and full scale = 2000 dps
	cmd = GYRO_BDU_FLAG | GYRO_FS1_FLAG ;
	GYRO_IO_Write(&cmd, GYRO_CTRL_REG4, sizeof(cmd)) ;

	// Enable X, Y and Z channels
	cmd = GYRO_PD_FLAG | GYRO_XEN_FLAG | GYRO_YEN_FLAG | GYRO_ZEN_FLAG ;
	GYRO_IO_Write(&cmd, GYRO_CTRL_REG1, sizeof(cmd)) ;

	// Use output from high-pass filter
	cmd = GYRO_OSEL1_FLAG ;
	GYRO_IO_Write(&cmd, GYRO_CTRL_REG5, sizeof(cmd)) ;
	}

static void GetVelocity(float degrees_per_sec[])
	{
	static float min[3] = {INT16_MAX, INT16_MAX, INT16_MAX} ;
	static float max[3] = {INT16_MIN, INT16_MIN, INT16_MIN} ;
	static float total[3] = {0} ;
	static int32_t count = 0 ;
	int16_t samples[3] ;
	float value ;
	int channel ;

	GYRO_IO_Read((uint8_t *) samples, GYRO_DATA_REG, sizeof(samples)) ;

	for (channel = 0; channel < 3; channel++)
		{
		float sample = (float) samples[channel] ;
		if (count < ZERO_RATE_SAMPLES)
			{
			total[channel] += sample ;
			if (sample > max[channel]) max[channel] = sample + 1 ;
			if (sample < min[channel]) min[channel] = sample - 1 ;
			count++ ;
			}

		// subtract zero-rate-level and compute angular velocity
		if (min[channel] <= sample && sample <= max[channel]) value = 0 ;
		else value = sample - total[channel] / count ;
		degrees_per_sec[channel] = SENSITIVITY * value ;
		}
	}

static int SanityChecksOK(void)
	{
	int row, col, shift, bugs, bit ;
	static int bits ;

	bugs = 0 ;
	printf("\n\n\n") ;

	bits = 0 ;
	row = GetRandomNumber() % 4 ;
	col = GetRandomNumber() % 4 ;
	PutBit(1, (uint16_t *) &bits, row, col) ;
	shift = 4*row + col ;
	if (bits != (1 << shift))
		{
		printf("bits = 0x0000;\nPutBit(1, &bits, row=%d, col=%d);\nbits --> %04X\n\n", row, col, bits & 0xFFFF) ;
		bugs++ ;
		}

	bits = 0xFFFFFFFF ;
	row = GetRandomNumber() % 4 ;
	col = GetRandomNumber() % 4 ;
	PutBit(0, (uint16_t *) &bits, row, col) ;
	shift = 4*row + col ;
	if (bits != ~(1 << shift))
		{
		printf("bits = 0xFFFF;\nPutBit(0, &bits, row=%d, col=%d);\nbits --> %04X\n\n", row, col, bits & 0xFFFF) ;
		bugs++ ;
		}

	row = GetRandomNumber() % 4 ;
	col = GetRandomNumber() % 4 ;
	shift = 4*row + col ;
	bits = 1 << shift ;
	bit = GetBit((uint16_t *) &bits, row, col) ;
	if (bit != 1)
		{
		printf("bits = %04X;\nGetBit(&bits, row=%d, col=%d) --> %d\n\n", bits & 0xFFFF, row, col, bit) ;
		bugs++ ;
		}

	row = GetRandomNumber() % 4 ;
	col = GetRandomNumber() % 4 ;
	shift = 4*row + col ;
	bits = ~(1 << shift) ;
	bit = GetBit((uint16_t *) &bits, row, col) ;
	if (bit != 0)
		{
		printf("bits = %04X;\nGetBit(&bits, row=%d, col=%d) --> %d\n\n", bits & 0xFFFF, row, col, bit) ;
		bugs++ ;
		}

	if (bugs != 0)
		{
		SetForeground(COLOR_WHITE) ;
		SetBackground(COLOR_RED) ;
		DisplayStringAt(0, ROW_OFFSET, "Sanity Check Errors:" ) ;
		}

	LEDs(!bugs, bugs) ;
	return bugs == 0 ;
	}

static void LEDs(int grn_on, int red_on)
	{
	static uint32_t * const pGPIOG_MODER	= (uint32_t *) 0x40021800 ;
	static uint32_t * const pGPIOG_ODR		= (uint32_t *) 0x40021814 ;
	
	*pGPIOG_MODER |= (1 << 28) | (1 << 26) ;	// output mode
	*pGPIOG_ODR &= ~(3 << 13) ;			// both off
	*pGPIOG_ODR |= (grn_on ? 1 : 0) << 13 ;
	*pGPIOG_ODR |= (red_on ? 1 : 0) << 14 ;
	}


