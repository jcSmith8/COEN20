/*
	This code was written to support the book, "ARM Assembly for Embedded Applications",
	by Daniel W. Lewis. Permission is granted to freely share this software provided
	that this notice is not removed. This software is intended to be used with a run-time
    library adapted by the author from the STM Cube Library for the 32F429IDISCOVERY 
    board and available for download from http://www.engr.scu.edu/~dlewis/book3.
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "library.h"
#include "graphics.h"

extern void				UseLDRB(void *dst, void *src) ;
extern void				UseLDRH(void *dst, void *src) ;
extern void				UseLDR(void *dst, void *src) ;
extern void				UseLDRD(void *dst, void *src) ;
extern void				UseLDM(void *dst, void *src) ;

typedef int				BOOL ;
#define	FALSE			0
#define	TRUE			1

typedef struct
	{
	uint16_t			hue ;	// 0 to 359 degrees
	uint8_t				sat ;	// 0 to 100 percent
	uint8_t				val ;	// 0 to 100 percent
	} HSV ;

typedef struct
	{
	uint8_t				blu ;	// 0 to 255
	uint8_t				grn ;	// 0 to 255
	uint8_t				red ;	// 0 to 255
	} RGB ;

typedef struct
	{
	char *				label ;
	void				(*func)() ;
	int					index ;
	unsigned			cycles ;
	} RESULT ;

static int				Check(uint8_t *src, uint8_t *dst) ;
static int				Compare(const void *p1, const void *p2) ;
static void				Delay(uint32_t msec) ;
static void				FillSpectrum(int x, int y, int height) ;
static uint32_t 		GetTimeout(uint32_t msec) ;
static void				LEDs(int grn_on, int red_on) ;
static void				Setup(uint8_t *src, uint8_t *dst) ;
static void				ShowBest(RESULT results[]) ;
static void				ShowResult(int which, RESULT results[], unsigned maxCycles) ;
static unsigned			UseDMA(void) ;
static RGB				HSV2RGB(HSV *hsv) ;

#define	BAR_OFFSET			70
#define	BAR_WIDTH			30
#define	MAX_HEIGHT			210
#define	FUNCTIONS			7
#define CPU_CLOCK_SPEED_MHZ 168

#define	MIN(a,b)	((a < b) ? a : b)
#define	MAX(a,b)	((a > b) ? a : b)

#define FONT_WIDTH		7
#define FONT_HEIGHT		12

static uint8_t src[512] __attribute__ ((aligned (1024))) ; // DMA burst mode cannot cross 1KB boiundary
static uint8_t dst[512] __attribute__ ((aligned (1024))) ; // DMA burst mode cannot cross 1KB boiundary

int main(void)
	{
	static RESULT results[FUNCTIONS] =
		{
		{"LDRB",	UseLDRB},
		{"LDRH",	UseLDRH},
		{"LDR",		UseLDR},
		{"LDRD",	UseLDRD},
		{"LDM",		UseLDM},
		{"mcpy",	(void (*)()) memcpy},
		{"DMA",		NULL}
		} ;
	static uint32_t iparams[] = {(uint32_t) dst, (uint32_t) src, 512} ;
	unsigned maxCycles, ovhd, dummy[2] ;
	int which, srcErr, dstErr ;

	InitializeHardware(HEADER, "Lab 3: Copying Data Quickly") ;
	LEDs(0, 1) ;

	srcErr = (((unsigned) src) & 0x3FF) != 0 ;
	dstErr = (((unsigned) src) & 0x3FF) != 0 ;
	if (srcErr) printf("src crosses 1KB boundary (%08X)\n", (unsigned) src) ;
	if (dstErr) printf("dst crosses 1KB boundary (%08X)\n", (unsigned) dst) ;
	if (srcErr || dstErr) while (1) ;

	LEDs(1, 0) ;
	ovhd = CountCycles(CallReturnOverhead, dummy, dummy, dummy) ;
	for (which = 0; which < FUNCTIONS - 1; which++)
		{
		Setup(src, dst) ;
		results[which].cycles = CountCycles(results[which].func, iparams, dummy, dummy) - ovhd ;
		results[which].index  = Check(src, dst) ;
		}
	Setup(src, dst) ;
	results[which].cycles = UseDMA() ;
	results[which].index  = Check(src, dst) ;

	qsort(results, FUNCTIONS, sizeof(RESULT), Compare) ;
	maxCycles = results[0].cycles ;

	SetColor(COLOR_BLACK) ;
	DrawHLine(0, BAR_OFFSET + MAX_HEIGHT, XPIXELS) ;
	for (which = 0; which < FUNCTIONS; which++)
		{
		ShowResult(which, results, maxCycles) ;
		}
	ShowBest(results) ;

	return 0 ;
	}

static void Setup(uint8_t *src, uint8_t *dst)
	{
	int k ;

	for (k = 0; k < 512; k++)
		{
		*src++ = rand() ;
		*dst++ = rand() ;
		}
	}

static int Check(uint8_t *src, uint8_t *dst)
	{
	int k ;

	for (k = 0; k < 512; k++)
		{
		if (*src++ != *dst++) return k ;
		}
	return -1 ;
	}

static void ShowResult(int which, RESULT results[], unsigned maxCycles)
	{
	float percent = ((float) results[which].cycles) / maxCycles ;
	char text[100] ;
	int x, y, offset ;

	x = (XPIXELS / FUNCTIONS)*which + XPIXELS / (2*FUNCTIONS) - BAR_WIDTH / 2 ;
	y = MAX_HEIGHT*(1 - percent) + BAR_OFFSET ;

	offset = (BAR_WIDTH - FONT_WIDTH*strlen(results[which].label)) / 2 ;
	DisplayStringAt(x + offset, BAR_OFFSET + MAX_HEIGHT + 5, results[which].label) ;
	if (results[which].index >= 0)
		{
		SetColor(COLOR_RED) ;
		FillRect(x, y, BAR_WIDTH, (unsigned) (percent*MAX_HEIGHT)) ;
		SetColor(COLOR_BLACK) ;
		DrawRect(x, y, BAR_WIDTH - 1, (unsigned) (percent*MAX_HEIGHT)) ;
		}
	else FillSpectrum(x, y, (unsigned) (percent*MAX_HEIGHT)) ;

	sprintf(text, "%u", results[which].cycles) ;
	offset = (BAR_WIDTH - FONT_WIDTH*strlen(text)) / 2 ;
	DisplayStringAt(x + offset, y - 13, text) ;

	if (results[which].index < 0) return ;

	LEDs(0, 1) ;
	SetForeground(COLOR_WHITE) ;
	SetBackground(COLOR_RED) ;
	sprintf(text, "Use%s failed at Index %d!", results[which].label, results[which].index) ;
	DisplayStringAt(15, YPIXELS/3 + 15*which, text) ;
	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_WHITE) ;
	}

static void FillSpectrum(int x, int ymin, int height)
	{
#	define	HUE_BEST	120
#	define	HUE_RNGE	190
	uint32_t color ;
	float percent ;
	int y, ybtm, hue ;
	HSV hsv ;
	RGB rgb ;

	hsv.sat = hsv.val = 100 ;
	ybtm = ymin + height ;
	if (ybtm >= BAR_OFFSET + MAX_HEIGHT)
		ybtm = BAR_OFFSET + MAX_HEIGHT - 1 ;
	for (y = 0; y < height; y++)
		{
		percent = (float) y / MAX_HEIGHT ;
		hue = HUE_BEST - percent * HUE_RNGE ;
		hsv.hue = (hue < 0) ? hue + 360 : hue ;
		rgb = HSV2RGB(&hsv) ;
		color = 0xFF000000 | *((uint32_t *) &rgb) ;
		SetColor(color) ;
		DrawHLine(x, ybtm - y, BAR_WIDTH) ;
		SetColor(COLOR_BLACK) ;
		DrawRect(x, ybtm - y - 1, BAR_WIDTH, y + 2) ;
		Delay(5) ;
		}
	}

static unsigned UseDMA(void)
	{
	static		uint32_t	const MBURST		= (1 << 23) ;	// Write burst of 4 beats
	static		uint32_t	const PBURST		= (1 << 21) ;	// Read burst of 4 beats
	static		uint32_t	const MSIZE			= (2 << 13) ;	// Write 32-bit words
	static		uint32_t	const PSIZE			= (2 << 11) ;	// Read 32-bit words
	static		uint32_t	const MINC			= (1 << 10) ;	// Autoincr dst adrs
	static		uint32_t	const PINC			= (1 <<  9) ;	// Autoincr src adrs
	static		uint32_t	const DIR			= (2 <<  6) ;	// Memory-To-Memory
	static		uint32_t	const EN			= (1 <<  0) ;	// "Go"
	static		uint32_t	const FTH			= (3 <<  0) ;	// FIFO threshold = full
	static		uint32_t	const DMDIS			= (1 <<  2) ;	// Direct mode disabled
	static		uint32_t	const TCIF0			= (1 <<  5) ;	// Transfer complete flag
	volatile	uint32_t *	const pDMA2_LISR	= (uint32_t *) 0x40026400 ;
	static		uint32_t *	const pDMA2_LIFCR	= (uint32_t *) 0x40026408 ;
	static		uint32_t *	const pDMA2_S0CR	= (uint32_t *) 0x40026410 ;
	static		uint32_t *	const pDMA2_S0NDTR	= (uint32_t *) 0x40026414 ;
	static		void **		const pDMA2_S0PAR	= (void **)    0x40026418 ;
	static		void **		const pDMA2_S0M0AR	= (void **)    0x4002641C ;
	static		uint32_t *	const pDMA2_S0FCR	= (uint32_t *) 0x40026424 ;
	static		uint32_t *	const pRCC_AHB1ENR	= (uint32_t *) 0x40023830 ;

	uint32_t strt, stop, zero ;

	*pRCC_AHB1ENR |= (1 << 22) ; 			// Enable DMA Clock

	*pDMA2_S0CR     = 0 ; 					// Disable DMA
	*pDMA2_S0PAR    = src ; 				// Setup src address
	*pDMA2_S0M0AR   = dst ; 				// Setup dst address
	*pDMA2_S0NDTR   = 512/4 ; 				// Setup word count
	*pDMA2_S0FCR	= DMDIS|FTH ;			// Setup FIFO

	zero = GetClockCycleCount() ;
	*pDMA2_LIFCR	= TCIF0 ;				// Clear TCIF0
	while ((*pDMA2_LISR & TCIF0) != 0) ;	// wait for TCIF0 = 0

	strt = GetClockCycleCount() ;
	*pDMA2_S0CR = MBURST|PBURST|MSIZE|PSIZE|MINC|PINC|DIR|EN ;
	while ((*pDMA2_LISR & TCIF0) == 0) ;	// wait for TCIF0 = 1
	stop = GetClockCycleCount() ;

	return (stop - strt) - (strt - zero) ;
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

static RGB HSV2RGB(HSV *hsv)
	{
	double f, p, q, t, h, s, v ;
	double *pRed[] = {&v, &q, &p, &p, &t, &v} ;
	double *pGrn[] = {&t, &v, &v, &q, &p, &p} ;
	double *pBlu[] = {&p, &p, &t, &v, &v, &q} ;
	unsigned val = hsv->val ;
	unsigned sat = hsv->sat ;
	unsigned hue = hsv->hue ;
	RGB rgb ;
	int i ;

	val = MIN(val, 100) ;
	if (sat == 0)
		{
		// achromatic (grey)
		rgb.red = rgb.grn = rgb.blu = (val * 255 + 50) / 100 ;
		return rgb ;
		}

	sat = MIN(sat, 100) ;
	s = sat / 100.0 ;
	h = (hue % 360) / 60.0 ;	// 0.0 <= h < 6.0
	i = (unsigned) h ;		//   0 <= i < 6
	f = h - (double) i ;		//   0 <= f < 1.0

	v = val / 100.0 ;
	p = v * (1.0 - s) ;
	q = v * (1.0 - s*f) ;
	t = v * (1.0 - s*(1.0 - f)) ;

	rgb.red = *pRed[i] * 255 + 0.5 ;
	rgb.grn = *pGrn[i] * 255 + 0.5 ;
	rgb.blu = *pBlu[i] * 255 + 0.5 ;

	return rgb ;
	}

static uint32_t GetTimeout(uint32_t msec)
	{
	uint32_t cycles = 1000 * msec * CPU_CLOCK_SPEED_MHZ ;
	return GetClockCycleCount() + cycles ;
	}

static void Delay(uint32_t msec)
	{
	uint32_t timeout = GetTimeout(msec) ;
	while ((int) (timeout - GetClockCycleCount()) > 0) ;
	}

static int Compare(const void *p1, const void *p2)
	{
	RESULT *r1 = (RESULT *) p1 ;
	RESULT *r2 = (RESULT *) p2 ;
	return r2->cycles - r1->cycles ;
	}

static void ShowBest(RESULT results[])
	{
#	define RESULT_Y			100
	char best[100], rate[100] ;
	int chars1, chars2, chars, x ;
	float bps ;

	bps = (512 * CPU_CLOCK_SPEED_MHZ) / results[FUNCTIONS-1].cycles ;
	sprintf(best, " Fastest: Use%-4s", results[FUNCTIONS-1].label) ;
	sprintf(rate, " Rate: %.0f MB/sec", bps) ;
	chars1 = strlen(best) ; chars2 = strlen(rate) ;
	chars = MAX(chars1, chars2) ;
	x = XPIXELS - FONT_WIDTH * (chars + 5) ;
	SetForeground(COLOR_YELLOW) ;
	FillRect(x, RESULT_Y, FONT_WIDTH*(chars + 1), 3*FONT_HEIGHT) ;
	SetForeground(COLOR_BLACK) ;
	DrawRect(x, RESULT_Y, FONT_WIDTH*(chars + 1), 3*FONT_HEIGHT) ;
	SetBackground(COLOR_YELLOW) ;
	DisplayStringAt(x + FONT_WIDTH/2, RESULT_Y + (1*FONT_HEIGHT/2), best) ;
	DisplayStringAt(x + FONT_WIDTH/2, RESULT_Y + (3*FONT_HEIGHT/2), rate) ;
	}



