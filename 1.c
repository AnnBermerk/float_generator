// Генератор индивидуальных заданий по теме "Машинное представление вещественных чисел"   
// 1) Выводит результаты в виде Markdown-таблиц              
// 2) Точность = |x - x_back|                       
//
// Папки создаются через system("mkdir -p ...").                  
// Если не хотите system() — создайте папки вручную и удалите вызов system(). 

#include <stdio.h> // Работа с файлами: FILE, fopen, fprintf, printf
#include <stdlib.h> // rand, srand, system, exit-коды
#include <string.h> // memcpy, strlen
#include <time.h> // time() — генерация seed
#include <math.h> // powl, roundl, fabsl, isfinite
#include <ctype.h> // toupper

// = 1) HEX помогаторы ^*^ =

static int hex_value(char c) { // Преобразует один hex-символ в число 0–15
    if ('0' <= c && c <= '9') return c - '0'; // Если цифра — возвращаем её числовое значение
    c = (char)toupper((unsigned char)c); // Переводим букву в верхний регистр
    if ('A' <= c && c <= 'F') return 10 + (c - 'A'); // Если A–F — возвращаем 10–15
    return -1; // Если символ не hex — ошибка
}

// Байты -> строка hex (без пробелов)
static void bytes_to_hex(const unsigned char *bytes, size_t size, char *out) {  // Перевод массива байт в hex-строку
    static const char *H = "0123456789ABCDEF"; // Таблица hex-символов
    for (size_t i = 0; i < size; i++) { // Проходим по каждому байту
        unsigned char b = bytes[size - 1 - i]; // Берем байты с конца, чтобы вывести число в привычном порядке "от старших байт к младшим"
        unsigned char hi = (unsigned char)(b / 16); // Старшая тетрада (старшие 4 бита)
        unsigned char lo = (unsigned char)(b % 16); // Младшая тетрада (младшие 4 бита) 
        out[2*i] = H[hi]; // В позицию 2*i записываем символ, соответствующий старшей тетраде
        out[2*i + 1] = H[lo]; // В позицию 2*i+1 записываем символ, соответствующий младшей тетраде
    }
    out[2*size] = '\0'; // Завершаем строку нулевым символом
}

// Строка hex -> байты
static int hex_to_bytes(const char *hex, unsigned char *out_bytes, size_t size) { // Обратное преобразование
    size_t need = 2 * size; // Сколько символов hex должно быть (на каждый байт приходится 2 hex-символа)
    if (strlen(hex) != need) return 1; // Если длина не совпадает — ошибка

    for (size_t i = 0; i < size; i++) { // По каждому байту
        int hi = hex_value(hex[2*i]); // Старшая hex-цифра
        int lo = hex_value(hex[2*i + 1]); // Младшая hex-цифра
        if (hi < 0 || lo < 0) return 2; // Если символ некорректный — ошибка

        unsigned char b = (unsigned char)(hi * 16 + lo); // Восстанавливаем исходный байт: ст. тетрада *16 + мл. тетрада
        out_bytes[size - 1 - i] = b; // Кладём в обратном порядке (т.е. как привыкли: big-endian)
    }
    return 0; // Ура
}

// = 2) Перевод в машинное представление =

