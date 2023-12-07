#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <sstream>
#include <numeric>
#include <cmath>
#include "date.h"
#include "financialFunctions.h"
#include <boost/math/distributions/normal.hpp>

// librerias para graficar
#include "TCanvas.h"
#include "TGraph.h"
#include "TLegend.h"
#include <TH1F.h>


using namespace std;
using namespace date;

// Struct para representar las columnas del CSV
struct Row {
    string description;
    float strike;
    string kind;
    float bid;
    float ask;
    float underBid;
    float underAsk;
    double optionPrice;
    double underPrice;
    string createdAt;
    
    friend ostream& operator<<(ostream& os, const Row& row);

};
// definimos el operador << para poder imprimir en consola los valores de una fila
ostream& operator<<(ostream& os, const Row& row) {
    os << "Description: " << row.description
       << ", Strike: " << row.strike
       << ", Kind: " << row.kind
       << ", Bid: " << row.bid
       << ", Ask: " << row.ask
       << ", UnderBid: " << row.underBid
       << ", UnderAsk: " << row.underAsk
       << ", OptionPrice: " << row.optionPrice
       << ", UnderPrice: " << row.underPrice
       << ", CreatedAt: " << row.createdAt;
    return os;
 }

// cambiamos las comas por puntos y los '\N' por 0
string cleanNumber(string floatAsString){
    for (char &c : floatAsString) {
            if (c == ',') {
                c = '.';
            }
        }
    if (floatAsString == "\\N"){
            floatAsString = "0";
    }
    return floatAsString;
}


Row convertLineToRow(string line){
    string palabra, description, strike, kind, bid, ask, underBid, underAsk, createdAt;
    stringstream s(line);

    getline(s, palabra, ';');
    description = palabra;
    
    getline(s, palabra, ';');
    strike = palabra;

    getline(s, palabra, ';');
    kind = palabra;

    getline(s, palabra, ';');
    bid = cleanNumber(palabra);
    
    getline(s, palabra, ';');
    ask = cleanNumber(palabra);

    getline(s, palabra, ';');
    underBid = cleanNumber(palabra);

    getline(s, palabra, ';');
    underAsk = cleanNumber(palabra);

    getline(s, palabra, ';');
    createdAt = palabra;
     
    double op = (stof(bid) + stof(ask))/2;
    if (stof(ask) == 0){
        op = stof(bid);
    }
    double up = (stof(underBid) + stof(underAsk))/2;
    Row row{description, stof(strike), kind, stof(bid), stof(ask), stof(underAsk),  stof(underBid), op, up, createdAt};
    return row;
}

vector<Row> readData(string path){
    ifstream file;
    string line;
    vector<Row> data;
    file.open("Exp_Octubre.csv");

    if (!file.is_open()) {
        std::cerr << "Error opening the file" << std::endl;
    }
    
    // leemos la primera linea que tiene los nombres de las columnas
    getline(file, line); 

    int i = 0;
    // iteramos y creamos nuestra lista de rows
    while (getline(file, line)) {
        Row row = convertLineToRow(line);
        data.push_back(row);
        i++;
    } 
    file.close();

    return data;
}

void saveInCSV(const std::vector<Row>& data,
                  const std::vector<double>& implicitVol,
                  const std::vector<double>& historicVol) {
    // Nombre del archivo CSV
    std::string nombreArchivo = "datos.csv";

    // Crear y abrir un archivo en modo de escritura
    std::ofstream archivo(nombreArchivo);
    char delimiter = ';';

    // Comprobar si el archivo está abierto correctamente
    if (!archivo.is_open()) {
        std::cerr << "Error al abrir el archivo." << std::endl;
        return;
    }

    // Escribir los encabezados en el archivo CSV
    archivo << "description;historicVolatolity;implicitVolatility;createdAt\n";

    // Asegurar que todos los vectores tienen el mismo tamaño
    if (!(data.size() == historicVol.size() && data.size() == implicitVol.size())) {
        std::cerr << "Error: Los vectores de salida no tienen el mismo tamaño." << std::endl;
        std::cerr << "data :" << data.size() << " historic: " << historicVol.size() <<  " implicit: " << implicitVol.size() << std::endl;
        return;
    }

    // Escribir datos en el archivo
    for (size_t i = 0; i < data.size(); ++i) {
        archivo << data[i].description << delimiter
                << historicVol[i] << delimiter
                << implicitVol[i] << delimiter
                << data[i].createdAt << "\n";
    }

    // Cerrar el archivo
    archivo.close();
}


auto calculateDateFromString(string date){
    std::istringstream dateStream(date);

    unsigned int d, m , y;
    char delimitador;

    // extrae el mes
    dateStream >> m >> delimitador;

    // extrae el dia
    dateStream >> d >> delimitador;

    // extrae el año
    dateStream >> y;

    auto res = day{d}/m/y;
    return res;

}


