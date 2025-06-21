
#define VERSION "1.0"
#define MSG_220 "220 srvFtp version " VERSION "\r\n"
#define MSG_331 "331 Password required for %s\r\n"
#define MSG_230 "230 User %s logged in\r\n"
#define MSG_530 "530 Login incorrect\r\n"
#define MSG_221 "221 Goodbye\r\n"
#define MSG_550 "550 %s: no such file or directory\r\n"
#define MSG_299 "299 File %s size %ld bytes\r\n"
#define MSG_226 "226 Transfer complete\r\n"
#define MSG_502 "502 Command not implemented\r\n"
#define MSG_215 "215 UNIX Type: L8\r\n"
#define MSG_211 "211-Features:\r\n UTF8\r\n REST STREAM\r\n MDTM\r\n211 End\r\n"

//  SYST 215 / 500: error de sintaxis, 501: sintaxis de parametros incorrecta / 421