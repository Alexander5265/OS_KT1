#include <windows.h>
#include <iostream>
#include <iomanip>
#include <tchar.h>
#include <time.h>
#include <algorithm> // для std::swap

const int n = 6; // количество строк
const int m = 5; // количество столбцов

float mtx[n][m];       // матрица n строк на m столбцов
int   col_numbers[m];  // массив номеров столбцов (0, 1, 2, 3, 4)

// Функция потока: сортирует ровно один столбец методом обмена (пузырьком)
DWORD WINAPI sort_column(LPVOID param)
{
    // Достаем переданный номер столбца
    int col = *((int*)param);

    // Классическая сортировка пузырьком для одного столбца
    // Идем по строкам: сравниваем элементы mtx[i][col] и mtx[i+1][col]
    for (int step = 0; step < n - 1; step++)
    {
        for (int i = 0; i < n - step - 1; i++)
        {
            if (mtx[i][col] > mtx[i + 1][col])
            {
                // Меняем местами элементы в пределах одного столбца
                std::swap(mtx[i][col], mtx[i + 1][col]);
            }
        }
    }

    return 0;
}

// Вспомогательная функция для аккуратного вывода матрицы
void print_matrix()
{
    for (int i = 0; i < n; i++)
    {
        std::cout << "Строка " << i << ": [ ";
        for (int j = 0; j < m; j++)
        {
            std::cout << std::setw(5) << (int)mtx[i][j] << " ";
        }
        std::cout << "]\n";
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "Russian");

    // 1. Заполняем матрицу случайными числами
    srand((unsigned int)time(NULL));
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < m; j++)
        {
            mtx[i][j] = (float)(rand() % 100);
        }
    }

    std::cout << "--- Исходная матрица ---\n";
    print_matrix();
    std::cout << "\n";

    // 2. Подготовка дескрипторов потоков
    HANDLE hThread[m];
    DWORD dwThreadID[m];

    for (int j = 0; j < m; j++)
    {
        col_numbers[j] = j;
    }

    std::cout << "Запуск " << m << " потоков для параллельной сортировки столбцов...\n";

    // 3. Запуск потоков — по одному на каждый столбец
    for (int j = 0; j < m; j++)
    {
        hThread[j] = CreateThread(
            NULL,
            0,
            sort_column,         // функция сортировки столбца
            &(col_numbers[j]),   // передаем номер столбца
            0,
            &(dwThreadID[j])
        );

        if (hThread[j] == NULL)
        {
            std::cout << "Ошибка создания потока для столбца № " << j << '\n';
        }
    }

    // 4. Ждем, пока ВСЕ m потоков закончат сортировку
    WaitForMultipleObjects(m, hThread, TRUE, INFINITE);

    std::cout << "\n--- Матрица после сортировки каждого столбца по возрастанию ---\n";
    print_matrix();

    // 5. Закрываем дескрипторы потоков
    for (int j = 0; j < m; j++)
    {
        CloseHandle(hThread[j]);
    }

    return 0;
}