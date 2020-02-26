#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "library.h"
#include "graphics.h"
#include "touch.h"

typedef struct
	{
	uint32_t			twenties ;
	uint32_t			tens ;
	uint32_t			fives ;
	uint32_t			ones ;
	} BILLS ;

typedef struct
	{
	uint32_t			quarters ;
	uint32_t			dimes ;
	uint32_t			nickels ;
	uint32_t			pennies ;
	} COINS ;

extern void				Bills(uint32_t dollars, BILLS *paper) ;
extern void				Coins(uint32_t cents, COINS *coins) ;

typedef int				BOOL ;
#define	FALSE			0
#define	TRUE			1

typedef struct
	{
	char *				lbl ;
	uint32_t			xpos ;
	uint32_t			ypos ;
	uint32_t *			pval ;
	uint32_t			mult ;
	uint32_t			min ;
	uint32_t			max ;
	} ADJUST ;

// Public fonts defined in run-time library
typedef struct
	{
	const uint8_t *		table ;
	const uint16_t		Width ;
	const uint16_t		Height ;
	} sFONT ;

extern sFONT			Font8, Font12, Font16, Font20, Font24 ;

#define	FONT_DFLT		Font16
#define	FONT_HEIGHT		16

#define	FONT_ADJ		Font20

#define	MARGIN			10

#define	XPOS_ADJUST		131
#define	XPOS_LABEL		12
#define	XPOS_SUBTTL		153
#define	XPOS_TOTAL		131

#define	YPOS_DOLLARS	60
#define	YPOS_CENTS		85
#define	YPOS_TOTAL		(YPOS_PENNIES	+ 2*FONT_HEIGHT)

#define	YPOS_TWENTIES	(YPOS_CENTS		+ 5*FONT_HEIGHT/2)
#define	YPOS_TENS		(YPOS_TWENTIES	+ 1*FONT_HEIGHT)
#define	YPOS_FIVES		(YPOS_TENS		+ 1*FONT_HEIGHT)
#define	YPOS_ONES		(YPOS_FIVES		+ 1*FONT_HEIGHT)

#define	YPOS_QUARTERS	(YPOS_ONES		+ 3*FONT_HEIGHT/2)
#define	YPOS_DIMES		(YPOS_QUARTERS	+ 1*FONT_HEIGHT)
#define	YPOS_NICKELS	(YPOS_DIMES		+ 1*FONT_HEIGHT)
#define	YPOS_PENNIES	(YPOS_NICKELS	+ 1*FONT_HEIGHT)

#define	ADJOFF_XMINUS	0
#define	ADJOFF_XVALUE	(ADJOFF_XMINUS +  1*FONT_ADJ.Width)
#define	ADJOFF_XPLUS	(ADJOFF_XVALUE +  5*FONT_ADJ.Width)

#define	TS_XFUDGE		-8
#define	TS_YFUDGE		0

#define	ENTRIES(a)		(sizeof(a)/sizeof(a[0]))

// Private functions defined in this file
static BOOL				Adjusted(ADJUST adjust[], int adjusts) ;
static BOOL				Between(uint32_t min, uint32_t val, uint32_t max) ;
static void				Delay(uint32_t msec) ;
static void				DisplayAdjusts(ADJUST adjust[], int adjusts) ;
static void				DisplayChange(uint32_t entered) ;
static uint32_t			GetTimeout(uint32_t msec) ;
static void				InitializeTouchScreen(void) ;
static void				LEDs(BOOL grn_on, BOOL red_on) ;
static void				PutStringAt(uint32_t x, uint32_t y, char *format, ...) ;
static void				SetFontSize(sFONT *pFont) ;
static void				SetUpAdjusts(ADJUST adjust[], int adjusts) ;

static BILLS			paper ;
static COINS			coins ;

int main()
	{
	static uint32_t dollars, cents ;
	static ADJUST adjust[] =
		{
		{" Dollars: ", XPOS_ADJUST, YPOS_DOLLARS, &dollars, 100, 0, 99},
		{"   Cents: ", XPOS_ADJUST, YPOS_CENTS,   &cents,     1, 0, 99}
		} ;
	uint32_t delay1, delay2 ;

	InitializeHardware(HEADER, "Lab 7b: Making Change") ;
	InitializeTouchScreen() ;

	dollars = GetRandomNumber() % 100 ;
	cents	= GetRandomNumber() % 100 ;
	SetUpAdjusts(adjust, ENTRIES(adjust)) ;

	delay1 = delay2 = 0 ;
	for (;;)
		{
		Bills(dollars, &paper) ;
		Coins(cents, &coins) ;
		DisplayChange(100*dollars + cents) ;

		Delay(delay1) ;
		delay1 = delay2 ;
		while (1)
			{
			if (PushButtonPressed())
				{
				dollars = cents = 0 ;
				DisplayAdjusts(adjust, ENTRIES(adjust)) ;
				break ;
				}

			if (TS_Touched())
				{
				if (Adjusted(adjust, ENTRIES(adjust)))
					{
					delay2 = 30 ;
					break ;
					}
				}
			else delay1 = 500 ;
			}
		}

	return 0 ;
	}

