# Entrega Tarea Tecnica
La tarea técnica requería calcular tanto la volatilidad histórica como la implícita de una opción financiera. Para ello, se dispuso de una serie temporal completa con todos los datos necesarios.

## Voltilidad Implicita
Para el cálculo de la volatilidad implícita, se emplearon dos métodos: Newton-Raphson y bisección. El método de Newton-Raphson es más rápido que el de bisección, aunque puede presentar fallos en ciertas situaciones. Por esta razón, también se implementó el método de bisección, dada su mayor robustez.

El "time to maturity" se determinó mediante la diferencia entre la fecha en la que se cotizó la opción y su fecha de vencimiento, posteriormente anualizando este resultado.

Con una tasa de interes libre de riesgo constante = 100%, se tomo r = 1.00


## Volatilidad historica
Para la volatilidad histórica, se calculó la desviación estándar de los rendimientos de las últimas 60 observaciones. Los rendimientos se obtuvieron utilizando la fórmula ln(precio_opcion_i / precio_opcion_i-1).

Este método presenta un inconveniente, ya que los valores no mantienen la misma distancia temporal entre ellos. Una solución viable es la interpolación de valores para crear una muestra más uniforme, permitiendo un cálculo más preciso de la volatilidad histórica.

## Ejecucion
Para ejecutar el código, basta con correr el siguiente comando:

```bash
make run
```

Dependencias: Es esencial tener instaladas las bibliotecas:[Root](https://root.cern/) y [Boost](https://www.boost.org/).

