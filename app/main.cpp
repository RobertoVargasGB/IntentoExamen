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
    //MPI::COMM_WORLD.Set_errhandler(MPI::ERRORS_THROW_EXCEPTIONS);

    try {

        if (argc > 4) { // que los argumentos sean 5
            std::string archivo_entrada(obtener_entrada(argv, argc));
            std::string archivo_salida(obtener_salida(argv, argc));

            if (!archivo_entrada.empty() && !archivo_salida.empty()) {

                std::ifstream entrada(archivo_entrada);
                std::ofstream salida(archivo_salida);

                int size = MPI::COMM_WORLD.Get_size(); // cantidad de procesadores
                int rank = MPI::COMM_WORLD.Get_rank(); // procedor actual
                if (rank == 0) {
                    // Nodo maestro
                    std::string linea;
                    while (std::getline(entrada, linea)) {
                        if (node >= size) { // asegura que la informacion se distribuya de forma coorrelativa a los nodos
                            node = 1;
                        }

                        // Mandamos datos a los demas nodos
                        MPI::COMM_WORLD.Send(linea.c_str(), linea.size(), MPI::CHAR, node-1, 0);
                        linea.clear();

                        node += 1;
                    }
                } else {
                    char buffer[100];
                    MPI::COMM_WORLD.Recv(buffer, 100, MPI::CHAR, 0, MPI::ANY_TAG);
                    std::string datosRecibidos(buffer);

                    Matriz matriz(datosRecibidos);
                    bool ok = matriz.esMagico();
                    int flag = ok ? 1 : 0;
                    MPI::COMM_WORLD.Send(&flag, 1, MPI::INT, 0, 0);
                    if (ok) {
                        MPI::COMM_WORLD.Send(matriz.toString().c_str(), matriz.toString().size(), MPI::CHAR, 0, 0);
                    }
                }

                if (rank == 0) {
                    char buffer[100];
                    MPI::COMM_WORLD.Recv(buffer, 100, MPI::CHAR, 0, MPI::ANY_TAG);
                    std::string str(buffer);
                    salida.write(str.c_str(), str.size());
                }

                entrada.close();
                salida.close();

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
