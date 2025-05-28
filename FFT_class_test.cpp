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
    twiddle-факторы (через constexpr) и уменьшить аллокации
    (vector<vector<Complex>> matrix).

    Сложности Big(O): time complexity - O(Nlog(N)); space complexity - O(N).
*/

#include <iostream>
#include <vector>
#include <complex>
#include <cmath>
#include <random>
#include <cassert>

using namespace std;
using Complex = complex<double>;
const double PI = acos(-1.0);

// Функция проверки: является ли length кратным только 2, 3 и 5
static inline bool is_valid_length(unsigned int length)
{
    while (length % 2 == 0) length /= 2;
    while (length % 3 == 0) length /= 3;
    while (length % 5 == 0) length /= 5;
    return length == 1;
}

// Класс FFT
class FFT
{
public:
    // Прямое БПФ
    static void fft(vector<Complex>& data)
    {
        unsigned int size = data.size();
        assert(is_valid_length(size));                                          // Проверка, что длина подходит для FFT (должна состоять только из множителей 2, 3 и 5)
        fft_recursive(data, false);
    }

    // Обратное БПФ
    static void ifft(vector<Complex>& data)
    {
        unsigned int size = data.size();
        assert(is_valid_length(size));
        fft_recursive(data, true);
        for (auto& x : data) x /= size;                                         // Нормализация
    }

private:
    static void fft_recursive(vector<Complex>& data, bool invert)
    {
        int size = data.size();
        if (size == 1) return;

        // Разделяем на подмассивы по радиксу (2, 3, 5)
        int radix = get_smallest_factor(size);
        int elements = size / radix;                                            // Количество элементов в каждом подмассиве
        vector<vector<Complex>> matrix(radix, vector<Complex>(elements));       // Матрица: radix подмассивов по elements элементов каждый

        for (int r = 0; r < radix; ++r)                                         // Распределяем элементы исходного массива data в подмассивы matrix
            for (int i = 0; i < elements; ++i)
                matrix[r][i] = data[i * radix + r];                             // Строим подмассивы из исходных данных

        for (int r = 0; r < radix; ++r)                                         // БПФ к каждому подмассиву
            fft_recursive(matrix[r], invert);

        // Сборка выходного массива data из подмассивов matrix с применением twiddle-факторов
        for (int i = 0; i < size; ++i)
        {
            data[i] = 0;                                                        // Обнуляем значение перед накоплением
            for (int r = 0; r < radix; ++r)
            {
                double angle = 2.0 * PI * r * i / size;                         // Угол поворота в комплексной плоскости
                Complex tw_factor = polar(1.0, invert ? angle : -angle);        // Для обратного БПФ - противоположный угол. Twiddle-фактор: tw_factor = e^{-2πi * r * i / size}
                data[i] += tw_factor * matrix[r][i % elements];
            }
        }
    }

    // Находит минимальный делитель из 2, 3, 5
    static int get_smallest_factor(int length)
    {
        if (length % 2 == 0) return 2;
        if (length % 3 == 0) return 3;
        if (length % 5 == 0) return 5;
        return 1;
    }
};

// Генерация случайного вектора комплексных чисел в диапазоне [-1.0, 1.0]
vector<Complex> generate_random_complex_vector(unsigned int length)
{
    random_device rd;                                                           // Источник случайных чисел (обычно аппаратный)
    mt19937 gen(rd());                                                          // Генератор случайных чисел Mersenne Twister, инициализированный rd
    uniform_real_distribution<> distrib(-1.0, 1.0);                             // Равномерное распределение вещественных чисел в диапазоне [-1.0, 1.0]

    vector<Complex> vector(length);                                             // Вектор комплексных чисел заданной длины
    for (auto& complex_number : vector)
        complex_number = Complex(distrib(gen), distrib(gen));                   // Присваиваем каждому элементу случайное комплексное число (Re и Im в [-1, 1])
    return vector;
}

// Подсчет средней ошибки между двумя векторами
double compute_error(const vector<Complex>& origin_data, const vector<Complex>& compute_data)
{
    assert(origin_data.size() == compute_data.size());                          // Проверка: входные векторы должны быть одинаковой длины
    double error = 0.0;
    for (size_t iter = 0; iter < origin_data.size(); ++iter)
        error += abs(origin_data[iter] - compute_data[iter]);                   // Считаем модуль разности комплексных чисел и накапливаем
    return error / origin_data.size();                                          // Среднее значение модулей разностей
}

// Основная функция
int main()
{
    unsigned int length = 300;                                                  // 300 = 2^2 * 3^1 * 5^2; подходит под условия

    vector<Complex> data = generate_random_complex_vector(length);
    vector<Complex> original = data;

    FFT fft;
    fft.fft(data);                                                              // Прямое БПФ
    fft.ifft(data);                                                             // Обратное БПФ

    double error = compute_error(original, data);
    cout << "Средняя ошибка: " << error << endl;

    return 0;
}
