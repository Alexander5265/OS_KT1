#include <windows.h>
#include <iostream>
#include <iomanip>  // для красивого форматирования вывода (std::setw)
#include <tchar.h>
#include <time.h>

const int m = 5, n = 6;  // 5 строк и 6 столбцов 
float mtx[m][n];         // сама матрица в памяти
int   row_numbers[m];    // массив номеров строк для передачи в потоки


#include <random>

// Функция потока, которая заполняет ровно одну строку
DWORD WINAPI fill_row(LPVOID param)
{
    int row_num = *((int*)param); // номер строки (0-4)

    // Инициализируем генератор для конкретного потока.
    // Используем seed-последовательность из времени и ID потока для уникальности
    std::seed_seq seed{ (unsigned int)time(NULL), (unsigned int)GetCurrentThreadId() };
    std::mt19937 gen(seed);
    
    // Задаем диапазон для вещественных чисел (например, от -50.0 до 50.0)
    std::uniform_real_distribution<float> dist(-50.0f, 50.0f);

    // Заполняем строку случайными числами
    for (int j = 0; j < n; j++)
    {
        mtx[row_num][j] = dist(gen); // Генерирует вещественное число (и полож., и отриц.)
    }
    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "Russian");

    HANDLE hThread[m];
    DWORD dwThreadID[m];

    for (int i = 0; i < m; i++)
    {
        row_numbers[i] = i;
    }

    std::cout << "Запуск " << m << " потоков для параллельного заполнения строк...\n";

    // Запуск потоков — по одному на каждую строку
    for (int i = 0; i < m; i++)
    {
        hThread[i] = CreateThread(
            NULL,
            0, // размер стека
            fill_row,
            &(row_numbers[i]), // аргумент, который передастся в fill_row как параметр
            0, // флаги создания
            &(dwThreadID[i]) // адрес переменной для id потока
        );

        if (hThread[i] == NULL)
        {
            std::cout << "Ошибка создания потока № " << i << ": " << GetLastError() << '\n';
        }
    }

    // Ждем, пока все потоки закончат заполнение
    WaitForMultipleObjects(m, hThread, TRUE, INFINITE); // точка синхронизации

    std::cout << "Все потоки завершили работу. Результирующая матрица:\n\n";

    // Выводим полученную матрицу в консоль
    for (int i = 0; i < m; i++)
    {
        std::cout << "Строка " << i << ": [ ";
        for (int j = 0; j < n; j++)
        {
            std::cout << std::setw(4) << mtx[i][j] << " ";
        }
        std::cout << "]\n";
    }

    // Закрываем дескрипторы потоков
    for (int i = 0; i < m; i++)
    {
        CloseHandle(hThread[i]);
    }

    return 0;
}
