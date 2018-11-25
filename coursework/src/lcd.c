#include "lcd.h"
#include "max.h"

static bit curr_position_ctrl = 1;   	// Отслеживание драйвером текущей позиции курсора
                            			// ЖКИ (1 - вкл., 0 - выкл.)

// Текущая позиция курсора. Имеют смысл, только если curr_position_ctrl = 1.
// Переменные используются функциями put_char_lcd(), goto_xy()
static char cur_x = 0;	// 0-15
static char cur_y = 0; 	// 0/1

/**----------------------------------------------------------------------------
                        switch_cur_position_control()
-------------------------------------------------------------------------------
Включение/выключение отслеживания текущей позиции курсора. После включения
контроля желательно установить текущую позицию заново (goto_xy()), т.к. 
значение переменных cur_x и cur_y могут содержать неправильные значения.

Вход:       bit o: o = 1 - включить контроль
                   o = 0 - выключить контроль
Выход:      нет
Результат:  нет
----------------------------------------------------------------------------- */
void switch_cur_position_control(bit o) {
    curr_position_ctrl = o;
}

/**----------------------------------------------------------------------------
                        strobe()
-------------------------------------------------------------------------------
Подача сигнала E интерфейса ПЛИС<->ЖКИ на время не менее 500 нс. Одновременно
подаются сигналы R/W и RS в заданном аргументом c значении.

Вход:       char c - биты 1,2 определяют значения сигналов R/W и RS:
                бит 1: 1 - R/W = 1 (чтение)
                       0 - R/W = 0 (запись)
                бит 2: 1 - RS = 1 (выбрать регистр данных ЖКИ)
                       0 - RS = 0 (выбрать регистр команд ЖКИ)
                бит 3: должен быть равным 1!
                Значения остальных бит не принимаются во внимание.

Выход:      нет
Результат:  нет
----------------------------------------------------------------------------- */

void strobe(char c) {
	unsigned int i;

    write_max(C_IND, c | 0x1);  //Установка строба E
    write_max(C_IND, c & 0xFE); //Сброс строба

    for (i = 0; i < 300; i++) 
    	continue; 				//Задержка на время исполнения команды (>1.59ms)    
}

/**----------------------------------------------------------------------------
                        switch_cursor()
-------------------------------------------------------------------------------
Установка режима курсора: включение/выключение курсора, вкл./выкл. мигания

Вход:       bit cursor: 1 - включить курсор,
                        0 - выключить курсор;
            bit blink:  1 - включить мигание,
                        0 - выключить мигание.

Выход:      нет
Результат:  нет
----------------------------------------------------------------------------- */
void switch_cursor (bit cursor, bit blink) {
	unsigned char i = 0;

    write_max(DATA_IND, DISPLAY_CTRL |
    					DISPLAY_ON | 
    					((cursor) ? CURSOR_ON : 0) |
                        ((blink) ? BLINK : 0));

    strobe(0x8); 		// R/W = 0; RS = 0
}


/**----------------------------------------------------------------------------
                        clear_lcd()
-------------------------------------------------------------------------------
Очистка ЖКИ и установка текущей позиции на первый символ первой строки

Вход:       нет
Выход:      нет
Результат:  нет
----------------------------------------------------------------------------- */
void clear_lcd(void) {
    write_max(DATA_IND, CLEAR);
    strobe(0x8); 		// R/W = 0; RS = 0
    cur_x = 0;
    cur_y = 0;
}


/**----------------------------------------------------------------------------
                        goto_xy()
-------------------------------------------------------------------------------
Установка текущей позиции ЖКИ. Сохранение ее во внутренних переменных драйвера.

Вход:       uchar x - номер столбца (позиции в строке): 0..15.
            bit y - номер строки: 0..1.
Выход:      нет
Результат:  нет
----------------------------------------------------------------------------- */
void goto_xy(unsigned char x, bit y) {
    write_max(DATA_IND, RAM_DD | (x + ((y) ? 0x40 : 0))); //установка адреса DDRAM в счетчик адреса.
    strobe(0x8);    
    cur_x = x;
    cur_y = y;
}



/**----------------------------------------------------------------------------
                        print_char_lcd()
-------------------------------------------------------------------------------
Вывод на текущую позицию символа. Текущая позиция определяется в зависимости
от значения флага curr_position_ctrl: если он установлен, то по переменным cur_x,
cur_y (при этом она увеличивается с каждым выведенным символом);
если он сброшен, то по внутреннему указателю ЖКИ (не увеличивается по выводу
символа).

Вход:       char ch - выводимый символ
Выход:      нет
Результат:  нет
----------------------------------------------------------------------------- */
void print_char_lcd(char ch) {
    if (curr_position_ctrl) {
        goto_xy(cur_x, cur_y);
        cur_x += 15;
        if (cur_x > 15) {	//переход на следующую строку 
        	cur_x = 0;
        	cur_y = (cur_y == 0) ? 1 : 0;
        }
    }

    write_max(DATA_IND, ch);
    strobe(0xC);	//R/W = 0, RS = 1 (данные)
}


/**----------------------------------------------------------------------------
                        print_string_lcd()
-------------------------------------------------------------------------------
Вывод ASCIIZ-строки на ЖКИ начиная с текущей позиции.

Вход:       char *s - указатель на строку.
			unsigned int n - количество символов в строке
Выход:      нет
Результат:  нет
----------------------------------------------------------------------------- */
void print_string_lcd (char* s, unsigned int n) {
    unsigned short i;
	bit t = curr_position_ctrl;
    switch_cur_position_control(1);	// включаем отслеживание текущей позиции курсора
    for (i = 0; i < n; i++) 
    	print_char_lcd(s[i]);
  
    switch_cur_position_control(t);
}
