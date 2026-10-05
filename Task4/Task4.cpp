#include <windows.h>
#include <iostream>
#include <iomanip>
#include <tchar.h>
#include <time.h>

const int N = 4; // Размерность матрицы 4x4
float mtx[N][N]; // Исходная матрица
float terms[N];  // Массив из 4 слагаемых для каждого потока
int   col_indices[N]; // Индексы 0, 1, 2, 3 для передачи в потоки

// Функция для подсчета определителя подматрицы 3x3
float det3x3(float m[3][3])
{
    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
        - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
        + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

// Функция потока: вычисляет одно слагаемое разложения определителя 4x4
DWORD WINAPI calc_term(LPVOID param)
{
    int col_to_skip = *((int*)param); // Столбец, который нужно вычеркнуть

    float sub[3][3];
    int sub_i = 0;

    // Формируем матрицу 3x3 (вычеркиваем строку 0 и столбец col_to_skip)
    for (int i = 1; i < N; i++) // Начинаем со строки 1 (строка 0 вычеркнута)
    {
        int sub_j = 0;
        for (int j = 0; j < N; j++)
        {
            if (j == col_to_skip)
                continue; // Пропускаем текущий столбец

            sub[sub_i][sub_j] = mtx[i][j];
            sub_j++;
        }
        sub_i++;
    }

    // Считаем определитель получившейся матрицы 3x3
    float minor_val = det3x3(sub);

    // Знак (-1)^col: для четных +1, для нечетных -1
    float sign = (col_to_skip % 2 == 0) ? 1.0f : -1.0f;

    // Слагаемое = знак * элемент первой строки * минор
    terms[col_to_skip] = sign * mtx[0][col_to_skip] * minor_val;

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "Russian");

    // 1. Заполним матрицу небольшими целыми числами от -5 до 5
    // (чтобы определитель не улетал в миллионы и легко проверялся)
    srand((unsigned int)time(NULL));
    std::cout << "--- Исходная квадратная матрица 4x4 ---\n";
    for (int i = 0; i < N; i++)
    {
        std::cout << "[ ";
        for (int j = 0; j < N; j++)
        {
            mtx[i][j] = (float)((rand() % 11) - 5);
            std::cout << std::setw(4) << (int)mtx[i][j] << " ";
        }
        std::cout << "]\n";
    }
    std::cout << "\n";

    // 2. Инициализация параметров для потоков
    HANDLE hThread[N];
    DWORD dwThreadID[N];

    for (int i = 0; i < N; i++)
    {
        col_indices[i] = i;
    }

    std::cout << "Запуск 4 параллельных потоков для вычисления миноров...\n";

    // 3. Запуск 4 потоков
    for (int i = 0; i < N; i++)
    {
        hThread[i] = CreateThread(
            NULL,
            0,
            calc_term,
            &(col_indices[i]),
            0,
            &(dwThreadID[i])
        );

        if (hThread[i] == NULL)
        {
            std::cout << "Ошибка создания потока № " << i << '\n';
        }
    }

    // 4. Ожидаем завершения всех 4 потоков
    WaitForMultipleObjects(N, hThread, TRUE, INFINITE);

    // 5. Суммируем посчитанные потоками слагаемые
    float total_det = 0.0f;
    std::cout << "\n Результаты работы каждого потока (слагаемые):\n";
    for (int i = 0; i < N; i++)
    {
        std::cout << "Слагаемое " << i << " (столбец " << i << "): " << terms[i] << '\n';
        total_det += terms[i];
    }

    std::cout << "\n ИТОГО: Определитель матрицы 4x4 = " << total_det << '\n';

    // 6. Закрываем дескрипторы потоков
    for (int i = 0; i < N; i++)
    {
        CloseHandle(hThread[i]);
    }

    return 0;
}