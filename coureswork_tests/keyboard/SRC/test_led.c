/****************************************************************************

    test_led.c - ���� �������� ����������������� ������
                 ��� �������� ������ SDK-1.1

    (C) test_led.c, ������ �.�.  2007 �.

��� ��������� ���������; �� ������ �������� �������������� �� �/���
�������������� �� � ������������ � ������������� ������������
��������� GNU, �������������� ������ ���������� ��; ���� ������ 2,
���� (�� ������ ������) ����� ����� ������� ������.

��� ��������� ���������������� � �������, ��� ��� ����� ��������,
�� ��� �����-���� ��������; ���� ��� ��������������� ��������
������������ �������� ��� ����������� ��� ���������� ����.  ���
��������� ��������� �������� �������� ������������� ������������
�������� GNU.

�� ������ ���� �������� ����� ������������� ������������ ��������
GNU ������ � ���� ����������; ���� ���, �������� �� ������: Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA

----------------------------------------------------------------------------
������, �����-���������, ������� �������������� ������� ���������
e-mail: kluchev@d1.ifmo.ru

****************************************************************************/
#include "aduc812.h"
#include "sio.h"
#include "keyboard.h"

// �������� �� �������� ���������� ��
void delay ( unsigned long ms )
{
    volatile unsigned long i, j;

    for( j = 0; j < ms; j++ )
    {
        for( i = 0; i < 50; i++ );
    }
}

// to test
void print_keyboard_buffer_to_sio(void)
{
    int i;

    for (i = 0; i < KB_BUFFER_SIZE; i++)
        WSio(keyboard_buff[i]);
}

// to test
void endless_dots_print(void)
{
    while (1)
    {
        WSio('.');
        delay(500);
    }
}

void main( void )
{
	InitSIO(S9600, 0);
    init_keyboard((void *)ScanKBOnce);
    
	EA = 1; // ��������� ����������
	
	Type("Hello!\r\n");
	Type("\r\nReading keyboard:\r\n");
	
	Type("test\n");
	
	
	while (1) {
		unsigned char ch;
		EA = 0;
		while (read_keyboard(&ch) > 0) WSio(ch);
		EA = 1;
		//delay(1000);
		
	}
}
