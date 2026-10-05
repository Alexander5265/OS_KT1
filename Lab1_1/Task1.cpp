#include <windows.h>
#include <iostream>
#include <iomanip>  // для красивого форматирования вывода (std::setw)
#include <tchar.h>
#include <time.h>

const int m = 5, n = 6;  // 5 строк и 6 столбцов (чтобы аккуратно помещалось в консоль)
float mtx[m][n];         // сама матрица в памяти
int   row_numbers[m];    // массив номеров строк для передачи в потоки


// Функция потока: заполняет ровно одну строку
DWORD WINAPI fill_row(LPVOID param)
{
    int row_num = *((int*)param); // лежит номер строки (0-4)

    // Инициализируем генератор уникальным значением для текущего потока
    srand((unsigned int)(time(NULL) ^ GetCurrentThreadId()));

    // Заполняем строку случайными числами
    for (int j = 0; j < n; j++)
    {
        mtx[row_num][j] = (float)(rand() % 100);
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
            0,
            fill_row,
            &(row_numbers[i]),
            0,
            &(dwThreadID[i])
        );

        if (hThread[i] == NULL)
        {
            std::cout << "Ошибка создания потока № " << i << ": " << GetLastError() << '\n';
        }
    }

    // Ждем, пока все потоки закончат заполнение
    WaitForMultipleObjects(m, hThread, TRUE, INFINITE);

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