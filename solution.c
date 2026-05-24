#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>

#define CAP 1000 	// максимальная емкость склада
#define N 5  		// количество доков
#define M 12 		// количество грузовиков
#define K 12 		// количество операций, для 1 грузовика
	  
int sleeping_cars = 0;						// количество спящих грузовиков, используется менеджером процессов, чтобы предотвратить взаимоблокировку
int active_cars = 0; 						// количество живых грузовиков, тоже нужно для менеджера
int current_inventory = 0;					// текущий объем склада

// Мьютекс
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; 		// мьютекс скалада

// Условные переменные
pthread_cond_t cond_car_load = PTHREAD_COND_INITIALIZER;	// условная переменная загрузчика
pthread_cond_t cond_car_unload = PTHREAD_COND_INITIALIZER;	// условная переменная разгрузчика

// Семафор
sem_t free_docs; // Считает число свободных доков. Изначально доков = N



void* car_func(void* arg) {
	int current_operation_number = K;
	int type_of_operation; // тип операции: загрузка || разгрузка
	int A; 		      // размер операции (объём)
			     
	pthread_mutex_lock(&mutex);
	active_cars++; 		// грузовик ожил
	pthread_mutex_unlock(&mutex);
	while (current_operation_number > 0) {
			// сгенерируем псевдослучайную
			// величину, чтобы определить загрузка || разгрузка
			type_of_operation = rand() % 2; // 1 - загрузка || 0 - разгрузка
			A = (rand() % 100) + 1;		// псевдослучайная величина от 1 до 100
			
			// логика работы, если грузовик решил загружаться
			if (type_of_operation) {
				// зацикливание, чтобы в случае вызова, через условную переменную,
				// при ситуации: разгрузчик кинул после своей работы броадкаст,
				// а разгрузил все равно мало, если бы был if, грузовичек просто бы
				// не выполнил свою n-ую(n не относится ни к какими переменным)
				// операцию
				while (1) {
					// грузовик пытается заехать в док
					sem_wait(&free_docs);

					// захватываем мьютекс скалад
					pthread_mutex_lock(&mutex);

					// условия совершения загрузки и выхода из цикла
					if (current_inventory + A <= CAP) {
						current_inventory += A; // загрузка на склад
						printf("[Загрузчик] успешно загрузил %d. На складе %d\n", A, current_inventory);
						// Будим разгрузчиков (всех)
						pthread_cond_broadcast(&cond_car_unload);

						// отпускаем мьютекс и освобождаем док
						pthread_mutex_unlock(&mutex);
						sem_post(&free_docs);
						break;
					}

					// Грузовик не выщел из цикла, значит 
					// на складе нет места
					printf("[Загрузчик] не смог найти место для %d (на складе %d). Он отъехал от дока в ожидании ...", A, current_inventory);

					// Освобождаем док
					pthread_mutex_unlock(&mutex);
					sem_post(&free_docs);

					// отправляем загрузчик в сон
					pthread_mutex_lock(&mutex);
					sleeping_cars++;
					pthread_cond_wait(&cond_car_load, &mutex);
					sleeping_cars--;
					pthread_mutex_unlock(&mutex);
					// далее он снова попбробуем загрузить склад
				}
			}
		       	// логика работы, если грузовик решил разгружаться	
			else {
				// тут логика аналогична, разгрузчик будет пытаться разгрузить,
				// пока на складе не появится нормальный объем
				while (1) {
					// разгрузчик пытается занять док
					sem_wait(&free_docs);
					
					// захват мьютекса
					pthread_mutex_lock(&mutex);

					// условие корректной разгрузки и выхода из цикла
					if (current_inventory >= A) {
						// разгрузка склада
						current_inventory -= A;
						printf("[Разгручик] разгрузил %d, на складе: %d\n", A, current_inventory);

						// будим загрузчиков
						pthread_cond_broadcast(&cond_car_load);

						// разгручик покидает док
						pthread_mutex_unlock(&mutex);
						sem_post(&free_docs);
						break;
					}

					// на складе недостаточно единиц для разгрузки
					printf("[Разгрузчик] не смог разгрузить %d. (на складе %d) он уехал ожидать ...\n", A, current_inventory);

					// разгрузчик освободает док
					pthread_mutex_unlock(&mutex);
					sem_post(&free_docs);

					// разгрузчик отправляется спать
					pthread_mutex_lock(&mutex);
					sleeping_cars++;
					pthread_cond_wait(&cond_car_unload, &mutex);
					sleeping_cars--;
					pthread_mutex_unlock(&mutex);
					// далее он повторяет попытки разгрузки					
				}
			}
		current_operation_number -= 1;
	}
	pthread_mutex_lock(&mutex);
	active_cars--; // грузовик умер
	pthread_mutex_unlock(&mutex);
	return NULL;
}

// Функция менеджера потоков
// он нужен на случай если все грузовики уснут
void* manager_func(void* arg) {
	while (1) {
		// проверяет раз в 1 секунду
		sleep(1);

		pthread_mutex_lock(&mutex);
		
		// проверка сна всех потоков
		if (sleeping_cars == active_cars) {
			printf("[Менеджер] Все грузовики заснули, менеджер решает вопрос ...\n");
			printf("[Менеджер] принудительно установит 500 единиц, на складе было %d\n", current_inventory);
			// принудительная "медианизация" объема
			current_inventory = 500;

			// менеджер будет всех
			pthread_cond_broadcast(&cond_car_load);
			pthread_cond_broadcast(&cond_car_unload);
		}
		// отпускаем мьютекс
		pthread_mutex_unlock(&mutex);
	}
	return NULL;
}

int main() {
	// M потоков-грузовиков (грузовики приехали)
	pthread_t cars[M];
	// менеджер потоков
	pthread_t manager;

	// инициализация семафора дока
	sem_init(&free_docs, 0, N); // N - доков
				   
	printf("Грузовики приехали ...\n");
	// создание M потоков грузовиков
	for (int i = 0; i < M; i++) {
		pthread_create(&cars[i], NULL, car_func, NULL);
	}
	// создание менеджера потоков
	pthread_create(&manager, NULL, manager_func, NULL);

	// заставляем функцию main ждать, пока грузовики закончат работу
	for (int i = 0; i < M; i++) {
		pthread_join(cars[i], NULL);
	}
	
	// убиваем менеджера
	pthread_detach(manager);

	return 0;
}
