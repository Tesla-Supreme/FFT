import numpy as np
import re

def read_results(filename, start_pos = 0, type: str = ""):
    """Чтение результатов из файла"""
    with open(filename, 'r', encoding = 'utf-8') as results:
        results.seek(start_pos)

        # Настройка позиции итератора
        if not start_pos and type == "cpp":                                     # Пропуск первых 3-х строк (при первом чтении С++-результата)
            for iterator in range(1, 4):   
                results.readline()                                                       
        elif not start_pos and type == "python":                                # Пропуск первых 10-х строк (при первом чтении python-результата)                       
            for iterator in range(1, 11):   
                results.readline()

        # Парсинг строки результата
        line = results.readline()
        line = line.strip().strip('[];')
        vector_size, error = [x.strip() for x in line.split(';')]                                        
        vector_size = int(vector_size)
        error = float(error)
        current_pos = results.tell()
        return vector_size, error, current_pos

def read_complex_numbers(filename, start_pos = 0):
    """Чтение вектора комплексных чисел из файла"""
    complex_vector = []
    with open(filename, 'r') as vectors:
        vectors.seek(start_pos)                                                 # Переход к сохраненной позиции
        
        # Настройка позиции итератора
        if not start_pos:                                                       # Пропуск первых 3-х строк (при первом чтении)
            vectors.readline()                                                       
            vectors.readline()
            vectors.readline()
        else:                                                                   # Пропуск строки (при последующих чтениях)
            vectors.readline()
        
        while True:
            line = vectors.readline()
            if (not line) or (line == '\n'):
                current_pos = vectors.tell()                                    # Запоминание текущей позиции
                return np.array(complex_vector), current_pos
            
            # Парсинг строки вектора
            line = line.strip().strip('()')
            real_str, imag_str = [x.strip() for x in line.split(',')]
            real = float(real_str)
            imag = float(imag_str)

            complex_vector.append(complex(real, imag))

def main():
    input_file = 'Complex_vectors.txt'                                          # Файл с векторами комплексных чисел
    with open(input_file, 'r') as vectors:
        line = vectors.readline()
        numbers = re.findall(r'\d+', line)                                      # Поиск числа в строке
        vectors_number = int(numbers[0])

    for iteration in range(1, vectors_number + 1):
        if iteration == 1:                                                      # Чтение данных из файла
            original_data, position = read_complex_numbers(input_file)
        else:
            original_data, position = read_complex_numbers\
                (input_file, start_pos = position)

        fft_data = np.fft.fft(original_data)                                    # Прямое преобразование Фурье
        ifft_data = np.fft.ifft(fft_data)                                       # Обратное преобразование Фурье
            
        errors = np.abs(original_data - ifft_data)                              # Расчёт средней ошибки                    
        mean_error = np.mean(errors)
            
        with open("Results.txt", 'a', encoding = 'utf-8') as results:           # Запись результатов в "Results.txt"
             if iteration == 1:
                 results.write("\nРезультаты python-скрипта:\n")
             results.write(f"[{original_data.size}; {mean_error:.5e}];\n")

    result_file = "Results.txt"
    for iteration in range(1, vectors_number + 1):                              # Сравнение ошибок
        if iteration == 1:
            size, result_cpp, position_cpp = read_results(result_file, 0, "cpp")
            size, result_python, position_python = read_results\
                (result_file, 0, "python")
        else:
            size, result_cpp, position_cpp = read_results\
                (result_file, position_cpp, "cpp")
            size, result_python, position_python = read_results\
                (result_file, position_python, "python")

        with open("Results.txt", 'a', encoding = 'utf-8') as results:
            if iteration == 1:
                results.write("\nРазность ошибок:\n")
            results.write(f"[{size}; {abs(result_cpp - result_python):.5e}];\n")

if __name__ == "__main__":                                                      # При исполнении файла напрямую
    main()