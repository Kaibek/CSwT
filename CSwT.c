#include <stdio.h>

#define N		5							// Выборачный макс. узлов

typedef double db;

typedef struct
{
	db a, b, c, d, k;							// Коэффициенты полинома: a + b*(x-xi) + k*(x-xi)^2 + d*(x-xi)^3
	db x;									// Левая граница интервала
}Spline;



// Решение трёхдиагональной системы методом прогонки или алгоритм Томаса
static int tridiagonal_solve(int n, db a[], db b[], db c[], db d[], db k[])
{
	if (!n || n > N || !a || !b || !c || !d || !k) return -1;

	db a_prime[N];
	db b_prime[N];

	a_prime[0] = c[0] / b[0];
	b_prime[0] = d[0] / b[0];

	// Прямой ход
	for (int i = 1; i < n - 1; i++)
	{
		db y_mid = b[i] - a[i] * a_prime[i - 1];
		if (y_mid < 0) return -2;

		a_prime[i] = c[i] / y_mid;
		b_prime[i] = (d[i] - a[i] * b_prime[i - 1]) / y_mid;
	}

	db y_mid = b[n - 1] - a[n - 1] * a_prime[n - 2];
	if (y_mid < 0) return -3;

	b_prime[n - 1] = (d[n - 1] - a[n - 1] * b_prime[n - 2]) / y_mid;

	// Обратный ход
	k[n - 1] = b_prime[n - 1];
	for (int i = n - 2; i >= 0; i--)
		k[i] = b_prime[i] - a_prime[i] * k[i + 1];

	return 0;
}


// Построение кубического сплайна
static int build_cubic_spline(int n, db x_vals[], db y[], Spline splines[])
{
	if (n <= 1 || n > N || !x_vals || !y) return -14;

	for (int i = 0; i < n - 1; i++)
		if (x_vals[i] > x_vals[i + 1]) return -15;

	db  h[N - 1];								// Шаги
	db	a[N] = { 0.0 }, b[N] = { 0.0 };					// Поддиагональ,  Диагональ,
	db	c[N] = { 0.0 }, d[N] = { 0.0 };					// Наддиагональ, Правая часть
	db  k[N];								// Вторые производные

	// Вычисление шагов
	for (int i = 0; i < n - 1; i++)
	{
		h[i] = x_vals[i + 1] - x_vals[i];
		if (h[i] == 0.0) return -16;
	}
	// Формируем трехдиагональную систему, т.к. нужно задать граничные узлы, чтобы система была полной
	b[0] = 1.0;
	d[0] = 0.0;
	b[n - 1] = 1.0;
	d[n - 1] = 0.0;

	// Вычисляем для внутренних узлов
	for (int i = 1; i < n - 1; i++)
	{
		a[i] = h[i - 1];
		c[i] = h[i];

		b[i] = 2 * (a[i] + c[i]);
		d[i] = 3 * ((y[i + 1] - y[i]) / c[i] - (y[i] - y[i - 1]) / a[i]);

	}


	int ret = tridiagonal_solve(n, a, b, c, d, k);
	if (ret) return ret;

	// Вычисляем коэффициенты сплайна
	for (int i = 0; i < n - 1; i++)
	{
		Spline* tmp = &splines[i];
		tmp->a = y[i];
		tmp->b = (y[i + 1] - y[i]) / h[i] - h[i] * (k[i + 1] + 2 * k[i]) / 3;
		tmp->k = k[i];
		tmp->d = (k[i + 1] - k[i]) / (3 * h[i]);
		tmp->x = x_vals[i];
	}

	// Последний сплайн не используется, но заполним для полноты и корректности 
	Spline* tmp = &splines[n - 1];
	tmp->a = y[n - 1];
	tmp->b = 0.0;
	tmp->k = 0.0;
	tmp->d = 0.0;
	tmp->x = x_vals[n - 1];

	return 0;
}



static int binary_search(Spline splines[], double x, int n)
{
	if (n <= 0 || n > N || !splines) return -17;

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
	if (n <= 0 || n > N || !splines || !result) return -18;

	if (x < splines[0].x)
	{
		*result = splines[0].a;
		return 0;
	}

	if (x > splines[n - 1].x)
	{
		*result = splines[n - 1].a;
		return 0;
	}

	int i = binary_search(splines, x, n);
	if (i < 0) return i;

	Spline* tmp = &splines[i];
	double dx = x - tmp->x;

	*result = tmp->a + tmp->b * dx + tmp->k * dx * dx + tmp->d * dx * dx * dx;

	return 0;
}


int main()
{
	db x_vals[N] = { 1, 2, 3, 4, 5 };
	db y_vals[N] = { 120.0, 130.0, 110.0, 110.0, 120.0 };
	Spline splines[N];

	if (build_cubic_spline(N, x_vals, y_vals, splines) != 0)
	{
		printf("Error while constructing spline.\n");
		return -19;
	}

	for (double x = 1; x <= 5; x += 0.5)
	{
		double result = 0;
		if (interpolate(splines, x, N, &result) == 0)
			printf("S(%g) = %g\n", x, result);
		else
			printf("Interpolation error at point %g\n", x);
	}

	return 0;
}

////////////////////////////////
//Output:
//S(1) = 120 => S(1.5) = 128.482

//S(2) = 130 => S(2.5) = 120.804

//S(3) = 110 => S(3.5) = 107.054

//S(4) = 110 => S(4.5) = 114.732

//S(5) = 120
////////////////////////////////

