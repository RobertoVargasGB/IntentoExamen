#ifndef MATRIZ_H
#define MATRIZ_H

#define LARGO 3

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

class Matriz {
public:
    /**
     * Constructor Por defecto inicializa cada dato de la matriz con un valor aleatoreo.
     */
    Matriz();

    /**
     * Constructor que toma una línea y la transforma en un objeto matriz.
     * @param orig String con el formato de definido en la tarea.
     */
    Matriz(const std::string& orig);

    /**
     * Constructor que copia una matriz.
     * @param orig Matriz original.
     */
    Matriz(const Matriz& orig);

    /**
     * Destructor virtual
     */
    virtual ~Matriz();

    /**
     * Determina si el cuadrado es mágico o no lo es.
     * @return 
     */
    bool esMagico();

    /**
     * 
     * @return Un string con el formato definido.
     */
    std::string toString();
private:
    int** matriz;
};

#endif /* MATRIZ_H */

