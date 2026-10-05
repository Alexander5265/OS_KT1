#include <windows.h>
#include <iostream>
#include <tchar.h>

// Структура узла бинарного дерева
struct TreeNode
{
    int val;
    TreeNode* left;
    TreeNode* right;

    TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
};

// Простая последовательная функция подсчета суммы поддерева
int sequential_tree_sum(TreeNode* node)
{
    if (node == nullptr)
        return 0;

    return node->val + sequential_tree_sum(node->left) + sequential_tree_sum(node->right);
}

// Структура для передачи параметров в поток
struct ThreadData
{
    TreeNode* root;  // Корень поддерева для подсчета
    int result_sum;  // Сюда поток запишет результат
};

// Функция потока: считает сумму поддерева
DWORD WINAPI calc_subtree_sum(LPVOID param)
{
    ThreadData* data = (ThreadData*)param;
    data->result_sum = sequential_tree_sum(data->root);
    return 0;
}

// Вспомогательная функция очистки памяти дерева
void delete_tree(TreeNode* node)
{
    if (node == nullptr) return;
    delete_tree(node->left);
    delete_tree(node->right);
    delete node;
}

int _tmain(int argc, _TCHAR* argv[])
{
    setlocale(LC_ALL, "Russian");

    /*
          Построим тестовое дерево:
                    10
                  /    \
                20      30
               /  \    /  \
              5    8  12  15

          Левое поддерево (корень 20):  20 + 5 + 8 = 33
          Правое поддерево (корень 30): 30 + 12 + 15 = 57
          Корень: 10
          ИТОГО сумма должна быть: 10 + 33 + 57 = 100
    */

    TreeNode* root = new TreeNode(10);
    root->left = new TreeNode(20);
    root->right = new TreeNode(30);

    root->left->left = new TreeNode(5);
    root->left->right = new TreeNode(8);

    root->right->left = new TreeNode(12);
    root->right->right = new TreeNode(15);

    std::cout << "--- Бинарное дерево создано ---\n";
    std::cout << "Корень: 10\n";
    std::cout << "Левое поддерево: 20, потомки: 5, 8 (ожидаемая сумма = 33)\n";
    std::cout << "Правое поддерево: 30, потомки: 12, 15 (ожидаемая сумма = 57)\n\n";

    // 1. Подготавливаем поток для левого поддерева
    ThreadData leftData;
    leftData.root = root->left;
    leftData.result_sum = 0;

    DWORD leftThreadID;
    HANDLE hLeftThread = CreateThread(
        NULL,
        0,
        calc_subtree_sum,
        &leftData,
        0,
        &leftThreadID
    );

    if (hLeftThread == NULL)
    {
        std::cout << "Ошибка создания потока для левого поддерева!\n";
        return 1;
    }

    std::cout << "Запущен отдельный поток для подсчета левого поддерева...\n";

    // 2. В это же время текущий поток считает сумму правого поддерева
    std::cout << "Основной поток считает сумму правого поддерева...\n";
    int right_sum = sequential_tree_sum(root->right);

    // 3. Ждем завершения потока левого поддерева
    WaitForSingleObject(hLeftThread, INFINITE);
    int left_sum = leftData.result_sum;

    // Закрываем дескриптор
    CloseHandle(hLeftThread);

    // 4. Складываем все компоненты
    int total_sum = root->val + left_sum + right_sum;

    std::cout << "\n--- Результаты вычислений ---\n";
    std::cout << "Сумма левого поддерева (из потока): " << left_sum << '\n';
    std::cout << "Сумма правого поддерева (из main):   " << right_sum << '\n';
    std::cout << "Значение в корне:                    " << root->val << '\n';
    std::cout << "-------------------------------------------\n";
    std::cout << "Общая сумма элементов дерева:        " << total_sum << '\n';

    // Освобождаем память
    delete_tree(root);

    return 0;
}