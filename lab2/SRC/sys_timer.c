#include "aduc812.h"
#include "sys_timer.h"

unsigned long __systime = 0;

//////////////////////// SetVector //////////////////////////
// Функция, устанавливающая вектор прерывания в пользовательской таблице
// прерываний.
// Вход: Vector - адрес обработчика прерывания,
// Address - вектор пользовательской таблицы прерываний.
// Выход: нет.
// Результат: нет.
//////////////////////////////////////////////////////////////

void SetVector(unsigned char __xdata * Address, void * Vector)
{
	unsigned char __xdata * TmpVector; // Временная переменная
	// Первым байтом по указанному адресу записывается
	// код команды передачи управления ljmp, равный 02h
	*Address = 0x02;
	// Далее записывается адрес перехода Vector
	TmpVector = (unsigned char __xdata *) (Address + 1);
	*TmpVector = (unsigned char) ((unsigned short)Vector >> 8);
	++TmpVector;
	*TmpVector = (unsigned char) Vector;
	// Таким образом, по адресу Address теперь
	// располагается инструкция ljmp Vector
}

//////////////////////// T0_ISR //////////////////////////////
// Обработчик прерывания от таймера 0.
//////////////////////////////////////////////////////////////
void T0_ISR( void ) __interrupt ( 1 )
{
	// Время в милисекундах
	__systime++;
	TH0 = 0xFC; // Инициализация таймера 0:
	TL0 = 0x67; // настройка на частоту работы 1000 Гц (чуть больше)
}

///////////////////////////////////////////////////////////////////////////
// Инициализация Таймера 0 (1000Гц)
///////////////////////////////////////////////////////////////////////////
void InitSystimer0( void )
{
	TCON = TCON | 0x00; 	// Выключение таймера 0 (и таймера 1)
	TMOD = TMOD | 0x01; 	// Выбор режима работы 16-разрядный таймер
	TH0 = 0xFC; 	// Инициализация таймера 0:
	TL0 = 0x67; 	// настройка на частоту работы 1000 Гц (чуть больше)
	TCON = TCON | 0x10; 	// Включение таймера 0

	SetVector(0x200B, (void *)T0_ISR );
}
///////////////////////////////////////////////////////////////////////////
// Чтение милисекундного счетчика
///////////////////////////////////////////////////////////////////////////
unsigned long GetMsCounter( void )
{
	unsigned long res;
	ET0 = 0;
	res = __systime;
	ET0 = 1;
	return res;
}
///////////////////////////////////////////////////////////////////////////
// Возвращает прошедшее время (от момента замера)
///////////////////////////////////////////////////////////////////////////
unsigned long DTimeMs( unsigned long t2 )
{
	unsigned long t1 = ( unsigned long )GetMsCounter();
	return t1 - t2;
}
///////////////////////////////////////////////////////////////////////////
// Задержка в милисекундах
///////////////////////////////////////////////////////////////////////////
void DelayMs( unsigned long ms )
{
	unsigned long t1 = ( unsigned long )GetMsCounter();
	while ( 1 )
	{
		if ( DTimeMs( t1 ) > ms ) break;
	}
}


