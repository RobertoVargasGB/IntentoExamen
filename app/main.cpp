#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <mpi/mpi.h>
#include "menu.h"
#include "Matriz.h"

int main(int argc, char** argv) {
    int returnCode = EXIT_FAILURE;
    int node = 1;
    MPI::Init(argc, argv);
    MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);

    try {

        if (argc > 4) {
            std::string archivo_entrada(obtener_entrada(argv, argc)); //Se obtiene la ruta del archivo de entrada
            std::string archivo_salida(obtener_salida(argv, argc));   //Se obtiene la ruta delarchivo de salida

            if (!archivo_entrada.empty() && !archivo_salida.empty()) {    //las variables logran ser obtenidas

                std::ifstream entrada(archivo_entrada);   //Se abre el archivo de entrada para leer
                std::ofstream salida(archivo_salida);   //Se abre el archivo de salida para escribir

                int size = MPI::COMM_WORLD.Get_size();    //Se obtiene la cantidad de procesadores
                int rank = MPI::COMM_WORLD.Get_rank();    //Se obtiene el numero del procesador actual
                if (rank == 0) {    // Nodo maestro

                    std::string linea;
                    int flag,flag_x=0;
                    while (std::getline(entrada, linea)) {    //Se obtiene linea a linea del archivo  de entrada
                        if (node >= size) {
                            node = 1;   //obtiene nodos desde 1 hasta size-1
                        }
                        MPI::COMM_WORLD.Send(&flag, 1, MPI::INT, node, 0);  //Se envía una señal para que el procesador continue iterando
                        // Mandamos datos a los demas nodos
                        MPI::COMM_WORLD.Send(linea.c_str(), linea.size()+1, MPI::CHAR, node, 0);    //Para que el string no llegue con basura se debe sumar 1 al tamaño
                        linea.clear();    //Se limpia el string linea

                        //Recibiendo string de matrices magicas
                        MPI::COMM_WORLD.Recv(&flag_x, 1, MPI::INT, node, MPI::ANY_TAG);   //Se recibe una señal, si es 1 quiere decir que viene acompañado de 'buffer'

                        if (flag_x == 1) {
                            char buffer[100];
                            MPI::COMM_WORLD.Recv(buffer, 100, MPI::CHAR, node, MPI::ANY_TAG);   //Se recibe el string de la matriz magica
                            std::string str(buffer);
                            std::cout << buffer << '\n';
                            salida.write(str.c_str(), str.size());    //Se escribe en el archivo de salida
                        }

                        node += 1;
                    }   //Se termina de leer el archivo

                    for (int fuente=1;fuente<size;fuente++){    //Una vez se termina el archivo, es necesario enviar una señal para detener el bucle de los procesadores
                      flag=1;
                      MPI::COMM_WORLD.Send(&flag, 1, MPI::INT, fuente, 0);    //Se envia una señal a cada procesador para que detenga el bucle
                    }

                } else {    //Nodos esclavos
                  int flag=0;
                  while(entrada.is_open()) {    //Mientras el archivo este abierto
                      MPI::COMM_WORLD.Recv(&flag, 1, MPI::INT, 0, MPI::ANY_TAG);    //Se recibe una señal
                      if (flag!=1) {    //Si la señal es distinta a 1, significa que siguen llegando lineas del archivo
                        char buffer[100];
                        MPI::COMM_WORLD.Recv(buffer, 100, MPI::CHAR, 0, MPI::ANY_TAG);
                        std::string datosRecibidos(buffer);
                        std::cout <<"Mi rango es: "<<rank<< " y recibi: " << datosRecibidos << '\n';

                        Matriz matriz;    //Se inicializa la variable matriz
                        matriz.recibir(datosRecibidos);     //Se modela la matriz con los datos recibidos
                        bool ok = matriz.esMagico();    //Se determina si la matriz es magica
                        int flag_x = ok ? 1 : 0;    //Si la matriz es magica el flag_x será 1
                        datosRecibidos.clear();   //Se limpia el string 'datosRecibidos'
                        std::cout << "FLAG: "<<flag_x <<" OK : "<< ok << '\n';
                        MPI::COMM_WORLD.Send(&flag_x, 1, MPI::INT, 0, 0);   //Se envia al nodo maestro flag_x, señal que determina si se recibe buffer

                        if (ok) {   //Si es magica, se enviará la matriz convertida a string
                            MPI::COMM_WORLD.Send(matriz.toString().c_str(), matriz.toString().size()+1, MPI::CHAR, 0, 0);   //Se envia la la matriz magica como un string
                        }
                      }
                      else{   //De lo contrario, significa que ya no llegarán más lineas
                        entrada.close();    //Se cierra el archivo
                      }
                    }
                }

                entrada.close();  //Se cierra el archivo de entrada
                salida.close();   //Se cierra el archivo de salida

                returnCode = EXIT_SUCCESS;
            } else {
                std::cerr << std::endl << "Error al determinar las rutas de los archivos de entrada " << archivo_entrada << " y salida " << archivo_salida << std::endl;
                returnCode = EXIT_FAILURE;
            }
        } else {
            std::cerr << std::endl << "Argumentos insuficientes" << std::endl;
            std::cerr << std::endl << argv[0] << " -f /ruta/archivo/entrada.txt -o /ruta/archivo/salida.txt" << std::endl;
            returnCode = EXIT_FAILURE;
        }

    } catch (MPI::Exception e) {
        std::cout << "MPI ERROR: " << e.Get_error_code() << " - " << e.Get_error_string() << std::endl;
        returnCode = EXIT_FAILURE;
    }


    MPI::Finalize();

    return returnCode;
}
