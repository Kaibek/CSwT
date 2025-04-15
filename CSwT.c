#include <stdio.h>

#define N 5                                                             // макс. узлов

typedef struct 
{
    double a, b, k, d;                                                  // Коэффициенты полинома: a + b*(x-xi) + k*(x-xi)^2 + d*(x-xi)^3
    double x;                                                           // Левая граница интервала
} Spline;




// Решение трёхдиагональной системы методом прогонки или алгоритм Томаса
void tridiagonal_solve(int n, double a[], double b[], double c[], double d[], double k[]) 
{
    double c_prime[N];
    double d_prime[N];

    c_prime[0] = c[0] / b[0];
    d_prime[0] = d[0] / b[0];

    for (int i = 1; i < n - 1; i++) 
    {
        double m = b[i] - a[i] * c_prime[i - 1];
        c_prime[i] = c[i] / m;
        d_prime[i] = (d[i] - a[i] * d_prime[i - 1]) / m;
    }

    d_prime[n - 1] = (d[n - 1] - a[n - 1] * d_prime[n - 2]) / (b[n - 1] - a[n - 1] * c_prime[n - 2]);

    // Обратный ход
    k[n - 1] = d_prime[n - 1];
    for (int i = n - 2; i >= 0; i--) 
        k[i] = d_prime[i] - c_prime[i] * k[i + 1];
}



// Построение кубического сплайна
void build_cubic_spline(const double x_vals[], const double y[], Spline splines[]) 
{
    double h[N - 1];                                                    // Шаги
    double a[N] = { 0 }, b[N] = { 0 }, c[N] = { 0 }, d[N] = { 0 };      // Поддиагональ,  Диагональ, Наддиагональ, Правая часть
    double k[N];                                                        // Вторые производные

    for (int i = 0; i < N - 1; i++) 
        h[i] = x_vals[i + 1] - x_vals[i];

    // Формируем трехдиагональную систему, т.к. нужно задать граничные узлы, чтобы система была полной: 
    b[0] = 1.0; 
    d[0] = 0.0;
    b[N - 1] = 1.0;
    d[N - 1] = 0.0;

    // Вычисляем для внутренних узлов
    for (int i = 1; i < N - 1; i++) 
    {
        a[i] = h[i - 1];
        b[i] = 2 * (h[i - 1] + h[i]);
        c[i] = h[i];
        d[i] = 3 * ((y[i + 1] - y[i]) / h[i] - (y[i] - y[i - 1]) / h[i - 1]);
    }

    tridiagonal_solve(N, a, b, c, d, k);

    // Вычисляем коэффициенты сплайна
    for (int i = 0; i < N - 1; i++)
    {
        Spline* tmp = &splines[i];
        tmp->a = y[i];
        tmp->b = (y[i + 1] - y[i]) / h[i] - h[i] * (k[i + 1] + 2 * k[i]) / 3;
        tmp->k = k[i];
        tmp->d = (k[i + 1] - k[i]) / (3 * h[i]);
        tmp->x = x_vals[i];
    }


    // Последний сплайн не используется, но заполним для красоты
    Spline* tmp = &splines[N - 1];
    tmp->a = y[N - 1];
    tmp->b = 0;
    tmp->k = 0;
    tmp->d = 0;
    tmp->x = x_vals[N - 1];
    
}



// Интерполяция значения или вычисление значения сплайна в точке x
double interpolate(const Spline splines[], double x) 
{
    for (int i = 0; i < N - 1; i++) 
    {
        Spline* tmp = &splines[i];
        if (x >= tmp->x && x <= splines[i + 1].x) 
        {
            double dx = x - tmp->x;
            return tmp->a + tmp->b * dx + tmp->k * dx * dx + tmp->d * dx * dx * dx;
        }
    }

    if (x < splines[0].x) return splines[0].a;
    return splines[N - 1].a;
}




// Пример использования
int main() 
{
    double x_vals[N] = { 0.0, 1.0, 2.0, 3.0, 4.0 };
    double y[N] = { 100.0, 120.0, 110.0, 130.0, 125.0 };

    Spline splines[N];
    build_cubic_spline(x_vals, y, splines);

    double test_points[] = { 0.5, 1.5, 2.5, 3.5 };
    for (int i = 0; i < 4; i++) 
    {
        double result = interpolate(splines, test_points[i]);
        printf("x = %.1f, interpolated RTT = %.2f ms\n", test_points[i], result);
    }

    return 0;
}
