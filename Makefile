PROG=	ft817
SRCS=	ft817.c
BINDIR=	/usr/local/bin
MKMAN=	no
CLEANFILES=	ft817.core

# tty device
CFLAGS+=	-DTTY_DEV=\"/dev/ttyU2\"
# baud rate
CFLAGS+=	-DB_FT817=B4800

.include <bsd.prog.mk>
