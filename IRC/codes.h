#ifndef __CODES_H__
#define __CODES_H__

/* Server commands */
#define CMDERR 1
#define JOIN 2
#define PRIVMSG 3
#define PING 4

/* Client commands /cmd -> SCMD */
#define SQUIT -1
#define SCMDERR 1
#define SNOTCMD 2
#define SMSG 3
#define SJOIN 4
#define SLEAVE 5

#endif
