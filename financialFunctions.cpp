#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include "financialFunctions.h"
#include <boost/math/distributions/normal.hpp>
#include <iostream>

using namespace std;

double calculateD1(double S, double K, double T, double r, double v) {
    return (log(S/K) + ((r + 0.5 * v * v)) * T) / (v * sqrt(T));
    
}

// Black-Scholes Call Price
double BSPrice (double S, double K, double r, double T, double v){ 
  // Calcula el precio de una opción europea de tipo call usando el modelo Black-Scholes.

    // Parámetros:
    // - S: precio actual del activo subyacente
    // - K: precio de ejercicio de la opción
    // - T: tiempo hasta el vencimiento de la opción (en años)
    // - r: tasa de interés libre de riesgo anual
    // - v: volatilidad del activo subyacente
  boost::math::normal dist(0.0, 1.0);
  double d = calculateD1(S, K, T, r, v);
              
  double callBoost = S * cdf(dist, d) - exp(-r * T) * K * cdf(dist, d - v * sqrt (T));
  return callBoost;
}

// Función para calcular la volatilidad implícita de una opción usando el método de Newton-Raphson.
double NewtonRaphsonBSV(double S, double K, double T, double r, double realOptionPrice) {


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

        // Calcular la diferencia entre el precio de la opción calculado y el real.
        double  optionPriceDiff = BSPrice(S, K, r, T, vol) - realOptionPrice;;
        // callPriceBlackScholes(S, K, T, r, vol)

        // Calcular Vega, que es la derivada del precio de la opción con respecto a la volatilidad.
        double vega  = S * pdf(dist, d1) * sqrt(T);
        // Actualizar la volatilidad implícita usando el método de Newton-Raphson.
        vol = -optionPriceDiff/vega + vol;
        
        epsilon = abs(optionPriceDiff);
    }
    // Asegurarse de que la volatilidad implícita no sea negativa.
    if (vol < 0 || isnan(vol)){
        vol = 0;
    }
    cout << "res: " << vol << endl << endl;
    return vol;
}

// Bisection Algorithm
double BisecBSV(double S, double K, double T, double r, double MktPrice) {
    double a = 0.00000001;	  	  // Valor inicial para la volatilidad inferior en el algoritmo de bisección
    double b = 30.0;			        // Valor inicial para la volatilidad superior en el algoritmo de bisección
    double q = 0.0;               // Tasa de dividendos (en este caso, cero)
    const int MaxIter = 50000;    // Número máximo de iteraciones permitidas
    double Tol = 0.0000001;       // Tolerancia para la convergencia del algoritmo
    double midP = 0.0, midCdif;


    // // Diferencia entre el precio de mercado y el precio teórico de la opción con volatilidad 'a' y 'b'
    double  lowCdif = MktPrice - BSPrice(S, K, r, T, a);   
    double highCdif = MktPrice - BSPrice(S, K, r, T, b);
   
    // Verificar si el signo de la diferencia de precios es el mismo para los valores iniciales, en cuyo caso no hay raíz
    if (lowCdif*highCdif > 0){
        return -1;
    }
    else{
        for (int i = 0; i <= MaxIter; i++) {
            midP = (a + b) / 2.0;                                           // Calcular el promedio entre 'a' y 'b'
            midCdif = MktPrice - BSPrice(S, K, r, T, midP);     // Diferencia con el precio teórico y el promedio;
            if (abs(midCdif)<Tol) {
                return midP;
            }
            else {
                if (midCdif>0) 
                  a = midP;
                else 
                  b = midP;
            }
            
        }
    }
    return midP;                    // Devolver la última estimación de la volatilidad histórica
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




