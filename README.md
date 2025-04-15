# CSwT
Cubic spline with Thomas algorithm. Experimental algorithm for CUBIC as an auxiliary module for predicting RTT trends for a congestion window.

RUS:
Алгоритм обеспечивает эффективную реализацию интерполяции кубического сплайна с использованием алгоритма Томаса (алгоритм трехдиагональной матрицы или метод прогонки) без динамического выделения памяти. Основная идея была, интегрировать в CUBIC как вспомогательный модуль для прогнозирования изменений RTT для адаптивного масштабирования в окнах перегрузки cwnd.

CSTB.c использует статистические данные, без выделения памяти.
CSTB_malloc.c как понятно уже, использует выделение памяти для динамических данных. 



ENG:
The algorithm provides an efficient implementation of cubic spline interpolation using the Thomas algorithm (tridiagonal matrix algorithm or sweep method) without dynamic memory allocation. The main idea was to integrate into CUBIC as an auxiliary module for predicting RTT changes for adaptive scaling in cwnd congestion windows.

CSTB.c uses statistical data, without memory allocation.
CSTB_malloc.c, on the other hand, uses memory allocation for dynamic data.


By: kalimollayev Bekzhan