static void DisplayChange(uint32_t entered)
	{
	typedef struct
		{
		int				ypos ;
		uint32_t *		coins ;
		int				value ;
		char	*		single ;
		char	*		plural ;
		} DISPLAY ;
	static DISPLAY display[] =
		{
		{YPOS_TWENTIES,	&paper.twenties,	2000,	"Twenty",	"Twenties"},
		{YPOS_TENS,		&paper.tens,		1000,	"Ten",		"Tens"},
		{YPOS_FIVES,	&paper.fives,		 500,	"Five",		"Fives"},
		{YPOS_ONES,		&paper.ones,		 100,	"One",		"Ones"},
		{YPOS_QUARTERS,	&coins.quarters,	  25,	"Quarter",	"Quarters"},
		{YPOS_DIMES,	&coins.dimes,		  10,	"Dime",		"Dimes"},
		{YPOS_NICKELS,	&coins.nickels,		   5,	"Nickel",	"Nickels"},
		{YPOS_PENNIES,	&coins.pennies,		   1,	"Penny",	"Pennies"}
		} ;
	static BOOL init = TRUE ;
	uint32_t total ;
	DISPLAY *dp ;
	char *label ;
	int k ;

	if (init)
		{
		SetForeground(COLOR_YELLOW) ;
		FillRect(MARGIN, YPOS_TWENTIES - FONT_DFLT.Height/2, XPIXELS - 2*MARGIN, 19*FONT_DFLT.Height/2) ;
		SetForeground(COLOR_RED) ;
		DrawRect(MARGIN, YPOS_TWENTIES - FONT_DFLT.Height/2, XPIXELS - 2*MARGIN, 19*FONT_DFLT.Height/2) ;
		init = FALSE ;
		}

	SetForeground(COLOR_YELLOW) ;
	FillRect(XPOS_SUBTTL, YPOS_TWENTIES, 5*FONT_DFLT.Width, 17*FONT_DFLT.Height/2) ;

	total = 0 ;
	SetFontSize(&FONT_DFLT) ;
	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_YELLOW) ;
	dp = display ;
	for (k = 0; k < ENTRIES(display); k++, dp++)
		{
		int cents = *dp->coins * dp->value ;
		PutStringAt(XPOS_LABEL, dp->ypos, "%2d %s", (int) *dp->coins, *dp->coins == 1 ? dp->single : dp->plural) ;
		if (*dp->coins != 0) PutStringAt(XPOS_SUBTTL, dp->ypos,	"%2d.%02d", cents / 100, cents % 100) ;
		total += cents ;
		}

	SetForeground(COLOR_BLACK) ;
	SetBackground(COLOR_WHITE) ;
	SetFontSize(&FONT_DFLT) ;
	label = (total == entered) ? "Total: " : "Incorrect! " ;
	PutStringAt(XPOS_TOTAL - strlen(label)*FONT_DFLT.Width, YPOS_TOTAL, label) ;

	SetForeground(total == entered ? COLOR_BLACK : COLOR_WHITE) ;
	SetBackground(total == entered ? COLOR_WHITE : COLOR_RED) ;
	SetFontSize(&FONT_ADJ) ;
	PutStringAt(XPOS_TOTAL, YPOS_TOTAL, "$%2d.%02d", (int) total / 100, (int) total % 100) ;

	LEDs(total == entered, total != entered) ;
	}

static void SetFontSize(sFONT *Font)
	{
	extern void BSP_LCD_SetFont(sFONT *) ;
	BSP_LCD_SetFont(Font) ;
	}

static void PutStringAt(uint32_t x, uint32_t y, char *format, ...)
	{
	va_list args ;
	char text[100] ;

	va_start(args, format) ;
	vsprintf(text, format, args) ;
	va_end(args) ;

	DisplayStringAt(x, y, text) ;
	}

static void LEDs(BOOL grn_on, BOOL red_on)
	{
	static uint32_t * const pGPIOG_MODER	= (uint32_t *) 0x40021800 ;
	static uint32_t * const pGPIOG_ODR		= (uint32_t *) 0x40021814 ;
	
	*pGPIOG_MODER |= (1 << 28) | (1 << 26) ;	// output mode
	*pGPIOG_ODR &= ~(3 << 13) ;			// both off
	*pGPIOG_ODR |= (grn_on ? 1 : 0) << 13 ;
	*pGPIOG_ODR |= (red_on ? 1 : 0) << 14 ;
	}

