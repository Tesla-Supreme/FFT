/*
    Задание:
    1.  Написать на С++ класс быстрого прямого и обратного преобразования Фурье комплексных значений с возможной длиной преобразования кратной 2, 3, 5.
    2.  Запустить для случайных комплексных входных данных сначала прямое, а потом обратное преобразование Фурье.
    3.  Сравнить ошибку между входными и выходными данными.
*/

/*
    C++-код, реализующий прямое (FFT) и обратное (IFFT) БПФ
    для комплексных чисел с длиной преобразования, кратной
    2, 3, 5. Используется алгоритм mixed-radix Cooley-Tukey
    FFT, способный обрабатывать входы размером 2^a * 3^b * 5^c.

    Базовая реализация рекурсивного БПФ с Radix-2/3/5.
    Работает корректно. Для оптимизации можно заменить
    рекурсию на итеративный алгоритм, предвычислять
    twiddle-факторы и уменьшить аллокации (vector<vector<Complex>> matrix).

    Сложности Big(O): time complexity - O(Nlog(N)); space complexity - O(N).
*/

#include <iostream>
#include <vector>
#include <complex>
#include <random>
#include <cassert>
#include <numbers>
#include <algorithm>
#include <windows.h>
#include <fstream>

using namespace std;
using Complex = complex<double>;
static const double PI = std::numbers::pi;
static random_device rd;                                                        // Источник энтропии (аппаратный)
static mt19937 gen(rd());                                                       // Вихрь Мерсенна

#if !defined(NUMBER_OF_LENGTHS)                                                 // Число длин последовательностей
    #define NUMBER_OF_LENGTHS 10                                                // (определяет количество итераций для проверки class FFT)
#endif

#if !defined(LOW_EDGE)                                                          // Нижняя граница длины последовательности
    #define LOW_EDGE 100
#endif

#if !defined(HIGH_EDGE)                                                         // Верхняя граница длины последовательности
    #define HIGH_EDGE 300
#endif

// Функция проверки: является ли length кратным только 2, 3 и 5
static inline bool is_valid_length(const unsigned int length)
{
    return (length % (2 * 3 * 5) == 0);
}

// Генерация целого случайного числа в диапазоне [LOW_EDGE, HIGH_EDGE]
static unsigned int generate_random_number(const bool multiple_235)
{
    uniform_int_distribution<unsigned int> dist(LOW_EDGE, HIGH_EDGE);           // Равномерное распределение целых чисел в диапазоне [LOW_EDGE, HIGH_EDGE]
    if (multiple_235)
    {
        while (true)
        {
            unsigned int randomValue = dist(gen);
            if (is_valid_length(randomValue)) return randomValue;
        }
    }
    else
    {
        while (true)
        {
            unsigned int randomValue = dist(gen);
            if (!is_valid_length(randomValue)) return randomValue;
        }
    }
}

// Вывод длин последовательностей
static inline void show_lengths(const vector<unsigned int>& lengths)
{
    cout << "Длины последовательностей, кратные 2, 3, 5: ";
    for (const auto& element : lengths)
        if (is_valid_length(element)) cout << element << " ";
    cout << endl;

    cout << "Длины последовательностей, не кратные 2, 3, 5: ";
    for (const auto& element : lengths)
        if (!is_valid_length(element)) cout << element << " ";
    cout << endl;
}

// Формирование вектора длин последовательностей
static vector<unsigned int> form_lengths_values()
{
    vector<unsigned int> lengths;
    lengths.reserve(NUMBER_OF_LENGTHS);                                         // Резервирование (во избежании реаллокаций)
    bool multiple_235 = true;                                                   // Кратный 2, 3, 5 (true - да)
    const unsigned int middlePos = (int)(NUMBER_OF_LENGTHS / 2);                // Середина вектора lengths
    for (unsigned int iter_vec = 0; iter_vec < NUMBER_OF_LENGTHS; ++iter_vec)   // Формирование длин последовательностей
    {
        unsigned int valueForVec{0};
        if (iter_vec < middlePos)
            valueForVec = generate_random_number(multiple_235);
        else valueForVec = generate_random_number(!multiple_235);
        lengths.push_back(valueForVec);
    }
    shuffle(lengths.begin(), lengths.end(), gen);                               // Перемешивание lengths
    show_lengths(lengths);
    return lengths;
}

// Генерация случайного вектора комплексных чисел в диапазоне [-1.0, 1.0]
static vector<Complex> generate_random_complex_vector(const unsigned int length)
{
    uniform_real_distribution<double> distrib(-1.0, 1.0);                       // Равномерное распределение вещественных чисел в диапазоне [-1.0, 1.0]
    vector<Complex> vector(length);                                             // Вектор комплексных чисел заданной длины
    for (auto& complex_number : vector)
        complex_number = Complex(distrib(gen), distrib(gen));                   // Присваиваем каждому элементу случайное комплексное число (Re и Im в [-1, 1])
    return vector;
}

// Запись вектора комплексных чисел в файл
static void write_data(const vector<Complex>& origin_data, const bool append)
{
    ofstream data_file;
    data_file.open("Complex_vectors.txt", append ? ios::app : ios::out);        // Открытие файла на запись
    assert(data_file.is_open() && "Failed to open file for writing");           // Проверка открытия файла
    if (!append)                                                                // Для чтения через python-скрипт
    {
        data_file << "Число валидных длин последовательностей: "\
                  << (int)(NUMBER_OF_LENGTHS / 2) << "\n\n";
    }
    data_file << "Длина вектора: " << origin_data.size() << " элементов\n";     // Запись в формате (real, imag)
    for (const auto& element : origin_data)                                     
        data_file << "(" << element.real() << ", " << element.imag() << ")\n";
    data_file << "\n";
    data_file.close();                                                          // Закрытие файла
}

