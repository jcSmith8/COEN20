/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include "library.h"
#include "graphics.h"

extern int32_t Discriminant(int32_t a, int32_t b, int32_t c) ;
extern int32_t Root1(int32_t a, int32_t b, int32_t c) ;
extern int32_t Root2(int32_t a, int32_t b, int32_t c) ;
extern int32_t Quadratic(int32_t x, int32_t a, int32_t b, int32_t c) ;

#define	ENTRIES(a)	(sizeof(a)/sizeof(a[0]))

#define	GFXCOL1		0
#define	GFXCOLN		(XPIXELS-1)
#define	GFXCOLS		(GFXCOLN - GFXCOL1 + 1)

#define	GFXROW1		45
#define	GFXROWN		305
#define	GFXROWS		(GFXROWN - GFXROW1 + 1)

#define	AXISROW		(GFXROW1 + GFXROWS/2)
#define	AXISCOL		(GFXCOL1 + GFXCOLS/2)

#define	XMIN		-30
#define	XMAX		+30
#define	YMIN		-130
#define	YMAX		+130

#define	TXT_HEIGHT	15
#define	TXT_WIDTH	7

#define	PARAM_ROW1	(GFXROW1 + 10)
#define	VALUE_ROW1	(GFXROWN - 3*TXT_HEIGHT)

#define	LBL_COL		10
#define	STS_COL		(LBL_COL + 25*TXT_WIDTH)

#define	TICKRATE	20
#define	TICKSIZE	4

#define	PLOT_RAD	1
#define	ROOT_RAD	4

#define	COLOR_PLOT	COLOR_BLUE
#define	COLOR_ROOT	COLOR_RED
#define	COLOR_AXIS	COLOR_BLACK

typedef	int	BOOL ;
#define	TRUE	1
#define	FALSE	0

typedef struct
	{
	int	a, b, c ;
	int d ;
	int r1, r2 ;
	} TESTCASE ;

static void	PlotQuadratic(int a, int b, int c) ;
static void	PlotRoot(int x) ;
static void	DrawXAxis(void) ;
static void	DrawYAxis(void) ;
static void LEDs(int grn_on, int red_on) ;
static int	PutStringAt(int row, int col, char *fmt, ...) ;
static int	PutStatus(int row, int col, BOOL ok) ;

int main(void)
	{
	static TESTCASE testcase[] =
		{
		{+1,   0, -100, +400, +10, -10},	// 2 roots
		{-1, +30, -225,    0, +15, +15},	// 1 root
		{+1, +30, +240,  -60,   0,   0}		// Imaginary roots
		} ;
	int my_d, my_r1, my_r2 ;
	int a, b, c, d, r1, r2 ;
	int k, row ;

	InitializeHardware(HEADER, "Lab 4a: Solving Quadratics") ;
	LEDs(1, 0) ;

	k = 0 ;
	while (1)
		{
		ClearDisplay() ;

		a  = testcase[k].a ;
		b  = testcase[k].b ;
		c  = testcase[k].c ;
		d  = testcase[k].d ;
		r1 = testcase[k].r1 ;
		r2 = testcase[k].r2 ;

		row = PARAM_ROW1 ;
		row = PutStringAt(row, LBL_COL, "Coefficient A:%10d (x^2)", a) ;
		row = PutStringAt(row, LBL_COL, "Coefficient B:%10d (x^1)", b) ;
		row = PutStringAt(row, LBL_COL, "Coefficient C:%10d (x^0)", c) ;

		row = VALUE_ROW1 ;
		my_d = Discriminant(a, b, c) ;
		PutStringAt(row, LBL_COL, " Discriminant:%10d", my_d) ;
		row = PutStatus(row, STS_COL, my_d == d) ;

		if (my_d < 0.0)
			{
			PutStringAt(row, LBL_COL, "        Roots:%10s", "Imag") ;
			row = PutStatus(row, STS_COL, TRUE) ;
			}
		else if (my_d > 0.0)
			{
			my_r1 = Root1(a, b, c) ;
			PutStringAt(row, LBL_COL, "   First root:%10d", my_r1) ;
			row = PutStatus(row, STS_COL, my_r1 == r1) ;

			my_r2 = Root2(a, b, c) ;
			PutStringAt(row, LBL_COL, "  Second root:%10d", my_r2) ;
			row = PutStatus(row, STS_COL, my_r2 == r2) ;

			PlotRoot(my_r1) ;
			PlotRoot(my_r2) ;
			}
		else // if (my_d == 0)
			{
			my_r1 = Root1(a, b, c) ;
			PutStringAt(row, LBL_COL, "  Single root:%10d", my_r1) ;
			row = PutStatus(row, STS_COL, my_r1 == r1) ;

			PlotRoot(my_r1) ;
			}

		PlotQuadratic(a, b, c) ;
		WaitForPushButton() ;
		k = (k + 1) % ENTRIES(testcase) ;
		}

	return 0 ;
	}