vector<double> calculateAllImplicitVolatolity(vector<Row> data){
    vector<double> res;
    // el valor expirationDate entra por dato, esta implicito en el nombre de la opcion.
    auto expirationDate = day{20}/11/2023;
    
    int i = 0;
    for(i = 0; i < data.size(); i++){
            
            // conseguimos la fecha desde el string
            auto currentDate = calculateDateFromString(data[i].createdAt);
            
            // hacemos la diferencia entre la fecha de expiracion con la fecha en la fecha de la opcion 
            int daysToexercise = (sys_days{expirationDate} - sys_days{currentDate}).count();
            // anualizamos la opcion
            double t = (daysToexercise/365.0);
            
            
            // el valor de la tasa de interes es del 100% ponemos r = 1.00
            double r = 1.0;
            //como el modelo de newtown raphson puede tener errores tamb esta implementeado el modelo de la biseccion
            double vol = 0;
            try{

                vol = impliedVolatilityNewtonRaphson(data[i].underPrice, data[i].strike, t,  r, data[i].optionPrice);
                if (vol == 0){
                    vol = BisecBSV(data[i].underPrice, data[i].strike, t,  r, data[i].optionPrice);
                }
            }
            catch(...){
                vol = BisecBSV(data[i].underPrice, data[i].strike, t,  r, data[i].optionPrice);
            }
             if(vol < 0){
                vol = 0;
            }
            res.push_back(vol);
    }
    return res;
}



vector<double> calculateAllHistoricVolatility(vector<Row> data){
    vector<double> res;
    vector<double> underYields;

    // Calcular los rendimientos logarítmicos de los precios del activo subyacente.
    for(int i = 1; i < data.size(); i++){
        underYields.push_back(log(data[i].optionPrice / data[i-1].optionPrice));
    }
    // Calcular la volatilidad histórica para ventanas de los utimos 60 valores.
    // tener en cuenta que esto esta mal, ya que para calcular la volatilidad historica los valores tienen que estar a una distancia en tiempo equidistante
    // esto no sucede, se solucionaria interpolando valores para tapar los baches en el tiempo.
    for(int i = 0; i < data.size(); i++){
        double vol = 0;
        if( i < 60){
            res.push_back(vol);
        } else {
            // Si el índice es menor que 60, se añade una volatilidad de 0.
            if (i < 60){
                res.push_back(vol);
            } else {
                // Seleccionar una subsecuencia de 60 días de rendimientos logarítmicos.
                vector<double> subVec = {underYields.begin() + (i - 60 + 1), underYields.begin() + (+ i + 1 )};
                // Calcular la desviación estándar (volatilidad) de la subsecuencia.
                vol = standarDev(subVec);
                res.push_back(vol);
            }
        }
    }
    return res;
}


void plot(const std::vector<double>& y_values1, const std::vector<double>& y_values2, const std::vector<std::string>& x_labels) {
    int n_points = x_labels.size();
    TGraph *gr1 = new TGraph(n_points);
    TGraph *gr2 = new TGraph(n_points);

    for (int i = 0; i < n_points; i++) {
        double y_value = y_values1[i];
        if(y_value > 0.2){
            y_value = 0.2;
        }
        gr1->SetPoint(i, i, y_value);
        gr2->SetPoint(i, i, y_values2[i]);
    }

    TCanvas *c1 = new TCanvas("c1", "Canvas", 1800, 700);
    gr1->SetLineColor(kBlue);
    gr1->Draw("ALP");

    gr2->SetLineColor(kRed);
    gr2->Draw("LP SAME");

    gr1->GetHistogram()->GetXaxis()->Set(n_points, 0, n_points);
    for (int i = 0; i < n_points; i++) {
        gr1->GetHistogram()->GetXaxis()->SetBinLabel(i+1, x_labels[i].c_str());
    }

    // gr1->GetHistogram()->GetXaxis()->LabelsOption("v");
    TLegend *leg = new TLegend(0.7, 0.7, 0.9, 0.9);
    leg->AddEntry(gr1, "volatilidad implicita", "l");
    leg->AddEntry(gr2, "volatilidad historica", "l");
    leg->Draw();

    c1->SaveAs("volatilidadHistoricaVsImplicita.png");
}

string getDate(string date){
    std::istringstream dateStream(date);

    unsigned int d, m , y;
    char delimitador;

    // extrae el mes
    dateStream >> m >> delimitador;

    // extrae el dia
    dateStream >> d >> delimitador;

    // extrae el año
    dateStream >> y;

    return to_string(d)+"/"+to_string(m);
}

// Transforma un vector de cadenas que representan fechas. Para cada fecha en el vector,
// convierte la fecha en un formato deseado y la añade a un nuevo vector. Si la diferencia
// en días entre una fecha y la anterior es cero, añade una cadena vacía en lugar de la fecha transformada.
// el objetivo es en los dias que cambia la fecha guardar la fecha, en los que no se pone un string vacio
vector<string> TransformDatesWithDiffCheck(vector<string> v){
    vector<string> res;
    res.push_back(getDate(v[0]));
    for(int i = 1; i < v.size(); i++){
        auto d1 = calculateDateFromString(v[i]);
        auto d2 = calculateDateFromString(v[i-1]);
        int a =(sys_days(d2) - sys_days(d1)).count();
        if(a != 0){
            res.push_back(getDate(v[i]));
        } else{
            res.push_back("");
        }
    }

    return res;
}

int main() {

    vector<Row> data;
    vector<double> implicitVol;
    vector<double> historicVol;
    vector<string> dates;
    string path = "Exp_Octubre.csv";
    data = readData(path);
    
    implicitVol = calculateAllImplicitVolatolity(data);
    historicVol = calculateAllHistoricVolatility(data);

    for(int i; i < data.size(); i++){
        dates.push_back(data[i].createdAt);
    }
    dates = TransformDatesWithDiffCheck(dates);
    plot(implicitVol, historicVol, dates);
    saveInCSV(data, implicitVol, historicVol);
    return 0;
}