// Подсчет средней ошибки между двумя векторами с последующей записью
static double compute_error(const vector<Complex>& origin_data,\
                            const vector<Complex>& compute_data,\
                            const bool append)
{
    assert(origin_data.size() == compute_data.size());                          // Проверка: входные векторы должны быть одинаковой длины
    double error = 0.0;
    for (size_t iter = 0; iter < origin_data.size(); ++iter)
        error += abs(origin_data[iter] - compute_data[iter]);                   // Считаем модуль разности комплексных чисел и накапливаем

    ofstream results_file;                                                      // Запись результатов моделирования
    results_file.open("Results.txt", append ? ios::app : ios::out);
    if (!append)
    {
        results_file << "Формат записи: [длина вектора; средняя ошибка]"\
                     << "\n\n";
        results_file << "Результаты С++-скрипта:\n";
    }
    results_file << "[" << origin_data.size() << "; " << error << "];\n";
    results_file.close();
    return error / origin_data.size();                                          // Среднее значение модулей разностей
}

// Класс FFT
class FFT
{
public:
    // Прямое БПФ
    static void fft(vector<Complex>& data)
    {
        fft_recursive(data, false);
    }

    // Обратное БПФ
    static void ifft(vector<Complex>& data)
    {
        const unsigned int size = data.size();
        fft_recursive(data, true);
        for (auto& element : data) element /= size;                             // Нормализация
    }

private:
    static void fft_recursive(vector<Complex>& data, const bool invert)
    {
        const unsigned int size = data.size();
        if (size == 1) return;

        // Разделяем на подмассивы по радиксу (2, 3, 5)
        const unsigned int radix = get_smallest_factor(size);
        const unsigned int elements = size / radix;                             // Количество элементов в каждом подмассиве
        vector<vector<Complex>> matrix(radix, vector<Complex>(elements));       // Матрица: radix подмассивов по elements элементов каждый

        for (unsigned int r = 0; r < radix; ++r)                                // Распределяем элементы исходного массива data в подмассивы matrix
            for (unsigned  int i = 0; i < elements; ++i)
                matrix[r][i] = data[i * radix + r];                             // Строим подмассивы из исходных данных

        for (unsigned int r = 0; r < radix; ++r)                                // БПФ к каждому подмассиву
            fft_recursive(matrix[r], invert);

        // Сборка выходного массива data из подмассивов matrix с применением twiddle-факторов
        for (unsigned int i = 0; i < size; ++i)
        {
            data[i] = 0;                                                        // Обнуляем значение перед накоплением
            for (unsigned int r = 0; r < radix; ++r)
            {
                const double angle = 2.0 * PI * r * i / size;                   // Угол поворота в комплексной плоскости
                const Complex tw_factor = polar(1.0, invert ? angle : -angle);  // Для обратного БПФ - противоположный угол. Twiddle-фактор: tw_factor = e^{-2πi * r * i / size}
                data[i] += tw_factor * matrix[r][i % elements];
            }
        }
    }

    // Находит минимальный делитель из 2, 3, 5
    static unsigned int get_smallest_factor(const unsigned int length)
    {
        if (length % 2 == 0) return 2;
        if (length % 3 == 0) return 3;
        if (length % 5 == 0) return 5;
        return 1;
    }
};

// Основная функция
int main()
{
    SetConsoleOutputCP(CP_UTF8);                                                // Для корректного отображения кириллицы
    const vector<unsigned int> lengths = form_lengths_values();                 // Вектор длин последовательностей
    
    // Поиск позиции 1-го валидного элемента в lengths
    // для корректной записи в файлы
    const auto iterator = find_if(lengths.cbegin(), lengths.cend(),\
    [](int element) { return is_valid_length(element); });
    const ptrdiff_t index = distance(lengths.cbegin(), iterator);
    
    // Вектора последовательностей
    vector<Complex> data;
    vector<Complex> original;
    data.reserve(HIGH_EDGE);
    original.reserve(HIGH_EDGE);
    cout << endl << "Средняя ошибка для вектора длиной:" << endl;               // Для отладки

    for (int main_iter = 0; main_iter < NUMBER_OF_LENGTHS; ++main_iter)
    {
        unsigned int length = lengths.at(main_iter);                            // Длина входного вектора комплексных чисел
        if (is_valid_length(length))
        {
            data = generate_random_complex_vector(length);
            original = data;

            if (main_iter == index) write_data(data, false);
            else write_data(data, true);

            FFT fft;
            fft.fft(data);
            fft.ifft(data);
            
            double error{0.0};
            if (main_iter == index) error = compute_error(original, data, false);
            else error = compute_error(original, data, true);
            cout << main_iter + 1 << ". " << length << " элементов: = "
                 << error << " (запись в файлы успешна);" << endl;
            
            data.clear();
            original.clear();
        }
        else
        {
            cout << main_iter + 1 << ". " << length
                 << " элементов: None (не кратный 2, 3, 5);" << endl;
        }
    }
    cout << endl;
    return 0;
}