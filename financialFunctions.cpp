#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "financialFunctions.h"
#include <boost/math/distributions/normal.hpp>


using namespace std;

double calculateD1(double S, double K, double T, double r, double sigma) {
    return 1 / (sigma * sqrt(T)) * ( log(S/K) + (r + pow(sigma, 2)/2) * T);
}

double calculateD2(double d1, double T, double sigma){
    return d1 - (sigma * sqrt(T));
}

double callPriceBlackScholes(double S, double K, double T, double r, double sigma, double d1, double d2) {
    // Crear una distribución normal estándar para el cálculo de precios.
    boost::math::normal dist(0.0, 1.0);
    // Calcula el precio de la opción call usando la fórmula de Black-Scholes.
    double C = cdf(dist, d1) * S -  cdf(dist, d2) * K * exp(-r * T);
    return C;
}

double callPriceBlackScholes(double S, double K, double T, double r, double sigma) {
    // Calcula el precio de una opción europea de tipo call usando el modelo Black-Scholes.

    // Parámetros:
    // - S: precio actual del activo subyacente: underlyng_price
    // - K: precio de ejercicio de la opción: strike?
    // - T: tiempo hasta el vencimiento de la opción (en años) : ???
    // - r: tasa de interés libre de riesgo anual: 100%?
    // - sigma: volatilidad del activo subyacente

    // Retorna:
    // - Precio de la opción
    double d1 = calculateD1(S, K, T, r, sigma);
    double d2 = calculateD2(S, T, sigma);
    
    return callPriceBlackScholes(S, K, T, r, sigma, d1, d2);
}

// Función para calcular la volatilidad implícita de una opción usando el método de Newton-Raphson.
double impliedVolatilityNewtonRaphson(double S, double K, double T, double r, double realOptionPrice) {


    boost::math::normal dist(0.0, 1.0);
    double vol = 0.5;  // Valor inicial arbitrario
    
    // Inicializar criterios de convergencia.
    double epsilon = 1.0; // Diferencia entre precio calculado y real.
    double abstol = 1e-4; // Tolerancia absoluta para la convercencia.
    int i = 0;
    int max_iter = 100;

    while (epsilon > abstol) {
        if (i > max_iter) {
            break;
        }
        
        i++;
        double orig = vol;

        // Calcular d1 y d2 usando el modelo Black-Scholes.
        double d1 = calculateD1(S, K, T, r, vol);
        double d2 = calculateD2(d1, T, vol);

        // Calcular la diferencia entre el precio de la opción calculado y el real.
        double  optionPriceDiff = callPriceBlackScholes(S, K, T, r, vol, d1, d2) - realOptionPrice;;


        // Calcular Vega, que es la derivada del precio de la opción con respecto a la volatilidad.
        double vega  = S * pdf(dist, d1) * sqrt(T);
        // Actualizar la volatilidad implícita usando el método de Newton-Raphson.
        vol = -optionPriceDiff/vega + vol;

        epsilon = abs(optionPriceDiff);
    }
    // Asegurarse de que la volatilidad implícita no sea negativa.
    if (vol < 0){
        vol = 0;
    }
    return vol;
}
// Función para calcular la desviación estándar de un conjunto de valores.
double standarDev(vector<double> v){
  // Calcular la suma de todos los elementos del vector.
    double sum = std::accumulate(v.begin(), v.end(), 0.0);
    // Calcular la media (promedio) de los valores del vector.

    double mean = sum / v.size();
 // Crear un nuevo vector para almacenar las diferencias entre cada elemento y la media.
    std::vector<double> diff(v.size());

    // Transformar el vector original restando la media de cada elemento.
    // La función lambda [mean](double x) { return x - mean; } se aplica a cada elemento.
    std::transform(v.begin(), v.end(), diff.begin(), [mean](double x) { return x - mean; });

    // Calcular la suma de cuadrados de las diferencias.
    double sq_sum = std::inner_product(diff.begin(), diff.end(), diff.begin(), 0.0);

     // Calcular la desviación estándar como la raíz cuadrada de la media de la suma de cuadrados.
    double stdev = std::sqrt(sq_sum / v.size());
    return stdev;
}

double f (double x)
{
  double pi = 4.0 * atan (1.0);
  return exp (-x * x * 0.5) / sqrt (2 * pi);
}

// Boole's Rule
double Boole (double StartPoint, double EndPoint, int n)
{
  vector < double >X (n + 1, 0.0);
  vector < double >Y (n + 1, 0.0);
  double delta_x = (EndPoint - StartPoint) / double (n);
  for (int i = 0; i <= n; i++)
    {
      X[i] = StartPoint + i * delta_x;
      Y[i] = f (X[i]);
    }
  double sum = 0;
  for (int t = 0; t <= (n - 1) / 4; t++)
    {
      int ind = 4 * t;
      sum +=
	(1 / 45.0) * (14 * Y[ind] + 64 * Y[ind + 1] + 24 * Y[ind + 2] +
		      64 * Y[ind + 3] + 14 * Y[ind + 4]) * delta_x;
    }
  return sum;
}

// N(0,1) cdf by Boole's Rule
double N (double x)
{
  return Boole (-10.0, x, 240);
}

// Black-Scholes Call Price
double BSPrice (double S, double K, double r, double T, double q, double v, char PutCall)
{
  
  double d = (log (S / K) + T * (r - q + 0.5 * v * v)) / (v * sqrt (T));
  double call = S *exp(-q*T)* N (d) - exp (-r * T) * K * N (d - v * sqrt (T));
  if (PutCall == 'C')
    return call;
  else
  // Put Parity
    return call - S*exp (-q * T) + K * exp (-r * T);
}

// Bisection Algorithm
double BisecBSV(double S, double K, double T, double r, double MktPrice) {
    double a = 0.00000001;		// Bisection algorithm starting value for lower volatility
    double b = 7.0;			// Bisection algorithm starting value for higher volatility
    double q = 0.0;
    char PutCall = 'C';
    const int MaxIter = 50000;
    double Tol = 0.0000001;
    double midP = 0.0, midCdif;
    double  lowCdif = MktPrice - BSPrice(S, K, r, T, q, a, PutCall);
    double highCdif = MktPrice - BSPrice(S, K, r, T, q, b, PutCall);
    if (lowCdif*highCdif > 0)
        return -1;
    else
        for (int i = 0; i <= MaxIter; i++) {
            midP = (a + b) / 2.0;
            midCdif = MktPrice - BSPrice(S, K, r, T, q, midP, PutCall);
            if (abs(midCdif)<Tol) {
                return midP;
            }
            else {
                if (midCdif>0) a = midP;
                else b = midP;
            }
        }
    return midP;
}
