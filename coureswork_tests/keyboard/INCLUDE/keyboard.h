#ifndef _KB_H
#define _KB_H

#define KB_BUFFER_SIZE 16

char ScanKBOnce(void);
void init_keyborad(void);
char read_keyboard(unsigned char* symbol);

#endif //_KB_H
