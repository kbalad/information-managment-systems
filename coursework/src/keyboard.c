/******************************************************************************
                (С) ООО "ЛМТ", Санкт-Петербург, Россия, 2002.
                http://lmt.cs.ifmo.ru, tel.: +7-812-233-3096,
                mailto: lmt@d1.ifmo.ru

        Данный файл является  свободно  распространяемым  примером  на
    языке  Си  (компилятор  Keil  C  фирмы  KEIL  ELEKTRONIK  GmbH)  и
    предназначен для  иллюстрации работы с клавиатурой стенда SDK-1.1.
        Данный  программный   модуль   не    подвергался   тщательному
    тестированию  и  может   содержать   ошибки.   Авторы   не   несут
    ответственности за потерю  информации  и  иные  возможные  вредные
    последствия использования данного программного  модуля  в  составе
    программных проектов.
        Данный файл может подвергаться любым изменениям, но  не  может
    распространяться в коммерческих целях  и  в  составе  коммерческих
    программных продуктов.


Файл:       kb.c
Версия:     1.0.0
Автор:      LAN
Описание:   Набор демонстрационного программного обеспечения для стенда
            SDK-1.1. Работа с клавиатурой. Данный модуль использует
            модуль работы с функциональным расширителем центрального
            процессора стенда на базе ПЛИС Altera MAX3064 (3128)
            (файлы max.c, max.h).

Изменения:
-------------------------------------------------------------------------------
N Дата     Версия   Автор               Описание
-------------------------------------------------------------------------------
1 10.04.02  1.0.0   LAN     Создан
******************************************************************************/
#include "max.h"
#include "keyboard.h"
#include "aduc812.h"

/*----------------------------------------------------------------------------
            Переменные и флаги
 -----------------------------------------------------------------------------*/

char KBTable[]="147*2580369#ABCD"; //Таблица символов, соответствующих клавишам
                                   //клавиатуры SDK-1.
								   
unsigned char key_flags[16];
	
unsigned char keyboard_buff[KB_BUFFER_SIZE]; // Буффер, в который записываются
unsigned char keyboard_len;


static void SetVector(unsigned char __xdata * Address, void * Vector)  {
	unsigned char __xdata * TmpVector; 
	*Address = 0x02;
	TmpVector = (unsigned char __xdata *) (Address + 1);
	*TmpVector = (unsigned char) ((unsigned short)Vector >> 8);
	++TmpVector;
	*TmpVector = (unsigned char) Vector;
}

/**----------------------------------------------------------------------------
                        init_keyborad()
-------------------------------------------------------------------------------
Инициализация  буфера и регистров, назначение обработчика прерывния клавиатуры.

Вход:   указатель на функцию обработчик прерывания.
Выход:  нет
Результат: нет
----------------------------------------------------------------------------- */
void init_keyboard(void* handler)
{
    int i;
	unsigned char old_ena;
	
    keyboard_len = 0;
    for (i = 0; i < KB_BUFFER_SIZE; i++)
        keyboard_buff[i] = 0;
	
	old_ena = read_max((unsigned char __xdata *)ENA);
	write_max((unsigned char __xdata *)ENA, old_ena | 0x40); // Разрешить прерывание от клавиатуры (6-й бит регистра ENA должен быть <1>);
	
	// Проверить следующую команду! Возможно, необходимо использовать "1", в учебнике четко не сказано.
	IT0 = 0; // Настроить внешнее прерывание INT0 так, чтобы оно работало по спаду, а не по уровню (регистр специального назначения TCON);
			 // по спаду - 1, по уровню - 0
	EX0 = 1; // Разрешить внешнее прерывание INT0 (бит EX0 = 1 в регистре специального назначения IE);
	
	old_ena = read_max((unsigned char __xdata *)ENA);
	write_max((unsigned char __xdata *)ENA, old_ena | 0x20); // Переключатели SW3 должны быть в положении OFF, а 5-й бит регистра ENA (ПЛИС) должен быть <1>;
	
	SetVector(IE0_VECTOR_N, (void *)handler);
	
	//
	for (i = 0; i < 16; ++i)
		key_flags[i] = 0;

    EA = 1;
}

/**----------------------------------------------------------------------------
                        read_keyboard()
-------------------------------------------------------------------------------
API-функция чтения символа из буфера клавиатуры

Вход:   переменная, в которую должен быть записан считанный символ из буфера
Выход:  1, если символ считан
        0, если буфер пуст
-----------------------------------------------------------------------------*/
char read_keyboard(unsigned char* symbol) {
    int i;

    EX0 = 1;
	if ( 0 < keyboard_len )
	{
		*symbol = keyboard_buff[0];
		--keyboard_len;

		for (i = 0; i < keyboard_len; i++)
			keyboard_buff[i] = keyboard_buff[i + 1];

		return 1;
	}

	return 0;
}

/**----------------------------------------------------------------------------
                        ScanKBOnce()
-------------------------------------------------------------------------------
Единичное сканирование клавиатуры SDK-1.1. Если нажата клавиша, то
соответствующий ей символ (см. таблицу KBTable[]) помещается в буфер

Вход:       нет
Результат:  0 - ни одна клавиша не была нажата при сканировании;
            1 - зарегистрировано нажатие.
----------------------------------------------------------------------------- */
void ScanKBOnce(void) __interrupt( 0 )
{
    unsigned char row,col,rownum,colnum;
    unsigned int i;
	unsigned char fl = 0;
	
    //Сканирование производится по "столбцам" клавиатуры, на которые подается
    //"бегущий 0".
    //
    // Считывание происходит следующим образом:
    //      Мы будем пробегаться по каждой клетке/клавише
    //      (проверяем каждую "горизонталь" в каждой вертикали/столбце )
    //      и проверять, замкнута ли она. Если да, то ее
    //      будет отображено в регистре KB (см.док-ию).
	EX0 = 0;
    for(colnum = 0; colnum < 4; colnum++)
    {
        col = 0x1 << colnum; //0001,0010,0100,1000,0001,...

        write_max((unsigned char __xdata *)KB, ~col); //11111110,11111101,11111011,11110111,11111110,...

        //При подаче нуля на очередной столбец на каждом из "рядов" проверяется
        //наличие нуля (факт замыкания контакта клавишей)
        for(rownum = 0; rownum < 4; rownum++)
        {
            row = read_max(KB) & (0x10 << rownum);
            if( !row ) //Обнаружено нажатие клавиши:
            {
                for(i = 0; i<5000; i++)continue;//проверка на дребезг контакта:
                       //через примерно 40мс повтор сканирования той же клавиши

                row = read_max(KB) & (0x10 << rownum);
                if( !row )
                {
					fl = 1;

					if (key_flags[(colnum<<2) + rownum] == 0)
					{
						key_flags[(colnum<<2) + rownum] = 1;

						keyboard_buff[keyboard_len % KB_BUFFER_SIZE] = KBTable[i];
						keyboard_len++;
					}
                }
				else{
					key_flags[(colnum<<2) + rownum] = 0;
				}
            }
        }

		if (fl == 0)
		{
			key_flags[(colnum<<2) + 0] = 0;
			key_flags[(colnum<<2) + 1] = 0;
			key_flags[(colnum<<2) + 2] = 0;
			key_flags[(colnum<<2) + 3] = 0;
		}
    }
}
