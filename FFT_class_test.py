import numpy as np
import re

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
            
            line = line.strip().strip('()')                                     # Выделение real- и imag-частей
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

if __name__ == "__main__":                                                      # При исполнении файла напрямую
    main()