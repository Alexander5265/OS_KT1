#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <tchar.h>
#include <algorithm> // для std::reverse

// Глобальные переменные для удобного доступа потоков
std::string text = "Hello, this message will be hidden soon!";
std::string key = "SECRET";

// Структура параметров, передаваемая в каждый поток
struct BlockTask
{
    int block_id;   // номер блока
    int start_idx;  // начальный индекс блока в строке text
    int len;        // фактическая длина блока
};

// Функция потока: шифрует ровно один блок
DWORD WINAPI encrypt_block(LPVOID param)
{
    BlockTask* task = (BlockTask*)param;

    int start = task->start_idx;
    int len = task->len;

    // Шаг 1: Перестановка символов (реверс блока)
    // Символы блока text[start ... start + len - 1] переворачиваются
    std::reverse(text.begin() + start, text.begin() + start + len);

    // Шаг 2: Посимвольный XOR с ключом
    for (int i = 0; i < len; i++)
    {
        text[start + i] = text[start + i] ^ key[i];
    }

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "Russian");

    int n = (int)text.length();
    int k = (int)key.length();

    // Количество блоков (округление вверх)
    int num_blocks = (n + k - 1) / k;

    std::cout << "--- Исходные данные ---\n";
    std::cout << "Текст: \"" << text << "\" (длина: " << n << ")\n";
    std::cout << "Ключ:  \"" << key << "\" (длина: " << k << ")\n";
    std::cout << "Количество блоков: " << num_blocks << "\n\n";

    // Создаем массивы для потоков и их параметров
    std::vector<HANDLE> hThreads(num_blocks);
    std::vector<DWORD> dwThreadIDs(num_blocks);
    std::vector<BlockTask> tasks(num_blocks);

    std::cout << "Запуск потоков шифрования блоков...\n";

    // Формируем параметры и запускаем поток для каждого блока
    for (int b = 0; b < num_blocks; b++)
    {
        tasks[b].block_id = b;
        tasks[b].start_idx = b * k;

        // Длина текущего блока: k, либо остаток для последнего блока
        tasks[b].len = (std::min)(k, n - tasks[b].start_idx);

        hThreads[b] = CreateThread(
            NULL,
            0,
            encrypt_block,
            &(tasks[b]),
            0,
            &(dwThreadIDs[b])
        );

        if (hThreads[b] == NULL)
        {
            std::cout << "Ошибка создания потока для блока № " << b << '\n';
        }
    }

    // Ожидаем завершения шифрования всех блоков
    WaitForMultipleObjects(num_blocks, hThreads.data(), TRUE, INFINITE);

    std::cout << "\nШифрование завершено!\n";

    // Зашифрованный текст содержит непечатные байты после XOR,
    // поэтому выведем его в виде шестнадцатеричных (HEX) кодов байтов:
    std::cout << "Зашифрованный текст (HEX): ";
    for (unsigned char c : text)
    {
        std::cout << std::hex << (int)c << " ";
    }
    std::cout << std::dec << "\n";

    // Закрываем дескрипторы потоков
    for (int b = 0; b < num_blocks; b++)
    {
        CloseHandle(hThreads[b]);
    }

    return 0;
}