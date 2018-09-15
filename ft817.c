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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/wait.h>
#include "ft817.h"

#ifndef TTY_DEV
#define	TTY_DEV		"/dev/tty00"
#endif

#ifndef B_FT817
#define B_FT817		B4800
#endif

#define TIMEOUT		2

#define HZ		1.0
#define KHZ		1000.0
#define MHZ		1000000.0

#define	UNIT_STR	"KHz"

static jmp_buf env;

void usages(progname)
char *progname;
{
	printf("usages: %s [-l on|off] [-p on|off] [-f freq] [-m mode] [-c val|+val|-val|on|off]\n",progname);
	printf("\t [-v] [-s on|off] [-R reapter_mode] [-S sql_mode] [-t tone] \n");
	printf("\t [-d dcs_code] [-r rx|tx|freq] [-P on|off] [-j ref_band]\n");
	exit(0);
}

void p_error(progname,msg,p)
char *progname, *msg;
int p;
{
	char buf[BUFSIZ];
	if(p) {
		snprintf(buf,sizeof(char)*BUFSIZ,"%s (error): %s ",progname,msg);
		perror(buf);
	}
	else
		fprintf(stderr,"%s (error): %s\n",progname,msg);
	exit(1);
}

char char2bcd(x)
char x;
{
	char i,j,k;

	i = x/10;
	j = x - i*10; 
	k = (i << 4) & 0xf0;

	return(k+j);

}

int str2freq(str,unit)
char *str;
float unit;
{
	char *buf, *p;
	float freq;

	buf=(char *)malloc(sizeof(char)*BUFSIZ);
	p=buf;

	while(*str != 0x0 && 
		((*str > 0x2f && *str < 0x3a) || *str == '.' || *str == ',')) {
		if(*str != ',') *p++ = *str;
		str++;
	}
	*p = 0x0;
	
	if(*str == 'H' || *str == 'h') unit = HZ;
	else if(*str == 'K' || *str == 'k') unit = KHZ;
	else if(*str == 'M' || *str == 'm') unit = MHZ;

	sscanf(buf,"%f",&freq);
	freq *= unit;
	free(buf);
	return((int)freq);
}

char *bcd2freq_str(c)
unsigned char *c;
{
	double freq; 
	char *buf;

	buf = (char *)malloc(sizeof(char)*16);

	freq  = ((double)(c[0] & 0xf0) * 100000.0 / 16.0) + ((double)(c[0] & 0x0f) * 10000.00); 
	freq += ((double)(c[1] & 0xf0) *   1000.0 / 16.0) + ((double)(c[1] & 0x0f) *   100.00); 
	freq += ((double)(c[2] & 0xf0) *     10.0 / 16.0) + ((double)(c[2] & 0x0f) *     1.00); 
	freq += ((double)(c[3] & 0xf0) *      0.1 / 16.0) + ((double)(c[2] & 0x0f) *     0.01); 

	if(freq < 1900.0) 
		snprintf(buf,sizeof(char)*BUFSIZ,"%'fKHz",freq);
	else
		snprintf(buf,sizeof(char)*BUFSIZ,"%'fMKHz",freq/KHZ);

	return(buf);
	
}

char *str2tone(str)
char *str;
{
	char *buf, *p, i;
	float freq;
	int  n,m;
	char *bcd,*x;

	buf=(char *)malloc(sizeof(char)*BUFSIZ);
	bcd=(char *)malloc(sizeof(char)*2); 
	x=(char *)malloc(sizeof(char)*2); 

	p=buf;

	while(*str != 0x0 && 
		((*str > 0x2f && *str < 0x3a) || *str == '.' || *str == ',')) {
		if(*str != ',') *p++ = *str;
		str++;
	}
	*p = 0x0;

	sscanf(buf,"%f",&freq);
	free(buf);

	i=0, m=(int)(freq *10.0);
	while((int)tones[i]) {
		if((int)(tones[i]*10.0) == m) {
			x[0]=(char)(m/100);
			n = (int)x[0]*100;
			x[1]= (char)(m-n);
			bcd[0]=char2bcd(x[0]);
			bcd[1]=char2bcd(x[1]);
			return((char *)bcd);
		}
	}
	bcd[0]=-1;
	return((char *)bcd);
}

