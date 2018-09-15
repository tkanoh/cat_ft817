/*
 * Copyright (c) 2014
 *      Tamotsu Kanoh <kanoh@kanoh.org>. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither my name nor the names of its contributors may be used to 
 *    endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */ 

#define MODE_LSB	0x00
#define MODE_USB	0x01
#define MODE_CW		0x02
#define MODE_CW_R	0x03
#define MODE_AM		0x04
#define MODE_FM		0x08
#define MODE_DIG	0x0a
#define MODE_PKT	0x0c
#define MODE_NONE	0xff

#define DEF_MODE	MODE_AM

struct cmd_type_t {
	char type[8];
	char cmd;
};

static struct cmd_type_t mode_type[] = {
	"LSB",	MODE_LSB,	
	"USB",	MODE_USB,
	"CW",	MODE_CW,
	"CW-R",	MODE_CW_R,
	"AM",	MODE_AM,
	"FM",	MODE_FM,
	"DIG",	MODE_DIG,
	"PKT",	MODE_PKT,
	"",	MODE_NONE
};

struct band_plan_t {
	int start;	
	int end;	
	char mode;	
};

static struct band_plan_t band_plan[] = { 
 	   135700,    135800, MODE_CW,
	  1810000,   1825000, MODE_CW,
	  1907500,   1912500, MODE_CW,
	  3500000,   3520000, MODE_CW,
	  3520000,   3525000, MODE_CW,
	  3525000,   3530000, MODE_LSB,
	  3530000,   3575000, MODE_LSB,
	  3599000,   3612000, MODE_LSB,
	  3680000,   3687000, MODE_LSB,
	  3702000,   3716000, MODE_LSB,
	  3745000,   3770000, MODE_LSB,
	  3791000,   3805000, MODE_CW,
	  7000000,   7025000, MODE_CW,
	  7025000,   7030000, MODE_CW,
	  7030000,   7100000, MODE_LSB,
	  7100000,   7200000, MODE_LSB,
	 10100000,  10130000, MODE_CW,
	 10130000,  10150000, MODE_CW,
	 14000000,  14070000, MODE_CW,
	 14070000,  14100000, MODE_CW,
	 14100000,  14112000, MODE_USB,
	 14112000,  14350000, MODE_USB,
	 18068000,  18100000, MODE_CW,
	 18100000,  18110000, MODE_CW,
	 18110000,  18168000, MODE_USB,
	 21000000,  21070000, MODE_CW,
	 21070000,  21125000, MODE_CW,
	 21125000,  21150000, MODE_CW,
	 21150000,  21450000, MODE_USB,
	 24890000,  24920000, MODE_CW,
	 24920000,  24930000, MODE_CW,
	 24930000,  24990000, MODE_USB,
	 28000000,  28070000, MODE_CW,
	 28070000,  28150000, MODE_CW,
	 28150000,  28200000, MODE_CW,
	 28200000,  29000000, MODE_USB,
	 29000000,  29300000, MODE_FM,
	 29300000,  29510000, MODE_CW,
	 29510000,  29590000, MODE_CW,
	 29590000,  29610000, MODE_FM,
	 29610000,  29700000, MODE_CW,
	 50000000,  50100000, MODE_CW,
	 50100000,  50300000, MODE_USB,
	 50300000,  51000000, MODE_USB,
	 51000000,  52000000, MODE_FM,
	 52000000,  52300000, MODE_DIG,
	 52300000,  52500000, MODE_USB,
	 52500000,  52900000, MODE_DIG,
	 52900000,  54000000, MODE_FM,
	 76000000, 108000000, MODE_FM,
	144000000, 144020000, MODE_USB,
	144020000, 144100000, MODE_CW,
	144100000, 144400000, MODE_USB,
	144400000, 144500000, MODE_USB,
	144500000, 144600000, MODE_DIG,
	144600000, 144700000, MODE_DIG,
	144700000, 145650000, MODE_FM,
	145650000, 145800000, MODE_FM,
	145800000, 146000000, MODE_FM,
	430000000, 430100000, MODE_CW,
	430100000, 430500000, MODE_USB,
	430500000, 430700000, MODE_USB,
	430700000, 431000000, MODE_DIG,
	431000000, 431400000, MODE_DIG,
	431400000, 431900000, MODE_FM,
	431900000, 432100000, MODE_USB,
	432100000, 434000000, MODE_FM,
	434000000, 435000000, MODE_FM,
	435000000, 438000000, MODE_FM,
	438000000, 439000000, MODE_FM,
	439000000, 440000000, MODE_FM,
	        0,         0, MODE_NONE 
};

struct frange_t {
	int fmin;
	int fmax;
	char mode_limit;
};

#define	FM_ONLY		MODE_FM
#define ALL_MODE	MODE_NONE

static struct frange_t frange[] = {
	   100000,  56000000, ALL_MODE,
	 76000000, 108000000, FM_ONLY,
	108000000, 154000000, ALL_MODE,
	420000000, 470000000, ALL_MODE,
	        0,         0
};

