#include <windows.h>
#include <iostream>
#include <string>
#include <vector>
#include <tchar.h>
#include <algorithm> // для std::reverse

// Глобальные данные
std::string text = "Hello, this message will be hidden soon!";
std::string key = "SECRET";

// Структура параметров для передачи в поток
struct BlockTask
{
    int block_id;   // номер блока
    int start_idx;  // индекс начала блока
    int len;        // длина блока
};

// --- Функция потока: ШИФРОВАНИЕ блока (Задание 5) ---
DWORD WINAPI encrypt_block(LPVOID param)
{
    BlockTask* task = (BlockTask*)param;
    int start = task->start_idx;
    int len = task->len;

    // 1. Реверс
    std::reverse(text.begin() + start, text.begin() + start + len);

    // 2. XOR с ключом
    for (int i = 0; i < len; i++)
    {
        text[start + i] = text[start + i] ^ key[i];
    }

    return 0;
}

// --- Функция потока: ДЕШИФРОВАНИЕ блока (Задание 6) ---
DWORD WINAPI decrypt_block(LPVOID param)
{
    BlockTask* task = (BlockTask*)param;
    int start = task->start_idx;
    int len = task->len;

    // 1. Сначала снимаем XOR тем же ключом
    for (int i = 0; i < len; i++)
    {
        text[start + i] = text[start + i] ^ key[i];
    }

    // 2. Затем делаем обратный реверс блока
    std::reverse(text.begin() + start, text.begin() + start + len);

    return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "Russian");

    int n = (int)text.length();
    int k = (int)key.length();
    int num_blocks = (n + k - 1) / k; // количество блоков

    std::cout << "--- Исходный текст ---\n";
    std::cout << text << "\n\n";

    // Подготовка задач для каждого блока
    std::vector<BlockTask> tasks(num_blocks);
    for (int b = 0; b < num_blocks; b++)
    {
        tasks[b].block_id = b;
        tasks[b].start_idx = b * k;
        tasks[b].len = (std::min)(k, n - tasks[b].start_idx);
    }

    // ==========================================
    // 1. ЭТАП ШИФРОВАНИЯ (Задание 5)
    // ==========================================
    std::vector<HANDLE> hEncryptThreads(num_blocks);
    for (int b = 0; b < num_blocks; b++)
    {
        hEncryptThreads[b] = CreateThread(NULL, 0, encrypt_block, &(tasks[b]), 0, NULL);
    }
    WaitForMultipleObjects(num_blocks, hEncryptThreads.data(), TRUE, INFINITE);

    for (int b = 0; b < num_blocks; b++)
    {
        CloseHandle(hEncryptThreads[b]);
    }

    std::cout << "--- Зашифрованный текст (HEX-байты) ---\n";
    for (unsigned char c : text)
    {
        std::cout << std::hex << (int)c << " ";
    }
    std::cout << std::dec << "\n\n";

    // ==========================================
    // 2. ЭТАП ДЕШИФРОВАНИЯ (Задание 6)
    // ==========================================
    std::cout << "Запуск потоков для параллельного дешифрования...\n";

    std::vector<HANDLE> hDecryptThreads(num_blocks);
    for (int b = 0; b < num_blocks; b++)
    {
        hDecryptThreads[b] = CreateThread(NULL, 0, decrypt_block, &(tasks[b]), 0, NULL);
    }
    WaitForMultipleObjects(num_blocks, hDecryptThreads.data(), TRUE, INFINITE);

    for (int b = 0; b < num_blocks; b++)
    {
        CloseHandle(hDecryptThreads[b]);
    }

    std::cout << "\n--- Расшифрованный текст ---\n";
    std::cout << text << "\n";

    return 0;
}