char *str2dcs(str)
char *str;
{
	char *p, i;
	int  n,m;
	char *bcd,*x;

	bcd=(char *)malloc(sizeof(char)*2); 
	x=(char *)malloc(sizeof(char)*2); 

	p=str;

	while(*p != 0x0) { 
		if(!isdigit(*p)) {
			bcd[0]=-1;
			return((char *)bcd);
		}
		p++;
	}

	sscanf(str,"%d",&m);

	i=0;
	while(dcs_codes[i]) {
		if(dcs_codes[i] == m) {
			x[0]=(char)(m/100);
			n = (int)x[0]*100;
			x[1]= (char)(m-n);
			bcd[0]=char2bcd(x[0]);
			bcd[1]=char2bcd(x[1]);
			return((char *)bcd);
		}
	}
	bcd[0]=-1;
	return((char *)bcd);
}

char str2mode(str)
char *str;
{
	int i;
	char *buf, *p;

	buf=(char *)malloc(sizeof(char)*BUFSIZ);
	p=buf;

	while(*str != 0x0) {
		*p++ = toupper(*str);
		str++;
	}
	*p = 0x0;

	i = 0;
	while(mode_type[i].cmd != (char)MODE_NONE) {
		if(strncmp(mode_type[i].type,buf,strlen(buf)) == 0) return(mode_type[i].cmd);
		i++;
	}
	return(MODE_NONE);
}

char *char2mode_str(c)
unsigned char c;
{
	int i;

	i=0;
	while(mode_type[i].cmd != (char)MODE_NONE) {
		if((unsigned char)mode_type[i].cmd == c) return(mode_type[i].type);
		i++;
	}
	return(mode_type[i].type);
}

char str2sql(str)
char *str;
{
	int i;
	char *buf, *p;

	buf=(char *)malloc(sizeof(char)*BUFSIZ);
	p=buf;

	while(*str != 0x0) {
		*p++ = toupper(*str);
		str++;
	}
	*p = 0x0;

	i = 0;
	while(sql_type[i].cmd != (char)CMD_ERROR) {
		if(strncmp(sql_type[i].type,buf,strlen(buf)) == 0) return(sql_type[i].cmd);
		i++;
	}
	return(CMD_ERROR);
}

char str2stat(str)
char *str;
{
	int i;
	char *buf, *p;

	buf=(char *)malloc(sizeof(char)*BUFSIZ);
	p=buf;

	while(*str != 0x0) {
		*p++ = toupper(*str);
		str++;
	}
	*p = 0x0;

	i = 0;
	while(stat_type[i].cmd != (char)CMD_ERROR) {
		if(strncmp(stat_type[i].type,buf,strlen(buf)) == 0) return(stat_type[i].cmd);
		i++;
	}
	return(CMD_ERROR);
}

char str2sw(str,on,off,err)
char *str;
char on,off,err;
{
	char i;
	char *buf, *p;

	buf=(char *)malloc(sizeof(char)*BUFSIZ);
	p=buf;

	while(*str != 0x0) {
		*p++ = toupper(*str);
		str++;
	}
	*p = 0x0;

	if(strncmp("ON",buf,strlen(buf)) == 0) return(on);
	if(strncmp("OFF",buf,strlen(buf)) == 0) return(off);
	return(err);
}