int to_machine(int bits, long double x, char *out_hex, size_t out_hex_size) {  // Преобразование числа в hex
    if (bits == 32) { // Если выбрано 32 бита
        float v = (float)x; // Приведение к float
        unsigned char bytes[sizeof(float)]; // Массив байт размером float
        memcpy(bytes, &v, sizeof(float)); // Копируем байтовое представление

        if (out_hex_size < 2*sizeof(float) + 1) return 10;  // Проверка размера буфера
        bytes_to_hex(bytes, sizeof(float), out_hex); // Переводим в hex-строку
        return 0; // Ура
    }

    if (bits == 64) {  // Если выбрано 64 бита
        double v = (double)x; // Приведение к double
        unsigned char bytes[sizeof(double)];
        memcpy(bytes, &v, sizeof(double));

        if (out_hex_size < 2*sizeof(double) + 1) return 10;
        bytes_to_hex(bytes, sizeof(double), out_hex);
        return 0;
    }

    if (bits == 128) { // Если выбрано 128 бит
        long double v = x; // Используем long double
        unsigned char bytes[sizeof(long double)];
        memcpy(bytes, &v, sizeof(long double));

        if (out_hex_size < 2*sizeof(long double) + 1) return 10;
        bytes_to_hex(bytes, sizeof(long double), out_hex);
        return 0;
    }

    return 20; // Неверное значение bits(
}

// = Перевод из машинного представление =
int from_machine(int bits, const char *hex, long double *x_back) { // Обратное преобразование
    if (!x_back) return 30; // Проверка указателя

    if (bits == 32) {
        unsigned char bytes[sizeof(float)];
        if (hex_to_bytes(hex, bytes, sizeof(float)) != 0) return 40;

        float v;
        memcpy(&v, bytes, sizeof(float)); // Восстанавливаем число из байтов
        *x_back = (long double)v; // Сохраняем в long double
        return 0;
    }

    if (bits == 64) {
        unsigned char bytes[sizeof(double)];
        if (hex_to_bytes(hex, bytes, sizeof(double)) != 0) return 40;

        double v;
        memcpy(&v, bytes, sizeof(double));
        *x_back = (long double)v;
        return 0;
    }

    if (bits == 128) {
        unsigned char bytes[sizeof(long double)];
        if (hex_to_bytes(hex, bytes, sizeof(long double)) != 0) return 40;

        long double v;
        memcpy(&v, bytes, sizeof(long double));
        *x_back = v;
        return 0;
    }

    return 20;
}

// = 3) Генерация =

static long double rand_in_range(long double a, long double b) {  // Генерация числа в диапазоне [a,b]
    long double r = (long double)rand() / (long double)RAND_MAX; // Случайное число в [0,1]
    return a + r * (b - a);  // Масштабирование в нужный диапазон
}

static long double round_to_p(long double x, int p) {  // Округление до p знаков
    long double factor = powl(10.0L, (long double)p); // 10^p
    return roundl(x * factor) / factor; // Формула округления
}


// = 4) main =

