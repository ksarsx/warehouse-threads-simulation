# warehouse-threads-simulation
My solution to the problem of simulating multi-threading in a warehouse for several trucks - loaders and unloaders.

# Лабораторная работа: Склад с ограниченным числом погрузочно-разгрузочных доков / Warehouse with a Limited Number of Loading Docks

## RU | Описание задания

### 1. Цель задания
* **Освоение инструментов синхронизации:** Научиться правильно использовать мьютексы (`mutex`) и семафоры (`counting-` и `binary-semaphore`) для синхронизации потоков.
* **Изучение проблем многопоточности:** Понять типичные подводные камни многопоточного программирования: состояние гонки, взаимные блокировки (deadlocks) и «залипающие» потоки.
* **Анализ производительности:** Оценить влияние количества ресурсов (доков) и количества работающих потоков на общую производительность системы.

---

### 2. Сценарий (Бизнес-логика)
В рамках задания моделируется работа склада, на котором хранится товар (вес в условных единицах):
* **Вместимость склада:** Максимальная ёмкость склада ($CAP$) ограничена и составляет **1000 единиц**.
* **Инфраструктура:** На складе доступно **$N$ погрузочно-разгрузочных доков** (например, 3–5). Одновременно в одном доке может обслуживаться только один грузовик.

#### Поведение грузовиков
Грузовики (по одному на каждый поток) приближаются к складу и случайным образом выбирают одну из двух операций:
1. **Загрузка:** Привозят товар и кладут его на склад. Операция возможна, только если:
   <img width="214" height="23" alt="изображение" src="https://github.com/user-attachments/assets/36bae219-5666-46ec-bf80-ad5925ff6b13" />

3. **Разгрузка:** Берут товар со склада и увозят его. Операция возможна, только если:
   <img width="165" height="17" alt="изображение" src="https://github.com/user-attachments/assets/f38718af-b4b8-4b62-9acd-05b92357269a" />


Каждая операция имеет случайный объём $A$ в диапазоне **от 1 до 100 единиц**.

> **Важно:** Если условие выполнения не выполнено, грузовик ждёт, пока ситуация изменится (например, другой грузовик выполнит противоположную операцию). Активное ожидание (busy-wait) недопустимо.

#### Требования к логированию
Все действия должны логироваться в консоль или в файл с обязательным указанием следующих данных:
* Идентификатор грузовика;
* Тип операции и её объём;
* Результат выполнения;
* Текущий запас склада;
* Номер занятого дока.

---

### 3. Требования к реализации

| № | Требование |
| :-: | :--- |
| **1** | Приложение должно создавать $M$ потоков-грузовиков ($M \ge 10$, задаётся параметром). |
| **2** | Каждый поток должен выполнить $K$ операций ($K \ge 10$, задаётся параметром). |
| **3** | Управление доками реализуется с помощью считающего семафора (`counting-semaphore`) с начальным значением $N$. |
| **4** | Защита инвентаря склада (целочисленная переменная) реализуется мьютексом (`mutex`). |
| **5** | Ожидание невозможной операции (недостаточно места/товара) должно использовать `binary-semaphore` или `std::condition_variable` (по желанию). Активный «busy-wait» недопустимо. |
| **6** | Приложение должно завершаться корректно: все потоки завершат работу, семафоры и мьютекс освободятся, финальный запас будет в диапазоне $[0, CAP]$. |
| **7** | Программа не должна уходить в `deadlock` при любой комбинации параметров $N, M, K$. |
| **8** | При запуске выводятся сообщения о начале/завершении каждой операции и итоговый запас. |
| **9** | Исходный код должен быть оформлен (комментарии, читаемые имена переменных, деление на функции). |
| **10** | *(Опционально)* Предусмотреть возможность задать параметры $N, M, K$ через аргументы командной строки. |

---
---

## EN | Task Description

### 1. Assignment Objectives
* **Mastering Synchronization Tools:** Learn how to properly use mutexes and semaphores (both counting and binary semaphores) for thread synchronization.
* **Understanding Multithreading Pitfalls:** Comprehend classic concurrent programming issues, such as race conditions, deadlocks, and thread starvation ("stuck" threads).
* **Performance Evaluation:** Analyze how the amount of resources (docks) and the number of working threads affect overall system performance.

---

### 2. Scenario (Business Logic)
The project models a warehouse that stores goods (measured in arbitrary weight units):
* **Warehouse Capacity:** The maximum capacity ($CAP$) is limited to **1000 units**.
* **Infrastructure:** The warehouse has **$N$ loading/unloading docks** (e.g., 3–5). Only one truck can be processed in a single dock at any given time.