char *int2bcd2(i_val)
int	i_val;
{
	int n;
	char *bcd,*x;

	bcd=(char *)malloc(sizeof(char)*2); 
	x=(char *)malloc(sizeof(char)*2); 

	x[0]=(char)(i_val/100);
	n= (int)x[0]*100;
	x[1]= (char)(i_val-n);

	bcd[0]=char2bcd(x[0]);
	bcd[1]=char2bcd(x[1]);

	return((char *)bcd);
}

char *int2bcd4(i_val)
int	i_val;
{
	int n;
	char *bcd, *x;

	bcd=(char *)malloc(sizeof(char)*4); 
	x=(char *)malloc(sizeof(char)*4); 

	x[0]=(char)(i_val/1000000);
	n=(int)x[0]*1000000;
	x[1]=(char)((i_val-n)/10000);
	n+= (int)x[1]*10000;
	x[2]=(char)((i_val-n)/100);
	n+= (int)x[2]*100;
	x[3]= (char)(i_val-n);

	bcd[0]=char2bcd(x[0]);
	bcd[1]=char2bcd(x[1]);
	bcd[2]=char2bcd(x[2]);
	bcd[3]=char2bcd(x[3]);

	return((char *)bcd);
}

int get_bp_mode(freq)
int freq;
{
	int i;

	i=0;
	while(band_plan[i].start) {
		if(freq < band_plan[i].start)  return(DEF_MODE);
		if(freq >= band_plan[i].start && freq < band_plan[i].end)
			return(band_plan[i].mode);
		i++;
	}
}

char get_ibp(str)
char *str;
{
	char i;

	i=0;
	while(ibp[i].freq) {
		if(strncmp(ibp[i].band,str,strlen(ibp[i].band)) == 0) return(i);
		i++;
	}
	return(-1);
}

int wait_do_cmd(fd)
int fd;
{
	unsigned char c;

#if 1
	while(read(fd,(char *)&c,1)<0);;
#else
	do {
		read(fd,(char *)&c,1);
	} while(c != 0x00);
#endif

	return(1);
}

unsigned char read_char(fd)
int fd;
{
	unsigned char c;
	int i;

#if 1
	while(read(fd,(char *)&c,1)<0);;
#else
	do {
		i=read(fd,(char *)&c,1);
	} while(i<0);
#endif

	return(c);
}

void cmd_write(fd,p1,p2,p3,p4,cmd)
int fd;
unsigned char p1,p2,p3,p4,cmd;
{
	char buf[BUFSIZ];

	snprintf(buf,sizeof(char)*BUFSIZ,"%c%c%c%c%c",p1,p2,p3,p4,cmd);
	write(fd,buf,5);
	wait_do_cmd(fd);
	tcflush(fd,TCIOFLUSH);
	return;
}

void read_rx_stat(fd)
int fd;
{
	char buf[BUFSIZ],val;
	unsigned char c;
	int i;

	snprintf(buf,sizeof(char)*BUFSIZ,"%c%c%c%c%c",PAD,PAD,PAD,PAD,CMD_RX_STAT);
	write(fd,buf,5);
	c=read_char(fd);
	fprintf(stdout,"\n");
	fprintf(stdout,"RX Status\n");
	for(i=5;i<8;i++) {
		val = 0x01 & (c >> i);
		fprintf(stdout,"  %24s : %s\n",rx_stat_str[i-5].name,rx_stat_str[i-5].stat[val]);
	} 
	fprintf(stdout,"  %16s : %d\n","S Mater",(int)(0x0f & c));
	fprintf(stdout,"\n");
	tcflush(fd,TCIOFLUSH);
	return;
}