static void PlotQuadratic(int a, int b, int c)
	{
	int oldRow = 0 ;
	int oldCol = 0 ;

	DrawXAxis() ;
	DrawYAxis() ;

	SetColor(COLOR_PLOT) ;
	for (int col4 = 0; col4 < GFXCOLS/4; col4++)
		{
		int ix, iy ;
		float dx ;

		ix = XMIN + col4 ;
		iy = Quadratic(ix, a, b, c) ;

		dx = 0.0 ;
		for (int dcol = 0; dcol < 4; dcol++, dx += 0.25)
			{
			int row, col, steps ;
			float rx, ry ;

			rx = ix + dx ;
			ry = iy + dx*(b + a*(2*rx + dx)) ;

			col = 4*col4 + dcol ;
			row = 0.5 + GFXROWN + (YMIN - ry) ;
			if (!oldRow) oldRow = row ;

			steps = row - oldRow ;
			if (steps < 0) steps = -steps ;
			for (int step = 0; step <= steps; step++)
				{
				if (step >= steps/2) oldCol = col ;
				if (GFXROW1 <= oldRow && oldRow <= GFXROWN) FillCircle(oldCol, oldRow, PLOT_RAD) ;
				if (oldRow < row) oldRow++ ;
				else if (oldRow > row) oldRow-- ;
				}
			}
		}
	}

static void PlotRoot(int x)
	{
	int col ;

	col = 4*(x - XMIN) ;

	if (col < ROOT_RAD || col > GFXCOLS - ROOT_RAD) return ;

	SetColor(COLOR_ROOT) ;
	FillCircle(col, AXISROW, ROOT_RAD) ;
	}

static void DrawXAxis(void)
	{
	int col, rowMin, rowMax ;

	SetColor(COLOR_AXIS) ;
	rowMin = AXISROW - TICKSIZE/2 ;
	rowMax = rowMin + TICKSIZE ;
	DrawLine(GFXCOL1, AXISROW, GFXCOLN, AXISROW) ;
	for (col = TICKRATE; col < GFXCOLS/2; col += TICKRATE)
		{
		DrawLine(AXISCOL + col, rowMin, AXISCOL + col, rowMax) ;
		DrawLine(AXISCOL - col, rowMin, AXISCOL - col, rowMax) ;
		}

	DisplayStringAt(AXISCOL - 4*TICKRATE - 12, AXISROW + 8, "-20") ;
	DisplayStringAt(AXISCOL - 2*TICKRATE - 12, AXISROW + 8, "-10") ;
	DisplayStringAt(AXISCOL + 2*TICKRATE - 12, AXISROW + 8, "+10") ;
	DisplayStringAt(AXISCOL + 4*TICKRATE - 12, AXISROW + 8, "+20") ;
	}

static void DrawYAxis(void)
	{
	int row, colMin, colMax ;

	SetColor(COLOR_AXIS) ;
	colMin = AXISCOL - TICKSIZE/2 ;
	colMax = colMin + TICKSIZE ;
	DrawLine(AXISCOL, GFXROW1, AXISCOL, GFXROWN) ;
	for (row = TICKRATE; row < GFXROWS/2; row += TICKRATE)
		{
		DrawLine(colMin, AXISROW + row, colMax, AXISROW + row) ;
		DrawLine(colMin, AXISROW - row, colMax, AXISROW - row) ;
		}

	DisplayStringAt(AXISCOL + 6, AXISROW - 2*TICKRATE - 5, "+40") ;
	DisplayStringAt(AXISCOL + 6, AXISROW + 2*TICKRATE - 5, "-40") ;
	}

static int PutStatus(int row, int col, BOOL ok)
	{
	if (!ok)
		{
		LEDs(0, 1) ;
		SetForeground(COLOR_WHITE) ;
		SetBackground(COLOR_RED) ;
		}

	PutStringAt(row, col, "%s", ok ? "CORRECT" : " WRONG ") ;

	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_WHITE) ;

	return row + TXT_HEIGHT ;
	}

static int PutStringAt(int row, int col, char *fmt, ...)
	{
	va_list args ;
	char text[100] ;

	va_start(args, fmt) ;
	vsprintf(text, fmt, args) ;
	va_end(args) ;

	DisplayStringAt(col, row, text) ;

	return row + TXT_HEIGHT ;
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

int32_t SquareRoot(int32_t n)
	{
	int32_t smallCandidate, largeCandidate ;

	if (n < 2) return n ;

	// Recursive call:
	smallCandidate = 2 * SquareRoot(n / 4) ;
	largeCandidate = smallCandidate + 1 ;

	if (largeCandidate * largeCandidate > n)
		{
		return smallCandidate ;
		}

	return largeCandidate ;
	}