int main(void) {
    srand((unsigned)time(NULL)); // Инициализация генератора случайных чисел

    int mk = system("mkdir -p \"Задания\" \"Проверка\""); // Попытка создать папки
    if (mk != 0) { // Если команда завершилась с ошибкой
        printf("Предупреждение: создайте папки вручную.\n"); // Предупреждение пользователю
    }

    int n, k, bits, p; // Параметры генерации
    long double a, b; // Границы диапазона

    FILE *in = fopen("input.txt", "r"); // Открываем входной файл
    if (!in) {
        printf("Ошибка: не могу открыть input.txt\n");
        return 1;
    }

    // Формат: n k bits a b p (в любом расположении по пробелам/переводам строк)
    if (fscanf(in, "%d%d%d%Lf%Lf%d", &n, &k, &bits, &a, &b, &p) != 6) {  // Читаем параметры
        printf("Ошибка: неверный формат input.txt\n");
        fclose(in);
        return 1;
    }
    fclose(in); // Закрываем файл

    // Проверка параметров
    if (n <= 0 || k <= 0) { // Проверка количества вариантов и чисел
        printf("Ошибка: n и k должны быть > 0\n");
        return 1; 
    }
    if (!(bits == 32 || bits == 64 || bits == 128)) { // Проверка разрядности
        printf("Ошибка: bits должен быть 32, 64 или 128\n");
        return 1; 
    }
    if (p < 0 || p > 50) { // 50 — просто разумный верхний предел для вывода
        printf("Ошибка: p должен быть в диапазоне от 0 до 50\n");
        return 1;
    }
    if (!isfinite(a) || !isfinite(b)) { // Проверка корректности чисел
        printf("Ошибка: a и b должны быть конечными числами\n");
        return 1; 
    }

    if (a > b) { // Если границы перепутаны
        long double tmp = a;
        a = b;
        b = tmp;
    }
    if (a == b) {
        printf("Ошибка: диапазон вырожденный (a == b)\n");
        return 1;
    }

    char hex[128]; // Буфер для hex-строки

    for (int v = 1; v <= n; v++) { // Цикл по вариантам
        char file_task[256]; // Имя файла задания
        char file_check[256]; // Имя файла проверки

        snprintf(file_task, sizeof(file_task), "Задания/variant_%d.md", v);  // Формируем имя файла задания
        snprintf(file_check, sizeof(file_check), "Проверка/variant_%d.md", v);

        FILE *ft = fopen(file_task, "w"); // Открываем файл задания
        if (!ft) {
            printf("Ошибка: не могу открыть для записи %s\n", file_task);
            return 1;
        }

        FILE *fc = fopen(file_check, "w"); // Открываем файл проверки
        if (!fc) {
            fclose(ft);
            printf("Ошибка: не могу открыть для записи %s\n", file_check);
            return 1;
        }

        // --- Шапка файлов ---
        fprintf(ft, "# Вариант %d\n\n", v);
        fprintf(ft, "**Параметры:** разрядность=%d, диапазон=[%.*Lf; %.*Lf], кол-во знаков после запятой=%d, чисел=%d\n\n",
                bits, p, a, p, b, p, k);

        fprintf(fc, "# Вариант %d — проверка\n\n", v);
        fprintf(fc, "**Параметры:** разрядность=%d, диапазон=[%.*Lf; %.*Lf], кол-во знаков после запятой=%d, чисел=%d\n\n",
                bits, p, a, p, b, p, k);

        // --- Файл 1: таблица заданий ---
        fprintf(ft, "| № | Вещественное число |\n");   // Заголовок таблицы
        fprintf(ft, "|--:|-------------------:|\n");

        // --- Файл 2: таблица проверки ---
        fprintf(fc, "| № | Вещественное число | Машинное представление | Точность |\n");
        fprintf(fc, "|--:|-------------------:|:----------------------:|---------:|\n");

        for (int i = 1; i <= k; i++) {  // Генерация k чисел
            long double x = rand_in_range(a, b); // Случайное число
            x = round_to_p(x, p); // Округление

            // косметика: убираем "-0.000" на выводе
            if (p >= 0) {
                long double eps0 = 0.5L * powl(10.0L, -(long double)p);
                if (fabsl(x) < eps0) x = 0.0L;
            }

            int rc1 = to_machine(bits, x, hex, sizeof(hex)); // Перевод в машинное представление
            if (rc1 != 0) {
                fclose(ft);
                fclose(fc);
                printf("Ошибка: to_machine() вернул код %d (variant=%d, i=%d)\n", rc1, v, i);
                return 1;
            }

            long double x_back = 0.0L;  // Переменная восстановления
            int rc2 = from_machine(bits, hex, &x_back); // Обратное преобразование
            if (rc2 != 0) {
                fclose(ft);
                fclose(fc);
                printf("Ошибка: from_machine() вернул код %d (variant=%d, i=%d)\n", rc2, v, i);
                return 1;
            }

            long double err = fabsl(x - x_back); // Вычисляем абсолютную ошибку

            fprintf(ft, "| %d | %.*Lf |\n", i, p, x); // Запись числа в файл задания
            fprintf(fc, "| %d | %.*Lf | `%s` | %.6Le |\n", // Запись в файл проверки
                        i, p, x, hex, err);
        }

        fclose(ft); // Закрываем файл задания
        fclose(fc); // Закрываем файл проверки
    }

    printf("Готово.\n");  
    return 0;                                        
}