void read_tx_stat(fd)
int fd;
{
	char buf[BUFSIZ],val;
	unsigned char c;
	int i;

	snprintf(buf,sizeof(char)*BUFSIZ,"%c%c%c%c%c",PAD,PAD,PAD,PAD,CMD_TX_STAT);
	write(fd,buf,5);
	c=read_char(fd);
	fprintf(stdout,"\n");
	fprintf(stdout,"TX Status\n");
	for(i=5;i<8;i++) {
		val = 0x01 & (c >> i);
		fprintf(stdout,"  %16s : %s\n",tx_stat_str[i-5].name,tx_stat_str[i-5].stat[val]);
	} 
	fprintf(stdout,"  %16s : %d\n","PO Mater",(int)(0x0f & c));
	fprintf(stdout,"\n");
	tcflush(fd,TCIOFLUSH);
	tcflush(fd,TCIOFLUSH);
	return;
}

void read_fr_stat(fd)
int fd;
{
	char buf[BUFSIZ];
	unsigned char c[5];
	int i;

	snprintf(buf,sizeof(char)*BUFSIZ,"%c%c%c%c%c",PAD,PAD,PAD,PAD,CMD_FR_STAT);
	write(fd,buf,5);
	for(i=0;i<5;i++)
		c[i]=read_char(fd);
	fprintf(stdout,"Frequency & Mode Status\n");
	fprintf(stdout,"  Frequency : %s\n",bcd2freq_str(c));
	fprintf(stdout,"  Mode      : %s\n",char2mode_str(c[4]));
	tcflush(fd,TCIOFLUSH);
	return;
}

void static system_timeout(sig)
int sig;
{
	siglongjmp(env, 1);
}

