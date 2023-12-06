#ifndef FINANCIAL_FUNCTIONS_HPP
#define FINANCIAL_FUNCTIONS_HPP
#include <vector>

double calculateD1(double , double , double , double , double );
double calculateD2(double d1, double T, double sigma);
double callPriceBlackScholes(double S, double K, double T, double r, double sigma, double d1, double d2);
double callPriceBlackScholes(double S, double K, double T, double r, double sigma);
double impliedVolatilityNewtonRaphson(double S, double K, double T, double r, double realOptionPrice);
double standarDev(std::vector<double> v);
double f (double x);
double Boole (double StartPoint, double EndPoint, int n);
double N (double x);
double BSPrice (double S, double K, double r, double T, double q, double v, char PutCall);
double BisecBSV(double S, double K, double T, double r, double MktPrice);




#endif // FINANCIAL_FUNCTIONS_HPP