static void InitializeTouchScreen(void)
	{
	static char *message[] =
		{
		"If this message remains on",
		"the screen, there is an",
		"initialization problem with",
		"the touch screen. This does",
		"NOT mean that there is a",
		"problem with your code.",
		" ",
		"To correct the problem,",
		"disconnect and reconnect",
		"the power.",
		NULL
		} ;
	char **pp ;
	unsigned x, y ;

	x = 25 ;
	y = 100 ;
	for (pp = message; *pp != NULL; pp++)
		{
		DisplayStringAt(x, y, *pp) ;
		y += 12 ;
		}
	TS_Init() ;
	ClearDisplay() ;
	}

static void SetUpAdjusts(ADJUST adjust[], int adjusts)
	{
	ADJUST *adj ;
	int k ;

	adj = adjust ;
	for (k = 0; k < adjusts; k++, adj++)
		{
		SetFontSize(&FONT_DFLT) ;
		SetForeground(COLOR_BLACK) ;
		SetBackground(COLOR_WHITE) ;
		DisplayStringAt(adj->xpos + ADJOFF_XMINUS - FONT_DFLT.Width*strlen(adj->lbl), adj->ypos, adj->lbl) ;

		SetFontSize(&FONT_ADJ) ;
		SetForeground(COLOR_WHITE) ;
		SetBackground(COLOR_BLACK) ;
		DisplayChar(adj->xpos + ADJOFF_XMINUS, adj->ypos, '-') ;

		SetForeground(COLOR_BLACK) ;
		SetBackground(COLOR_WHITE) ;
		DrawRect(adj->xpos + ADJOFF_XMINUS - 1, adj->ypos - 1, 7*FONT_ADJ.Width + 1, FONT_ADJ.Height + 1) ;

		SetForeground(COLOR_WHITE) ;
		SetBackground(COLOR_BLACK) ;
		DisplayChar(adj->xpos + ADJOFF_XPLUS, adj->ypos, '+') ;
		}

	DisplayAdjusts(adjust, adjusts) ;
	}

static void DisplayAdjusts(ADJUST adjust[], int adjusts)
	{
	uint32_t amount ;
	char text[100] ;
	ADJUST *adj ;
	int k ;

	SetFontSize(&FONT_ADJ) ;
	adj = adjust ;
	for (k = 0; k < adjusts; k++, adj++)
		{
		SetForeground(COLOR_WHITE) ;
		SetBackground(*adj->pval == adj->max ? COLOR_RED : COLOR_DARKGREEN) ;
		DisplayChar(adj->xpos + ADJOFF_XPLUS, adj->ypos, '+') ;
		SetBackground(*adj->pval == adj->min ? COLOR_RED : COLOR_DARKGREEN) ;
		DisplayChar(adj->xpos + ADJOFF_XMINUS, adj->ypos, '-') ;

		SetForeground(COLOR_BLACK) ;
		SetBackground(COLOR_WHITE) ;
		amount = *adj->pval * adj->mult ;
		sprintf(text, "%2d.%02d", (int) amount / 100, (int) amount % 100) ;
		DisplayStringAt(adj->xpos + ADJOFF_XVALUE, adj->ypos, text) ;
		}
	}

static BOOL Adjusted(ADJUST adjust[], int adjusts)
	{
	ADJUST *adj ;
	int k, x, y ;

	x = TS_GetX() + TS_XFUDGE ;
	y = TS_GetY() + TS_YFUDGE ;

	adj = adjust ;
	for (k = 0; k < adjusts; k++, adj++)
		{
		if (Between(adj->ypos, y, adj->ypos + FONT_DFLT.Height - 1))
			{
			if (Between(adj->xpos + ADJOFF_XMINUS, x, adj->xpos + ADJOFF_XMINUS + FONT_DFLT.Width - 1))
				{
				if (*adj->pval > adj->min) --*adj->pval ;
				DisplayAdjusts(adjust, adjusts) ;
				return TRUE ;
				}

			if (Between(adj->xpos + ADJOFF_XPLUS, x, adj->xpos + ADJOFF_XPLUS + FONT_DFLT.Width - 1))
				{
				if (*adj->pval < adj->max) ++*adj->pval ;
				DisplayAdjusts(adjust, adjusts) ;
				return TRUE ;
				}
			}
		}

	return FALSE ;
	}

static uint32_t GetTimeout(uint32_t msec)
	{
#	define	CPU_CLOCK_SPEED_MHZ 168
	uint32_t cycles = 1000 * msec * CPU_CLOCK_SPEED_MHZ ;
	return GetClockCycleCount() + cycles ;
	}

static void Delay(uint32_t msec)
	{
	uint32_t timeout = GetTimeout(msec) ;
	while ((int) (timeout - GetClockCycleCount()) > 0) ;
	}

static BOOL Between(uint32_t min, uint32_t val, uint32_t max)
	{
	return (min <= val && val <= max) ;
	}