int main(argc, argv)
int argc;
char **argv;
{
	int i, j, k, n, fd;
	char c;
	char lock_flag, lock;
	char ptt_flag, ptt;
	char freq_flag, *freq;
	char mode_flag, mode;
	char clar_flag, *clar, clar_sig, clar_sw;
	char vfo_flag; 
	char split_flag, split; 
	char rep_flag, *rep, rep_sig;
	char sql_flag, sql;
	char tone_flag, *tone;
	char dcs_flag, *dcs;
	char rx_stat_flag, tx_stat_flag, fr_stat_flag; 
	char power_flag, power;
	char ibp_n_flag, ibp_n;
	int status, exit_code, fr;
	static pid_t pid;
	char buf[BUFSIZ];
	struct termios term, term_def;
	extern char *optarg;

	if(argc < 2) usages(argv[0]);
	
	lock_flag=0, ptt_flag=0, freq_flag=0, mode_flag=0, clar_flag=0; 
	vfo_flag=0, split_flag=0, rep_flag=0, sql_flag=0, tone_flag=0, dcs_flag=0;
	rx_stat_flag=0, tx_stat_flag=0, fr_stat_flag=0; 
	power_flag=0, ibp_n_flag=0;

	while ((i = getopt(argc, argv, "l:p:f:m:c:vs:R:S:t:d:r:P:j:")) != -1) {
		switch (i) {
			case 'l':
				lock_flag=1;
				lock=str2sw(optarg,CMD_LOCK_ON,CMD_LOCK_OFF,CMD_ERROR);
				if(lock==(char)CMD_ERROR)
					p_error(argv[0],"Lock option value either 'on' or 'off'",0);
				break;
			case 'p':
				ptt_flag=1;
				ptt=str2sw(optarg,CMD_PTT_ON,CMD_PTT_OFF,CMD_ERROR);
				if(ptt==(char)CMD_ERROR)
					p_error(argv[0],"PTT option value either 'on' or 'off'",0);
				break;
			case 'f':
				freq_flag=1;
				fr = str2freq(optarg,KHZ);
				k=0, n=1;
				while(frange[k].fmin != 0) {
					if(fr >= frange[k].fmin && fr <= frange[k].fmax) {
						n=0;
						if(frange[k].mode_limit == (char)FM_ONLY) {
							mode_flag=1;
							mode = MODE_FM;
						}
					}
					k++;
				}
				if(n)
					p_error(argv[0],"Frequency out of band",0);
				freq=int2bcd4(fr/10);
				break;
			case 'm':
				mode_flag=1;
				mode = str2mode(optarg);
				if(mode == (char)MODE_NONE)
					p_error(argv[0],"illgale mode",0);
				break;
			case 'c':
				clar_flag=1;
				if(*optarg='+') {
					clar_sig=PAR_P_OFFSET;
					clar_sw=CMD_CLAR_ON;
					optarg++;
					j = str2freq(optarg,KHZ);
				}
				else if(*optarg='-') {
					clar_sig=PAR_N_OFFSET;
					clar_sw=CMD_CLAR_ON;
					optarg++;
					j = str2freq(optarg,KHZ);
				}
				else if(*optarg >= '0' && *optarg <= '9') {
					clar_sig=PAR_P_OFFSET;
					clar_sw=CMD_CLAR_ON;
					j = str2freq(optarg,KHZ);
				}
				else { 
					clar_sw=str2sw(optarg,CMD_CLAR_ON,CMD_CLAR_OFF,CMD_ERROR);
					if(clar_sw==(char)CMD_ERROR)
						p_error(argv[0],"CLAR option value either CLAR freq or 'on' or 'off'",0);
					clar_sig=PAR_P_OFFSET;
					j = 0;
				}
				if(j > MAX_CLAR)
					p_error(argv[0],"Clar freq out of renge",0);
				clar=int2bcd2(j);
				break;
			case 'v':
				vfo_flag=1;
				break;
			case 's':
				split_flag=1;
				split=str2sw(optarg,CMD_SPLIT_ON,CMD_SPLIT_OFF,CMD_ERROR);
				if(split==(char)CMD_ERROR)
					p_error(argv[0],"Split option either 'on' or 'off'",0);
				break;
			case 'R':
				rep_flag=1;
				if(*optarg='+') {
					rep_sig=PAR_P_SHIFT;
					optarg++;
					j = str2freq(optarg,MHZ);
				}
				else if(*optarg='-') {
					rep_sig=PAR_N_SHIFT;
					optarg++;
					j = str2freq(optarg,MHZ);
				}
				else if(*optarg >= '0' && *optarg <= '9') {
					rep_sig=PAR_P_SHIFT;
					j = str2freq(optarg,MHZ);
				}
				else if(*optarg='s') {
					rep_sig=PAR_SIMPLEX;
					j = 0;
				}
				else 
					p_error(argv[0],"Reapter option value either shift freq or 's' for simplex",0);
				if(j > MAX_REP_SHIFT)
					p_error(argv[0],"reapter shift freq out of range",0);
				rep=int2bcd4(j);
				break;
			case 'S':
				sql_flag=1;
				sql=str2sql(optarg);
				if(sql==(char)CMD_ERROR)
					p_error(argv[0],"SQL option value either 'dcs','ctcss','encoder' or 'off'",0);
				break;
			case 't':
				tone_flag=1;
				tone=str2tone(optarg);
				if(tone[0]<0)
					p_error(argv[0],"illgale tone freq",0);
				break;
			case 'd':
				dcs_flag=1;
				dcs=str2dcs(optarg);
				if(dcs[0]<0)
					p_error(argv[0],"illgale DCS code",0);
				break;
			case 'r':
				c=str2stat(optarg);
				if(c==(char)CMD_RX_STAT) rx_stat_flag=1;
				else if(c==(char)CMD_TX_STAT) tx_stat_flag=1;
				else if(c==(char)CMD_FR_STAT) fr_stat_flag=1;
				else
					p_error(argv[0],"Read status option either 'rx','tx', or 'freq'",0);
				break;
			case 'P':
				power_flag=1;
				power=str2sw(optarg,CMD_POWER_ON,CMD_POWER_OFF,CMD_ERROR);
				if(power==(char)CMD_ERROR)
					p_error(argv[0],"Power option either 'on' or 'off'",0);
				break;
			case 'j':
				ibp_n_flag=1;
				ibp_n = get_ibp(optarg); 
				if(ibp_n < 0)
					p_error(argv[0],"No beacon station in band",0);
				break;
			case 'h':
			case '?':
			default:
				usages(argv[0]);
		}
	}


	if(ibp_n_flag) {
		freq_flag=1;
		mode_flag=1;
		freq=int2bcd4(ibp[ibp_n].freq/10);
		mode = ibp[ibp_n].mode;
	}

	if(freq_flag && !mode_flag) {
		mode_flag=1;
		mode = get_bp_mode(fr);
	}

	if(sigsetjmp(env, 1)) {
		alarm(0);
		signal(SIGALRM, SIG_DFL);
		if(pid > 0)
			kill(pid, SIGKILL);
		p_error(argv[0],"CAT access timeout",0);
	}

	pid=fork();
	switch(pid) {
		case -1:
			p_error(argv[0],buf,1);
			break;
		case 0:
			if((fd = open(TTY_DEV, O_RDWR | O_NONBLOCK)) < 0)
				p_error(argv[0],TTY_DEV,1);

			tcgetattr(fd,&term_def);

			term.c_iflag = IGNBRK | IGNPAR | IMAXBEL;
			term.c_oflag = 0;
			term.c_cflag = CS8 | CSTOPB | CREAD | CLOCAL;
			term.c_lflag = 0;
			term.c_cc[VMIN] = 1;
			term.c_cc[VTIME] = 0;

			cfsetispeed(&term,B_FT817);
			cfsetospeed(&term,B_FT817);
			tcflush(fd,TCIOFLUSH);
			tcsetattr(fd, TCSANOW, &term);

			cmd_write(fd,PAD,PAD,PAD,PAD,PAD);

			if(power_flag) 
				cmd_write(fd,PAD,PAD,PAD,PAD,power);

			if(freq_flag)
				cmd_write(fd,freq[0],freq[1],freq[2],freq[3],CMD_SET_FREQ);

			if(mode_flag)
				cmd_write(fd,mode,PAD,PAD,PAD,CMD_SET_MODE);

			if(lock_flag)
				cmd_write(fd,PAD,PAD,PAD,PAD,lock);

			if(ptt_flag)
				cmd_write(fd,PAD,PAD,PAD,PAD,ptt);

			if(clar_flag) {
				cmd_write(fd,PAD,PAD,PAD,PAD,clar_sw);
				cmd_write(fd,clar_sig,PAD,clar[0],clar[1],CMD_SET_CLAR);
			}

			if(vfo_flag)
				cmd_write(fd,PAD,PAD,PAD,PAD,CMD_TOGLE_VFO);

			if(split_flag)
				cmd_write(fd,PAD,PAD,PAD,PAD,split);

			if(rep_flag) {
				cmd_write(fd,rep_sig,PAD,PAD,PAD,CMD_SET_REP);
				cmd_write(fd,rep[0],rep[1],rep[2],rep[3],CMD_SET_SHIFT);
			}

			if(sql_flag)
				cmd_write(fd,sql,PAD,PAD,PAD,CMD_SET_SQL);

			if(tone_flag)
				cmd_write(fd,tone[0],tone[1],PAD,PAD,CMD_SET_TONE);

			if(dcs_flag)
				cmd_write(fd,dcs[0],dcs[1],PAD,PAD,CMD_SET_DCS);

			if(rx_stat_flag)
				read_rx_stat(fd);

			if(tx_stat_flag)
				read_tx_stat(fd);

			if(fr_stat_flag)
				read_fr_stat(fd);

			tcsetattr(fd, TCSANOW, &term_def);
			close(fd);
			_exit(0);
			break;
		default:
			alarm(TIMEOUT);
			signal(SIGALRM, system_timeout);
			pid = waitpid(pid, &status, WUNTRACED);
			exit_code = WEXITSTATUS(status);
	}
	alarm(0);
	signal(SIGALRM, SIG_DFL);
	exit(exit_code);
}
