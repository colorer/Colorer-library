#ifndef SAN_TIMER
#define SAN_TIMER

/*
	Copyright 2008 Sukhinov Anton
	E-mail: Soukhinov@gmail.com

	Version 1.0

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <cassert>
#include <windows.h>

class CFrequency; //Forward-определение класса CFrequency

class CTimer
{ //Класс CTimer имитирует некий счётчик времени, который можно запустить и остановить
public:
	typedef long long tTime;

private:
	static CFrequency frequency; //Частота таймера для перевода tTime в секунды
	
	bool state; //Если true, то таймер запущен. Иначе -- нет
	
	tTime timeTotal;    //Сколько времени набежало за все предыдущие интервалы
	tTime timeLastStart; //Момент начала последнего, ещё незавершённого интервала (если запущен)
	
	static inline tTime Now(void); //Узнать текущее значение времени в "тиках"

public:
	inline CTimer(bool start = false); //Конструктор по-умолчанию
	inline CTimer(CTimer const &T);    //Конструктор копии
	inline ~CTimer(void);              //Деструктор
	inline CTimer &operator=(CTimer const &T); //Оператор присваивания

	inline double Get(void) const; //Узнать текущее значение таймера в секундах
	inline tTime GetTickCount(void) const; //Узнать текущее значение таймера в "тиках" (максимальная точность)
	static inline tTime Frequency(void); //Узнать частоту таймера (число "тиков" в секунду)

	inline void Start(void); //Запустить таймер
	inline double StartGet(void); //Запустить таймер и узнать значение на момент запуска

	inline void Pause(void); //Пауза таймера
	inline double PauseGet(void); //Пауза таймера и возврат значения

	inline void Stop(void); //Остановка таймера. Отличается от паузы обнулением счётчика
	inline double StopGet(void); //Остановка таймера. Возвращается значение на момент остановки, ДО обнуления
};

//***********************************************************************************************************

class CFrequency
{ //Класс для получения и хранения частоты таймера
	friend class CTimer;
public:
	typedef CTimer::tTime tTime;

private:
	tTime value; //Частота таймера для перевода tTime в секунды

	inline CFrequency(CFrequency const &); //Скрытый конструктор копии
	inline CFrequency &operator=(CFrequency const &); //Скрытый оператор присваивания
public:
	inline CFrequency(void); //Конструктор по-умолчанию
	inline ~CFrequency(void); //Деструктор
};

//***********************************************************************************************************

CFrequency CTimer::frequency; //Частота таймера для перевода tTime в секунды; создание объекта

inline CTimer::tTime CTimer::Now(void)
{ //Узнать текущее значение времени в "тиках"
	CTimer::tTime timeNow;
	BOOL const res( QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER *>(&timeNow) ) );
	assert( res );
	return timeNow;
}

inline CTimer::CTimer(bool const start): state(false),
                                         timeTotal(0),
                                         timeLastStart(0)
{ //Конструктор по-умолчанию
	if(start) Start();
} 

inline CTimer::CTimer(CTimer const &T): state(T.state),
                                        timeTotal(T.timeTotal),
                                        timeLastStart(T.timeLastStart)
{ //Конструктор копии
	;
}

inline CTimer::~CTimer(void)
{ //Деструктор
	;
}

inline CTimer &CTimer::operator=(CTimer const &T)
{ //Оператор присваивания
	state         = T.state;
	timeTotal     = T.timeTotal;
	timeLastStart = T.timeLastStart;
	return *this;
}

inline double CTimer::Get(void) const
{ //Узнать текущее значение таймера в секундах
	if(state) //Таймер бежит
		return static_cast<double>( timeTotal + ( Now() - timeLastStart ) ) / frequency.value;
	else //Таймер остановлен
		return static_cast<double>( timeTotal ) / frequency.value;
}

inline CTimer::tTime CTimer::GetTickCount(void) const
{ //Узнать текущее значение таймера в "тиках"
	if(state) //Таймер бежит
		return timeTotal + ( Now() - timeLastStart );
	else //Таймер остановлен
		return timeTotal;
}

inline CTimer::tTime CTimer::Frequency(void)
{ //Узнать частоту таймера (число "тиков" в секунду)
	return frequency.value;
}

inline void CTimer::Start(void)
{ //Запустить таймер
	assert( !state );
	state = true;
	timeLastStart = Now(); //Запуск таймера
}

inline double CTimer::StartGet(void)
{ //Запустить таймер и узнать значение на момент запуска
	assert( !state );
	double const res( Get() ); //Чтобы возвр. время совпало со временем старта, вызываю Get() до старта
	state = true;
	timeLastStart = Now(); //Запуск таймера
	return res;
}

inline void CTimer::Pause(void)
{ //Пауза таймера
	assert( state );
	state = false;
	timeTotal += Now()-timeLastStart; //Пауза таймера
}

inline double CTimer::PauseGet(void)
{ //Пауза таймера и возврат значения
	assert( state );
	state = false;
	timeTotal += Now()-timeLastStart; //Пауза таймера
	return Get();
}

inline void CTimer::Stop(void)
{ //Остановка таймера. Отличается от паузы обнулением счётчика
	assert( state );
	state = false;
	timeTotal = 0; //Остановка таймера
}

inline double CTimer::StopGet(void)
{ //Остановка таймера. Возвращается значение на момент остановки, ДО обнуления
	assert( state );
	state = false;
	timeTotal += Now()-timeLastStart; //Пауза таймера
	double const res( Get() ); //Чтобы вернуть набежавшее время на момент остановки
	timeTotal = 0; //Обнуляю набежавшее время
	return res; //Возвращаю время до обнуления
}

//***********************************************************************************************************

inline CFrequency::CFrequency(void): value(0)
{ //Конструктор по-умолчанию
	if( !QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER *>(&value) ) ) value = 0;
	assert( value );
}

inline CFrequency::~CFrequency(void)
{ //Деструктор
	;
}

#endif //#ifndef SAN_TIMER