# Компилятор и флаги
CXX := g++
CXXFLAGS := -std=c++20 -O2 -Wall -Wextra

# Редактируемые переменные
NUMBER_OF_LENGTHS ?= 10							# Число длин последовательностей
LOW_EDGE ?= 100									# Нижняя граница длины последовательности
HIGH_EDGE ?= 300								# Нижняя граница длины последовательности

DEFINES = -DNUMBER_OF_LENGTHS=$(NUMBER_OF_LENGTHS) \
		  -DLOW_EDGE=$(LOW_EDGE) \
          -DHIGH_EDGE=$(HIGH_EDGE)
LDFLAGS := -Wl,--stack,2147483648				# 2 ГБ под стек

# Имя исполняемого файла
TARGET := FFT_class_test
TARGET_EXE := $(TARGET).exe

# Исходные C++-файлы
SRCS := $(wildcard *.cpp)
OBJS := $(SRCS:.cpp=.o)

# Python-скрипт
PY_SCRIPT := FFT_class_test.py

.PHONY: all run run_py clean

all: $(TARGET) run run_py clean

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) $(LDFLAGS) -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) -c $< -o $@

# Запуск скомпилированной программы
run: $(TARGET)
	./$(TARGET)

# Запуск python-скрипта
run_py:
	python $(PY_SCRIPT)

# Очистка
clean:
	-del /Q $(subst /,\,$(TARGET_EXE) $(OBJS)) >nul 2>&1

# Инструкция по использованию
# ---------------------------
# make или make all - компиляция C++-программы, её запуск, запуск python-скрипт, удаление .o- и .exe-файлов
# make run - запуск скомпилированной программы
# make run_py - запуск python-скрипта
# make clean - удалит объектные и исполняемый файлы
# ---------------------------
# Makefile предполагает, что все .cpp-файлы в текущей директории являются частью проекта
# Для работы требуется установленные g++ и python3 в системе