#### Truck Behavior
Trucks (one per thread) approach the warehouse and randomly choose one of two operations:
1. **Loading:** Trucks bring cargo and place it into the warehouse. This operation is allowed only if:
   <img width="214" height="23" alt="изображение" src="https://github.com/user-attachments/assets/36bae219-5666-46ec-bf80-ad5925ff6b13" />
2. **Unloading:** Trucks take cargo from the warehouse and transport it away. This operation is allowed only if:
   <img width="165" height="17" alt="изображение" src="https://github.com/user-attachments/assets/f38718af-b4b8-4b62-9acd-05b92357269a" />

Each operation handles a random volume $A$ ranging **from 1 to 100 units**.

> **Important:** If the required condition is not met, the truck waits until the situation changes (e.g., another truck performs the opposite operation). Active waiting (busy-wait) is strictly prohibited.

#### Logging Requirements
All actions must be logged to the console or a file, containing the following details:
* Truck ID;
* Operation type and volume;
* Operation result;
* Current warehouse inventory;
* Occupied dock number.

---

### 3. Implementation Requirements

| # | Requirement |
| :-: | :--- |
| **1** | The application must spawn $M$ truck threads ($M \ge 10$, passed as a parameter). |
| **2** | Each thread must execute exactly $K$ operations ($K \ge 10$, passed as a parameter). |
| **3** | Dock management must be implemented using a `counting-semaphore` with an initial value of $N$. |
| **4** | Warehouse inventory protection (an integer variable) must be implemented using a `mutex`. |
| **5** | Waiting for an impossible operation (not enough space/goods) must utilize a `binary-semaphore` or `std::condition_variable`. Active "busy-wait" is not allowed. |
| **6** | The application must terminate gracefully: all threads finish their work, semaphores and mutexes are released, and the final stock stays within the range $[0, CAP]$. |
| **7** | The program must not encounter a `deadlock` under any combination of parameters $N, M, K$. |
| **8** | The program output must display messages at the start/completion of each operation and show the final warehouse balance. |
| **9** | The source code must be well-formatted (including comments, meaningful variable names, and proper function decomposition). |
| **10** | *(Optional)* Provide the ability to pass $N, M, K$ parameters via command-line arguments. |

## RU | Краткое описание решения

> Для имитации доков был использован семафор `free_docs`. Также для предотвращения одновременной записи использовался общий мьютекс (`mutex`), а для отправки потоков, которые не могут выполнить текущую операцию, созданы две условные переменные: `cond_car_load` и `cond_car_unload`. 
> 
> Сами грузовики реализованы в виде потоков с функцией `car_func`, которая, в свою очередь, псевдослучайно определяет загрузку или разгрузку и значение $A$. В этой же функции отдельно в виде двух условий реализована работа загрузчика и разгрузчика. 
> 
> Загрузчик проверяет, может ли он положить на склад единицы, если может — то кладет и будит разгрузчиков, в противном случае засыпает. Аналогично работают и разгрузчики. 
> 
> Для предотвращения случая взаимной блокировки реализован отдельный поток `manager`, который следит за числом живых потоков и числом спящих из них. Если все засыпают, `manager` «медианизирует» объем склада и будит все потоки.

---

## EN | Brief Solution Description

> To simulate docks, the `free_docs` semaphore was used. Also, to prevent simultaneous writing, a shared mutex (`mutex`) was used, and to send threads that cannot perform the current operation, two condition variables were created: `cond_car_load` and `cond_car_unload`.
> 
> The trucks themselves are implemented as threads with the `car_func` function, which, in turn, pseudo-randomly determines loading or unloading and the value of $A$. In the same function, the work of the loader and unloader is implemented separately in the form of two conditions.
> 
> The loader checks if it can put units into the warehouse; if it can, it puts them and wakes up the unloaders, otherwise it goes to sleep. The unloaders work similarly.
> 
> To prevent a deadlock case, a separate `manager` thread is implemented, which monitors the number of alive threads and the number of sleeping ones among them. If everyone falls asleep, the `manager` "medianizes" the warehouse volume and wakes up all threads.

## RU | Инструкция по запуску решения

> В репозитории представлен Makefile для сборки. Достаточно скачать проект на Linux, выполнить команду `make` и запустить программу `./solution`, которую соберёт Makefile. Параметры $N, M, K$ можно изменить непосредственно в самом коде, там они определяются директивой `#define`.

---

## EN | Instructions for Running the Solution

> The repository provides a Makefile for building the project. It is sufficient to download the project on Linux, run the `make` command, and execute the `./solution` program generated by the Makefile. The parameters $N, M, K$ can be modified directly within the code, where they are defined using the `#define` directive.
