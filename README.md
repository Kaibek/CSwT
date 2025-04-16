# CSwT
Cubic spline with Thomas algorithm. Experimental algorithm for CUBIC as an auxiliary module for predicting RTT trends for a congestion window.

RUS:
Алгоритм обеспечивает эффективную реализацию интерполяции кубического сплайна с использованием алгоритма Томаса (алгоритм трехдиагональной матрицы или метод прогонки) без динамического выделения памяти. Основная идея была, интегрировать в CUBIC как вспомогательный модуль для прогнозирования изменений RTT для адаптивного масштабирования в окнах перегрузки cwnd.

CSwT.c использует статистические данные, без выделения памяти.
 


ENG:
The algorithm provides an efficient implementation of cubic spline interpolation using the Thomas algorithm (tridiagonal matrix algorithm or sweep method) without dynamic memory allocation. The main idea was to integrate into CUBIC as an auxiliary module for predicting RTT changes for adaptive scaling in cwnd congestion windows.

CSwT.c uses statistical data, without memory allocation.


By: kalimollayev Bekzhan
