/****************************************************************************

    led.c - простейший драйвер светодиодов
            для учебного стенда SDK-1.1

    (C) led.c, Ключев А.О.  2007 г.

Это свободная программа; вы можете повторно распространять ее и/или
модифицировать ее в соответствии с Универсальной Общественной
Лицензией GNU, опубликованной Фондом Свободного ПО; либо версии 2,
либо (по вашему выбору) любой более поздней версии.

Эта программа распространяется в надежде, что она будет полезной,
но БЕЗ КАКИХ-ЛИБО ГАРАНТИЙ; даже без подразумеваемых гарантий
КОММЕРЧЕСКОЙ ЦЕННОСТИ или ПРИГОДНОСТИ ДЛЯ КОНКРЕТНОЙ ЦЕЛИ.  Для
получения подробных сведений смотрите Универсальную Общественную
Лицензию GNU.

Вы должны были получить копию Универсальной Общественной Лицензии
GNU вместе с этой программой; если нет, напишите по адресу: Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
02111-1307 USA

----------------------------------------------------------------------------
Россия, Санкт-Петербург, кафедра вычислительной техники СПбГУИТМО 
e-mail: kluchev@d1.ifmo.ru

****************************************************************************/
#include "aduc812.h"
#include "max.h"

static unsigned char old_led = 0;   // "Видеопамять" линейки светодиодов


/**----------------------------------------------------------------------------
                            led
-------------------------------------------------------------------------------
Управление одним светодиодом

Вход:       n  - порядковый номер светодиода ( от 0 до 7 )
            on - 1 - зажигает, 0 гасит светодиод
Выход:      нет
Результат:  нет
Описание:   Производится доступ к регистру расширителя портов SV с помощью
            функции write_max. Состояние светодиодов хранится в регистре old_led.
Пример:
-----------------------------------------------------------------------------*/

void led( unsigned char n, unsigned char on )
{
unsigned char c;
unsigned char mask = 1;

    if( n > 7 ) return;

    c = old_led;

    mask <<= n;

    if( on )
        c |= mask;
    else
        c &= ~mask;         

    write_max( (unsigned char __xdata *)SV, c );     

    old_led = c;
}


/**----------------------------------------------------------------------------
                            leds
-------------------------------------------------------------------------------
Зажигание линейки светодиодов

Вход:       on - управление светодиодами. Каждый бит переменной отвечает за один 
            светодиод: 1 - зажигает, 0 гасит светодиод
Выход:      нет
Результат:  нет
Описание:   Производится доступ к регистру расширителя портов SV с помощью
            функции write_max. Состояние светодиодов хранится в регистре old_led.
Пример:
-----------------------------------------------------------------------------*/

void leds( unsigned char on )
{
    write_max( (unsigned char __xdata *)SV, on );     

    old_led = on;
}