#define MAX_CLAR	9990
#define MAX_REP_SHIFT	99000000

struct ibp_t {
	char band[4];
	int freq;
	char mode;
};

static struct ibp_t ibp[] = {
 	"2.5", 2500000, MODE_AM,
 	 "5",  5000000, MODE_AM,
	"10", 10000000, MODE_AM,
	"14", 14100000, MODE_CW,
	"15", 15000000, MODE_AM,
	"18", 18110000, MODE_CW,
	"20", 20000000, MODE_AM,
	"21", 21150000, MODE_CW,
	"24", 24930000, MODE_CW,
	"28", 28200000, MODE_CW,
	"50", 50010000, MODE_CW,
	  "",        0, MODE_NONE
};

#define PAD		0x00
#define	CMD_LOCK_ON	0x00
#define	CMD_LOCK_OFF	0x80
#define	CMD_PTT_ON	0x08
#define	CMD_PTT_OFF	0x88
#define	CMD_SET_FREQ	0x01
#define	CMD_SET_MODE	0x07
#define	CMD_CLAR_ON	0x05
#define	CMD_CLAR_OFF	0x85
#define CMD_SET_CLAR	0xf5
#define PAR_P_OFFSET	0x00
#define PAR_N_OFFSET	0x80
#define CMD_TOGLE_VFO	0x81
#define CMD_SPLIT_ON	0x02
#define CMD_SPLIT_OFF	0x82
#define CMD_SET_REP	0x09
#define PAR_P_SHIFT	0x49
#define PAR_N_SHIFT	0x09
#define PAR_SIMPLEX	0x89
#define CMD_SET_SHIFT	0xf9
#define CMD_SET_SQL	0x0a
#define PAR_SQL_DCS	0x0a
#define PAR_SQL_CTCSS	0x2a
#define PAR_SQL_ENCODER	0x4a
#define PAR_SQL_OFF	0x8a
#define CMD_SET_TONE	0x0b
#define CMD_SET_DCS	0x0c
#define CMD_RX_STAT	0xe7
#define CMD_TX_STAT	0xf7
#define CMD_FR_STAT	0x03
#define CMD_POWER_ON	0x0f
#define CMD_POWER_OFF	0x8f
#define	CMD_ERROR	0xff

static struct cmd_type_t sql_type[] = {
	"DCS",	   PAR_SQL_DCS,
	"CTCSS",   PAR_SQL_CTCSS,
	"ENCODER", PAR_SQL_ENCODER,
	"OFF",     PAR_SQL_OFF,
	"", 	   CMD_ERROR
};

static float tones[] = {
	 67.0,  69.3,  71.9,  74.4,  77.0,  79.7,
	 82.5,  85.4,  88.5,  91.5,  94.8,  97.4,
	100.0, 103.5, 107.2, 110.9, 114.8, 118.8,
	123.0, 127.3, 131.8, 136.5, 141.3, 146.2,
	151.4, 156.7, 159.8, 162.2, 165.5, 167.9,
	171.3, 173.8, 177.3, 179.9, 183.5, 186.2,
	189.9, 192.8, 196.6, 199.5, 203.5, 206.5,
	210.7, 218.1, 225.7, 229.1, 233.6, 241.8,
	250.3, 254.1, 0
};

static int dcs_codes[] = {
	023, 025, 026, 031, 032, 036, 043, 047, 051, 053,
	054, 065, 071, 072, 073, 074, 114, 115, 116, 122,
	125, 131, 132, 134, 143, 145, 152, 155, 156, 162, 
	165, 172, 174, 205, 212, 223, 225, 226, 243, 244,
	245, 246, 251, 252, 255, 261, 263, 265, 266, 271,
	274, 306, 311, 315, 325, 331, 332, 343, 346, 351,
	356, 364, 365, 371, 411, 412, 413, 423, 431, 432,
	445, 446, 452, 454, 455, 462, 464, 465, 466, 503,
	506, 516, 523, 526, 532, 546, 565, 606, 612, 624,
	627, 631, 632, 654, 662, 664, 703, 712, 723, 731,
	732, 724, 743, 754, 0
};

static struct cmd_type_t stat_type[] = {
	"RX",	CMD_RX_STAT,
	"TX",	CMD_TX_STAT,
	"FREQ",	CMD_FR_STAT,
	"FR",	CMD_FR_STAT,
	"MODE",	CMD_FR_STAT,
	"",	CMD_ERROR
};

struct stat_str_t {
	char bit;	
	char name[32];	
	char stat[2][32];
};

static struct stat_str_t rx_stat_str[] = {
	5, "Discriminator Centering", "Centered", "Off-Center",
	6, "CTCSS/DCS Code", "Matched", "Un-Matched",
	7, "Squelch Status", "OFF (Signal Present)", "ON (No Signal)",
	0, "","",""
};

static struct stat_str_t tx_stat_str[] = {
	5, "SPLIT Status", "OFF", "ON",
	6, "HI SWR Status", "OFF", "ON",
	7, "PTT", "OFF", "ON",
	0, "","",""
};
