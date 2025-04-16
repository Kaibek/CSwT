#include <stdio.h>
#include <stdlib.h>


#define N                   5                                               // Выборачный макс. узлов
#define NTR_DG              1.0                                             // Величина диагонали для нат. сплайна
#define ZD                  0.0                                             // Величина для наддиагональ и поддиагональ для нат. сплайна
#define cr_st               0                                               // начало координат



typedef struct
{
    double                     a, b, k, d;                                  // Коэффициенты полинома: a + b*(x-xi) + k*(x-xi)^2 + d*(x-xi)^3
    double                     x;                                           // Левая граница интервала
} Spline;



// Решение трёхдиагональной системы методом прогонки или алгоритм Томаса
static int tridiagonal_solve(int n, double a[], double b[], double c[], double d[], double k[])
{
    if (n <= 0 || n > N || !b || !c || !d || !k) return -1;
    if (N == 0) return -1;

    double c_prime[N];
    double d_prime[N];

    c_prime[cr_st] = c[cr_st] / b[cr_st];
    d_prime[cr_st] = d[cr_st] / b[cr_st];

    for (int i = 1; i < n - 1; i++)
    {
        double m = b[i] - a[i] * c_prime[i - 1];
        if (m == 0.0)
            return -1; // Деление на ноль

        c_prime[i] = c[i] / m;
        d_prime[i] = (d[i] - a[i] * d_prime[i - 1]) / m;
    }

    double m = b[n - 1] - a[n - 1] * c_prime[n - 2];
    if (m == 0.0) return -1;

    d_prime[n - 1] = (d[n - 1] - a[n - 1] * d_prime[n - 2]) / (b[n - 1] - a[n - 1] * c_prime[n - 2]);

    // Обратный ход
    k[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= cr_st; i--)
        k[i] = d_prime[i] - c_prime[i] * k[i + 1];

    return 0;
}



// Построение кубического сплайна
static int build_cubic_spline(const double x_vals[], const double y[],int n, Spline splines[])
{
    if (n <= 1 || n > N || !x_vals || !y || !splines)
        return -1;

    for (int i = 1; i < n; i++) 
        if (x_vals[i] <= x_vals[i - 1]) return -1;                          // x_vals должны быть строго возрастающими
    


    double h[N - 1];                                                        // Шаги
    double a[N] = { 0 }, b[N] = { 0 };                                      // Поддиагональ,  Диагональ,
    double c[N] = { 0 }, d[N] = { 0 };                                      // Наддиагональ, Правая часть
    double k[N];                                                            // Вторые производные

    // Вычисление шагов
    for (int i = 0; i < N - 1; i++)
        h[i] = x_vals[i + 1] - x_vals[i];

    // Формируем трехдиагональную систему, т.к. нужно задать граничные узлы, чтобы система была полной
    b[cr_st] = NTR_DG;
    d[cr_st] = ZD;
    b[N - 1] = NTR_DG;
    d[N - 1] = ZD;

    // Вычисляем для внутренних узлов
    for (int i = 1; i < N - 1; i++)
    {
        a[i] = h[i - 1];
        b[i] = 2 * (h[i - 1] + h[i]);
        c[i] = h[i];
        d[i] = 3 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]);
    }

    int ret = tridiagonal_solve(n, a, b, c, d, k);
    if (ret) return ret;

    // Вычисляем коэффициенты сплайна
    for (int i = cr_st; i < N - 1; i++)
    {
        Spline* tmp = &splines[i];
        tmp->a = y[i];
        tmp->b = (y[i + 1] - y[i]) / h[i] - h[i] * (k[i + 1] + 2 * k[i]) / 3;
        tmp->k = k[i];
        tmp->d = (k[i + 1] - k[i]) / (3 * h[i]);
        tmp->x = x_vals[i];
    }


    // Последний сплайн не используется, но заполним для полноты и корректности 
    Spline* tmp = &splines[N - 1];
    tmp->a = y[N - 1];
    tmp->b = ZD;
    tmp->k = ZD;
    tmp->d = ZD;
    tmp->x = x_vals[N - 1];
    
    return 0;
}



static int binary_search(Spline splines[], double x, int n)
{
    if (n <= 0 || n > N || !splines) return -1;

    int left = 0, right = n - 1;
    while (left <= right) 
    {
        int mid = (left + right) / 2;
        if (x >= splines[mid].x && (mid == n - 1 || x <= splines[mid + 1].x))
            return mid;
        
        if (x < splines[mid].x)
            right = mid - 1;
            
        else
            left = mid + 1;
    }
    
    return n - 1; // Граничный случай
}



// Интерполяция значения или вычисление значения сплайна в точке x
static int interpolate(Spline splines[], double x, int n, double* result)
{
    if (n <= 0 || n > N || !splines || !result) return -1;
    int i = binary_search(splines, x, n);
    
    if (i < 0) return i;

    if (x < splines[0].x) 
        *result = splines[0].a;

    if (x > splines[n - 1].x) 
        *result = splines[n - 1].a;

    
    Spline* tmp = &splines[i];
    double dx = x - tmp->x;
    
    *result = tmp->a + tmp->b * dx + tmp->k * dx * dx + tmp->d * dx * dx * dx;
    
    return 0;
}
