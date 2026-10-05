#include <windows.h>
#include <iostream>
#include <iomanip>
#include <tchar.h>
#include <time.h>

const int n = 5; // n строк
const int m = 6; // m столбцов

float mtx[n][m];          // матрица
float row_averages[n];    // массив, куда потоки запишут средние значения строк
int   row_numbers[n];     // номера строк (0, 1, 2, 3, 4)

// Функция потока: считает среднее значение для одной строки
DWORD WINAPI calc_row_average(LPVOID param)
{
    // Достаем номер строки
    int row_num = *((int*)param);

    // Считаем сумму элементов строки
    float sum = 0.0f;
    for (int j = 0; j < m; j++)
    {
        sum += mtx[row_num][j];
    }

    // Записываем среднее арифметическое в общий массив
    row_averages[row_num] = sum / m;

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "Russian");

    // 1. Заполним матрицу числами от 0 до 99 и сразу выведем её
    srand((unsigned int)time(NULL));
    std::cout << "Исходная матрица:\n";
    for (int i = 0; i < n; i++)
    {
        std::cout << "Строка " << i << ": [ ";
        for (int j = 0; j < m; j++)
        {
            mtx[i][j] = (float)(rand() % 100);
            std::cout << std::setw(4) << mtx[i][j] << " ";
        }
        std::cout << "]\n";
    }
    std::cout << "\n";

    // 2. Подготовка к запуску потоков
    HANDLE hThread[n];
    DWORD dwThreadID[n];

    for (int i = 0; i < n; i++)
    {
        row_numbers[i] = i;
    }

    // 3. Запуск n параллельных потоков
    for (int i = 0; i < n; i++)
    {
        hThread[i] = CreateThread(
            NULL,
            0,
            calc_row_average,    // функция вычисления среднего
            &(row_numbers[i]),   // передаем номер строки
            0,
            &(dwThreadID[i])
        );

        if (hThread[i] == NULL)
        {
            std::cout << "Ошибка создания потока № " << i << '\n';
        }
    }

    // 4. Ожидаем завершения ВСЕХ n потоков
    WaitForMultipleObjects(n, hThread, TRUE, INFINITE);

    // 5. Выводим посчитанные средние значения
    std::cout << "Средние значения строк, посчитанные потоками:\n";
    for (int i = 0; i < n; i++)
    {
        std::cout << "Строка " << i << ": " << std::fixed << std::setprecision(2) << row_averages[i] << '\n';
    }

    // 6. Ищем строку с максимальным средним значением
    int max_row_index = 0;
    float max_avg = row_averages[0];

    for (int i = 1; i < n; i++)
    {
        if (row_averages[i] > max_avg)
        {
            max_avg = row_averages[i];
            max_row_index = i;
        }
    }

    std::cout << "\nРезультат: строка с максимальным средним значением — № "
        << max_row_index << " (среднее = " << max_avg << ")\n";

    // 7. Закрываем дескрипторы
    for (int i = 0; i < n; i++)
    {
        CloseHandle(hThread[i]);
    }

    return 0;
}