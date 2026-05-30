// ======================================================================
// MODULO 1: LIBRERIAS Y DEPENDENCIAS
// ======================================================================
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cstring>
#include <iomanip>
#include <cmath>
#include <fstream>
#include <clocale>

#ifdef _WIN32
#include <windows.h>
#endif

#include "ConexionBD.h"     // Asegurate de que este archivo este en el mismo directorio
#include "OperacionesBD.h"  // ? BD — operaciones CRUD sobre MySQL
#include "Tipos.h"

using namespace std;

// ======================================================================
// MODULO 2: CONSTANTES Y MACROS - COLORES ANSI
// ======================================================================
constexpr const char* RESET = "\033[0m";
constexpr const char* ROJO = "\033[31m";
constexpr const char* VERDE = "\033[32m";
constexpr const char* AMARILLO = "\033[33m";
constexpr const char* AZUL = "\033[34m";
constexpr const char* CYAN = "\033[36m";
constexpr const char* BOLD = "\033[1m";

// ======================================================================
// ? BD — Conexion global accesible desde todas las funciones
// ======================================================================
MYSQL* g_con = nullptr;

// ======================================================================
// MODULO 3: ESTRUCTURAS DE DATOS
// ======================================================================
struct Usuario {
    string id;
    string nombreCompleto;
    int    edad = 0;
    string genero;
    string direccion;
    float  peso = 0.0f;
    float  altura = 0.0f;
    string nivelActividad;
    string ocupacion;
    double imc = 0.0;

    int desayunoIdx = -1;
    int almuerzoIdx = -1;
    int cenaIdx = -1;

    bool   yaTieneCaloriasCalculadas = false;
    double caloriasRecomendadas = 0.0;
    double tmbCalculada = 0.0;
    int    nivelActividadSeleccionado = 0;
    int    metaSeleccionada = 0;  // 1: Bajar, 2: Mantener, 3: Aumentar

    // ? BD — ID generado por MySQL al insertar el usuario
    int dbId = -1;
};

// ======================================================================
// MODULO 4: DATOS GLOBALES CONSTANTES
// ======================================================================
const vector<Alimento> listaAlimentos = {
    {"Hamburguesa con queso", 303, 15.0, 30.0, 14.0},
    {"Piza de pepperoni",     290, 12.0, 32.0, 12.0},
    {"Papas fritas",          365,  4.0, 48.0, 17.0},
    {"Nuggets de pollo",      270, 13.0, 16.0, 16.0},
    {"Hot Dog sencillo",      290, 10.0, 24.0, 16.0},
    {"Refresco de cola",      150,  0.0, 39.0,  0.0},
    {"Cerveza clara",         153,  1.6, 13.0,  0.0},
    {"Jugo naranja nat",      110,  2.0, 26.0,  0.2},
    {"Cafe Latte",            120,  6.0, 12.0,  6.0},
    {"Bebida energetica",     110,  0.0, 28.0,  0.0},
    {"Pechuga de pollo",      165, 31.0,  0.0,  3.6},
    {"Carne asar (res)",      250, 26.0,  0.0, 15.0},
    {"Filete de salmon",      208, 20.0,  0.0, 13.0},
    {"Arroz blanco",          130,  2.4, 28.0,  0.3},
    {"Frijoles negros",       132,  9.0, 23.0,  0.5},
    {"Pasta cocida",          158,  5.8, 31.0,  0.9},
    {"Ensalada Cesar",        350, 18.0, 15.0, 24.0},
    {"Sushi Filadelfia",      320,  8.0, 52.0,  8.0}
};

// ======================================================================
// MODULO 5: PROTOTIPOS DE FUNCIONES
// ======================================================================
// 5.1 Utilidades de UI
void limpiar();
void pausa();
void dibujarLinea(int ancho = 70, char c = '=');
void dibujarTitulo(const string& titulo);

// 5.2 Utilidades de strings y validacion
string optimizarEspacios(const string& s);
string aMinusculas(const string& s);
bool   validarDireccion(const string& dir, string& mensajeError);
bool   validarPesoEdad(float peso, int anios, string& mensajeError);
void   evaluacionPesoYMensaje(int edad, double imc);

// 5.3 Menus principales
void pantallaBienvenida();
void mostrarMenuPrincipal(int& opcion);
void subMenuUsuario(Usuario& u);
void menuNutricion(int& opcion, const Usuario& u);

// 5.4 Modulos funcionales de nutricion
void registroDietaDiaria(Usuario& u);
void estadoNutricional(Usuario& u);
void historialYReporte(Usuario& u);

// 5.5 Gestion de usuarios
void registrarNuevoUsuario(vector<Usuario>& usuarios);
void cargarPerfilExistente(vector<Usuario>& usuarios);
void listarUsuariosRegistrados(const vector<Usuario>& usuarios);
void editarEliminarPerfil(vector<Usuario>& usuarios);

// ======================================================================
// MODULO 6: FUNCION MAIN
// ======================================================================
int main() {
    setlocale(LC_ALL, "es_ES.UTF-8");
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif

    vector<Usuario> usuariosRegistrados;

    // ? BD — abrir conexion y guardar puntero global
    ConexionBD cn;
    cn.abrir_conexion();
    g_con = cn.get_conexion();

    if (g_con) {
        cout << VERDE << "----------------------------------------------------------------------------------------------------------------------." << RESET << "\n";
        cout << VERDE << "\t\t\t\t[OK] DB Connection" << RESET << "\n";
        cout << VERDE << "----------------------------------------------------------------------------------------------------------------------." << RESET << "\n";
    }
    else {
        cout << ROJO << "[ERROR] Error connecting to the DB." << RESET << "\n";
    }

    pantallaBienvenida();

    int opcionPrincipal = 0;
    do {
        mostrarMenuPrincipal(opcionPrincipal);

        switch (opcionPrincipal) {
        case 1: registrarNuevoUsuario(usuariosRegistrados);      break;
        case 2: cargarPerfilExistente(usuariosRegistrados);      break;
        case 3: listarUsuariosRegistrados(usuariosRegistrados);  break;
        case 4: editarEliminarPerfil(usuariosRegistrados);       break;
        case 5:
            dibujarTitulo("SALIENDO DEL SISTEMA");
            cout << "Gracias por usar el Sistema de Gestion Nutricional 2026\n";
            break;
        default:
            cout << ROJO << "\nOpcion invalida.\n" << RESET;
            pausa();
            break;
        }
    } while (opcionPrincipal != 5);

    return 0;
}

// ======================================================================
// MODULO 7: IMPLEMENTACION - UTILIDADES DE UI
// ======================================================================
void limpiar() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void pausa() {
    cout << "\n" << CYAN << "Presione Enter para continuar..." << RESET;
    cin.ignore(1000, '\n');
    cin.get();
}

void dibujarLinea(int ancho, char c) {
    cout << CYAN << string(ancho, c) << RESET << endl;
}

void dibujarTitulo(const string& titulo) {
    limpiar();
    dibujarLinea();
    int espacios = (70 - (int)titulo.length()) / 2;
    if (espacios < 0) espacios = 0;
    cout << CYAN << BOLD << string(espacios, ' ') << titulo << RESET << endl;
    dibujarLinea();
}

// ======================================================================
// MODULO 8: IMPLEMENTACION - MENUS PRINCIPALES
// ======================================================================
void pantallaBienvenida() {
    cout << CYAN << BOLD;
    cout << R"(
    +-------------------------------------------------------------------+
    ¦                                                                   ¦
    ¦                SISTEMA DE GESTION NUTRICIONAL 2026                ¦
    ¦                                                                   ¦
    ¦                Tu salud comienza con lo que comes                 ¦
    ¦                                                                   ¦
    +-------------------------------------------------------------------+
    )" << RESET << endl;
    pausa();
}

void mostrarMenuPrincipal(int& opcion) {
    dibujarTitulo("MENU PRINCIPAL");
    cout << " " << BOLD << "1." << RESET << " Registro de Nuevo Usuario\n";
    cout << " " << BOLD << "2." << RESET << " Cargar Perfil Existente\n";
    cout << " " << BOLD << "3." << RESET << " Listado de Usuarios Registrados\n";
    cout << " " << BOLD << "4." << RESET << " Eliminar / Editar Perfil\n";
    cout << " " << BOLD << "5." << RESET << " Salir del Sistema\n";
    dibujarLinea(70, '-');
    cout << " Seleccione una opcion: ";
    if (!(cin >> opcion)) { cin.clear(); cin.ignore(1000, '\n'); opcion = 0; }
}

void subMenuUsuario(Usuario& u) {
    int opcion = 0;
    do {
        menuNutricion(opcion, u);
        switch (opcion) {
        case 1: registroDietaDiaria(u); break;
        case 2: estadoNutricional(u);   break;
        case 3: historialYReporte(u);   break;
        case 4: cout << AMARILLO << "Cerrando sesion..." << RESET << endl; pausa(); break;
        default: cout << ROJO << "Opcion no valida." << RESET << endl; pausa(); break;
        }
    } while (opcion != 4);
}

void menuNutricion(int& opcion, const Usuario& u) {
    dibujarTitulo("CONSULTAS DE MI ALIMENTACION");
    cout << " Usuario activo: " << BOLD << CYAN << u.nombreCompleto << RESET
        << " | IMC: " << fixed << setprecision(2) << u.imc << "\n";
    dibujarLinea(70, '-');
    cout << " " << BOLD << "1." << RESET << " Registro de Dieta Diaria\n";
    cout << " " << BOLD << "2." << RESET << " Estado Nutricional y Requerimiento\n";
    cout << " " << BOLD << "3." << RESET << " Historial y Reporte TXT\n";
    cout << " " << BOLD << "4." << RESET << " Regresar al Menu Principal\n";
    dibujarLinea(70, '-');
    cout << " Seleccione una opcion: ";
    if (!(cin >> opcion)) { cin.clear(); cin.ignore(1000, '\n'); opcion = 0; }
}

// ======================================================================
// MODULO 9: IMPLEMENTACION - GESTION DE USUARIOS
// ======================================================================
void registrarNuevoUsuario(vector<Usuario>& usuarios) {
    Usuario nuevo;
    string buf;
    cin.ignore(1000, '\n');

    dibujarTitulo("REGISTRO DE NUEVO USUARIO");
    cout << "ID del Usuario: ";
    getline(cin, buf);
    nuevo.id = optimizarEspacios(buf);

    cout << "Nombre completo: ";
    getline(cin, buf);
    nuevo.nombreCompleto = optimizarEspacios(buf);

    cout << "Edad: ";
    while (!(cin >> nuevo.edad) || nuevo.edad <= 0) {
        cout << ROJO << "Edad invalida. Ingrese de nuevo: " << RESET;
        cin.clear(); cin.ignore(1000, '\n');
    }

    cin.ignore(1000, '\n');
    cout << "Genero (M/F): ";
    getline(cin, buf);
    nuevo.genero = optimizarEspacios(buf);

    string errDir;
    while (true) {
        cout << "\nDireccion (Ej.): "
            << AMARILLO << "3-44, Zona 1, El Tejar, Chimaltenango" << RESET << "\n";
        cout << "Ingrese direccion: ";
        getline(cin, buf);
        nuevo.direccion = optimizarEspacios(buf);
        if (validarDireccion(nuevo.direccion, errDir)) break;
        cout << ROJO << "\n[ERROR EN DIRECCION]\n" << errDir << RESET << endl;
    }

    string errPeso;
    while (true) {
        cout << "Peso (kg): ";
        if (!(cin >> nuevo.peso)) {
            cout << ROJO << "Entrada invalida.\n" << RESET;
            cin.clear(); cin.ignore(1000, '\n');
            continue;
        }
        if (validarPesoEdad(nuevo.peso, nuevo.edad, errPeso)) break;
        cout << AMARILLO
            << "\n[ADVERTENCIA]\nEl peso ingresado no parece realista.\n"
            << errPeso << "\nIngrese nuevamente el dato.\n" << RESET;
    }

    cout << "Altura (m): ";
    while (!(cin >> nuevo.altura) || nuevo.altura <= 0) {
        cout << ROJO << "Altura invalida. Ingrese de nuevo: " << RESET;
        cin.clear(); cin.ignore(1000, '\n');
    }
    nuevo.imc = nuevo.peso / pow(nuevo.altura, 2);

    cin.ignore(1000, '\n');
    cout << "Nivel de actividad fisica (Sedentario/Ligero/Moderado/Activo): ";
    getline(cin, buf);
    nuevo.nivelActividad = optimizarEspacios(buf);

    cout << "Trabajo u ocupacion: ";
    getline(cin, buf);
    nuevo.ocupacion = optimizarEspacios(buf);

    limpiar();
    dibujarLinea(90, '-');
    cout << BOLD << "\t\t\tObjetivo Fisico:" << RESET << "\n";
    dibujarLinea(90, '-');
    cout << " 1. Bajar de peso\n 2. Mantener peso\n 3. Aumentar masa muscular\nOpcion: ";
    while (!(cin >> nuevo.metaSeleccionada) || nuevo.metaSeleccionada < 1 || nuevo.metaSeleccionada > 3) {
        cout << ROJO << "Seleccion invalida (1-3): " << RESET;
        cin.clear(); cin.ignore(1000, '\n');
    }

    dibujarTitulo("REGISTRO COMPLETADO");
    cout << BOLD << "\n===== INFORMACION DEL USUARIO =====\n" << RESET;
    cout << "ID: " << CYAN << nuevo.id << RESET << endl;
    cout << "Nombre: " << CYAN << nuevo.nombreCompleto << RESET << endl;
    cout << "Edad: " << nuevo.edad << " anios\n";
    cout << "Genero: " << nuevo.genero << endl;
    cout << "Direccion: " << nuevo.direccion << endl;
    cout << "Peso: " << nuevo.peso << " kg\n";
    cout << "Altura: " << nuevo.altura << " m\n";
    cout << "Actividad Fisica: " << nuevo.nivelActividad << endl;
    cout << "Ocupacion: " << nuevo.ocupacion << endl;

    cout << "\n===== RESULTADO NUTRICIONAL =====\n";
    cout << "IMC Calculado: " << BOLD << fixed << setprecision(2) << nuevo.imc << RESET << " -> ";
    evaluacionPesoYMensaje(nuevo.edad, nuevo.imc);
    cout << "\n";

    if (nuevo.imc < 18.5) cout << AMARILLO << "Recomendacion: Debes mejorar tu alimentacion y aumentar tu ingesta calorica." << RESET << endl;
    else if (nuevo.imc < 25.0) cout << VERDE << "Excelente. Mantienes un peso saludable. Continua con buenos habitos alimenticios." << RESET << endl;
    else if (nuevo.imc < 30.0) cout << AMARILLO << "Se recomienda moderar el consumo de grasas y azucares." << RESET << endl;
    else                       cout << ROJO << "Es importante mejorar los habitos alimenticios y realizar actividad fisica." << RESET << endl;

    usuarios.push_back(nuevo);

    // ? BD — guardar usuario en MySQL
    if (g_con) {
        int idGenerado = bd_insertarUsuario(
            g_con,
            nuevo.id,
            nuevo.nombreCompleto,
            nuevo.edad,
            nuevo.genero,
            nuevo.direccion,
            nuevo.peso,
            nuevo.altura,
            nuevo.nivelActividad,
            nuevo.ocupacion,
            nuevo.imc,
            nuevo.metaSeleccionada
        );

        if (idGenerado > 0) {
            usuarios.back().dbId = idGenerado;
            cout << VERDE << "[BD] Usuario guardado en la base de datos con ID: "
                << idGenerado << RESET << "\n";
        }
    }

    pausa();
    subMenuUsuario(usuarios.back());
}

void cargarPerfilExistente(vector<Usuario>& usuarios) {
    if (usuarios.empty()) {
        cout << ROJO << "\nNo hay usuarios registrados aun.\n" << RESET;
        pausa();
        return;
    }
    string buscarId, buscarNombre;
    cin.ignore(1000, '\n');
    dibujarTitulo("CARGAR PERFIL EXISTENTE");
    cout << "Ingrese ID: ";
    getline(cin, buscarId);
    cout << "Ingrese Nombre Completo: ";
    getline(cin, buscarNombre);

    buscarId = optimizarEspacios(buscarId);
    buscarNombre = optimizarEspacios(buscarNombre);

    bool encontrado = false;
    for (auto& u : usuarios) {
        if (aMinusculas(u.id) == aMinusculas(buscarId) &&
            aMinusculas(u.nombreCompleto) == aMinusculas(buscarNombre)) {

            // ? BD — sincronizar dbId si no lo tiene (ej: sesion reiniciada)
            if (g_con && u.dbId < 0) {
                u.dbId = bd_buscarIdUsuario(g_con, u.id, u.nombreCompleto);
            }

            cout << VERDE << "\n¡Perfil cargado! Bienvenido(a) " << BOLD
                << u.nombreCompleto << RESET << ".\n";
            pausa();
            subMenuUsuario(u);
            encontrado = true;
            break;
        }
    }
    if (!encontrado) {
        cout << ROJO << "\nUsuario no encontrado.\n" << RESET;
        pausa();
    }
}

void listarUsuariosRegistrados(const vector<Usuario>& usuarios) {
    dibujarTitulo("LISTADO DE USUARIOS REGISTRADOS");
    cout << BOLD << left
        << setw(12) << "ID"
        << setw(32) << "NOMBRE COMPLETO"
        << setw(8) << "EDAD"
        << setw(10) << "PESO(kg)"
        << setw(10) << "IMC"
        << RESET << endl;
    dibujarLinea(70, '-');

    if (usuarios.empty()) {
        cout << "No hay usuarios en el sistema.\n";
    }
    else {
        for (const auto& u : usuarios) {
            cout << setw(12) << left << u.id
                << setw(32) << u.nombreCompleto.substr(0, 30)
                << setw(8) << u.edad
                << setw(10) << u.peso
                << setw(10) << fixed << setprecision(2) << u.imc << endl;
        }
    }
    dibujarLinea();
    pausa();
}

void editarEliminarPerfil(vector<Usuario>& usuarios) {
    if (usuarios.empty()) {
        cout << ROJO << "\nNo hay usuarios para editar/eliminar.\n" << RESET;
        pausa();
        return;
    }

    string buscarId;
    cin.ignore(1000, '\n');
    dibujarTitulo("EDITAR / ELIMINAR PERFIL");
    cout << "Ingrese ID del usuario: ";
    getline(cin, buscarId);
    buscarId = optimizarEspacios(buscarId);

    for (auto it = usuarios.begin(); it != usuarios.end(); ++it) {
        if (aMinusculas(it->id) == aMinusculas(buscarId)) {
            cout << "Usuario: " << BOLD << it->nombreCompleto << RESET << endl;
            cout << "1. Eliminar\n2. Editar peso\nOpcion: ";
            int op;
            if (!(cin >> op)) { cin.clear(); cin.ignore(1000, '\n'); op = 0; }

            if (op == 1) {
                // ? BD — eliminar de MySQL antes de borrar del vector
                if (g_con) bd_eliminarUsuario(g_con, it->id);
                usuarios.erase(it);
                cout << VERDE << "Usuario eliminado.\n" << RESET;
            }
            else if (op == 2) {
                cout << "Nuevo peso (kg): ";
                cin >> it->peso;
                it->imc = it->peso / pow(it->altura, 2);
                // ? BD — actualizar peso e IMC en MySQL
                if (g_con) bd_actualizarPeso(g_con, it->id, it->peso, it->imc);
                cout << VERDE << "Peso actualizado. Nuevo IMC: "
                    << fixed << setprecision(2) << it->imc << RESET << endl;
            }
            else {
                cout << ROJO << "Opcion invalida.\n" << RESET;
            }

            pausa();
            return;
        }
    }
    cout << ROJO << "Usuario no encontrado.\n" << RESET;
    pausa();
}

// ======================================================================
// MODULO 10: IMPLEMENTACION - MODULO 1: REGISTRO DE DIETA
// ======================================================================
void registroDietaDiaria(Usuario& u) {
    dibujarTitulo("1. REGISTRO DE DIETA DIARIA");

    if (!u.yaTieneCaloriasCalculadas) {
        cout << BOLD << "Primera vez: Configuremos tus calculos base\n" << RESET;
        cout << "Nivel de actividad (1.Sedentario 2.Ligero 3.Moderado 4.Activo 5.Muy activo): ";
        while (!(cin >> u.nivelActividadSeleccionado) || u.nivelActividadSeleccionado < 1 || u.nivelActividadSeleccionado > 5) {
            cout << ROJO << "Opcion invalida: " << RESET;
            cin.clear(); cin.ignore(1000, '\n');
        }

        double alturaCm = u.altura * 100;
        if (u.genero == "M" || u.genero == "m")
            u.tmbCalculada = 10 * u.peso + 6.25 * alturaCm - 5 * u.edad + 5;
        else
            u.tmbCalculada = 10 * u.peso + 6.25 * alturaCm - 5 * u.edad - 161;

        double factor = 1.2;
        switch (u.nivelActividadSeleccionado) {
        case 1: factor = 1.2;   break;
        case 2: factor = 1.375; break;
        case 3: factor = 1.55;  break;
        case 4: factor = 1.725; break;
        case 5: factor = 1.9;   break;
        }

        u.caloriasRecomendadas = u.tmbCalculada * factor;
        if (u.metaSeleccionada == 1) u.caloriasRecomendadas -= 500;
        else if (u.metaSeleccionada == 3) u.caloriasRecomendadas += 500;
        u.yaTieneCaloriasCalculadas = true;
        cout << VERDE << "[OK] Calorias calculadas: " << u.caloriasRecomendadas << " kcal\n" << RESET;
    }

    cout << "\n" << BOLD
        << "+-----------------------------------------------------------------+\n"
        << "¦  #  ¦ Alimento              ¦ Kcal   ¦ Prot(g) ¦ Carb(g)¦Gras(g)¦\n"
        << "¦-----+-----------------------+--------+---------+--------+-------¦\n" << RESET;

    for (size_t i = 0; i < listaAlimentos.size(); i++) {
        cout << "¦ " << setw(3) << left << i + 1
            << " ¦ " << setw(21) << listaAlimentos[i].nombre
            << " ¦ " << setw(6) << listaAlimentos[i].kcal
            << " ¦ " << setw(7) << listaAlimentos[i].proteina
            << " ¦ " << setw(6) << listaAlimentos[i].carbohidratos
            << " ¦ " << setw(5) << listaAlimentos[i].grasa << " ¦\n";
    }
    cout << BOLD << "+-----------------------------------------------------------------+\n" << RESET;

    int seleccion;
    cout << "\n" << BOLD << ">>> REGISTRO DE ALIMENTOS DEL DIA <<<\n" << RESET;

    if (u.desayunoIdx == -1) {
        cout << "Desayuno #: ";
        while (!(cin >> seleccion) || seleccion < 1 || seleccion >(int)listaAlimentos.size()) {
            cout << ROJO << "Numero invalido: " << RESET; cin.clear(); cin.ignore(1000, '\n');
        }
        u.desayunoIdx = seleccion - 1;
        cout << VERDE << "[OK] Desayuno: " << listaAlimentos[u.desayunoIdx].nombre << RESET << "\n";
    }
    else { cout << VERDE << "[OK] Desayuno ya registrado: " << listaAlimentos[u.desayunoIdx].nombre << RESET << "\n"; }

    if (u.almuerzoIdx == -1) {
        cout << "Almuerzo #: ";
        while (!(cin >> seleccion) || seleccion < 1 || seleccion >(int)listaAlimentos.size()) {
            cout << ROJO << "Numero invalido: " << RESET; cin.clear(); cin.ignore(1000, '\n');
        }
        u.almuerzoIdx = seleccion - 1;
        cout << VERDE << "[OK] Almuerzo: " << listaAlimentos[u.almuerzoIdx].nombre << RESET << "\n";
    }
    else { cout << VERDE << "[OK] Almuerzo ya registrado: " << listaAlimentos[u.almuerzoIdx].nombre << RESET << "\n"; }

    if (u.cenaIdx == -1) {
        cout << "Cena #: ";
        while (!(cin >> seleccion) || seleccion < 1 || seleccion >(int)listaAlimentos.size()) {
            cout << ROJO << "Numero invalido: " << RESET; cin.clear(); cin.ignore(1000, '\n');
        }
        u.cenaIdx = seleccion - 1;
        cout << VERDE << "[OK] Cena: " << listaAlimentos[u.cenaIdx].nombre << RESET << "\n";
    }
    else { cout << VERDE << "[OK] Cena ya registrada: " << listaAlimentos[u.cenaIdx].nombre << RESET << "\n"; }

    cout << VERDE << "\n¡Datos guardados exitosamente!\n" << RESET;

    // ? BD — guardar dieta en MySQL
    if (g_con && u.dbId > 0) {
        int    totalK = 0;
        double totalP = 0.0, totalC = 0.0, totalG = 0.0;
        int    idx[3] = { u.desayunoIdx, u.almuerzoIdx, u.cenaIdx };

        for (int i = 0; i < 3; i++) {
            if (idx[i] != -1) {
                totalK += listaAlimentos[idx[i]].kcal;
                totalP += listaAlimentos[idx[i]].proteina;
                totalC += listaAlimentos[idx[i]].carbohidratos;
                totalG += listaAlimentos[idx[i]].grasa;
            }
        }

        bd_guardarDieta(
            g_con,
            u.dbId,
            u.desayunoIdx,
            u.almuerzoIdx,
            u.cenaIdx,
            totalK, totalP, totalC, totalG,
            listaAlimentos
        );

        // Actualizar calorias y TMB calculadas en la fila del usuario
        bd_actualizarCaloriasUsuario(
            g_con,
            u.dbId,
            u.caloriasRecomendadas,
            u.tmbCalculada,
            u.nivelActividad
        );
    }

    pausa();
}

// ======================================================================
// MODULO 11: IMPLEMENTACION - MODULO 2: ESTADO NUTRICIONAL
// ======================================================================
void estadoNutricional(Usuario& u) {
    dibujarTitulo("2. ESTADO NUTRICIONAL Y REQUERIMIENTOS");

    if (!u.yaTieneCaloriasCalculadas) {
        cout << ROJO << "Debe registrar su dieta (Opcion 1) antes de ver el estado.\n" << RESET;
        pausa();
        return;
    }

    cout << BOLD << "--- Datos Fisiologicos ---\n" << RESET;
    cout << "IMC Actual: " << BOLD << fixed << setprecision(2) << u.imc << RESET << " -> ";
    evaluacionPesoYMensaje(u.edad, u.imc);
    cout << "\nMetabolismo Basal (TMB): " << u.tmbCalculada << " kcal\n";

    cout << "\n" << BOLD << "--- Meta Calorica ---\n" << RESET;
    cout << "Objetivo: ";
    if (u.metaSeleccionada == 1) cout << AMARILLO << "Bajar de Peso" << RESET << "\n";
    else if (u.metaSeleccionada == 2) cout << CYAN << "Mantener Peso" << RESET << "\n";
    else                              cout << VERDE << "Aumentar Masa Muscular" << RESET << "\n";
    cout << "Meta Diaria: " << BOLD << u.caloriasRecomendadas << " kcal" << RESET << "\n";

    int    total_kcal = 0;
    double total_prot = 0, total_carb = 0, total_gras = 0;
    int    indices[3] = { u.desayunoIdx, u.almuerzoIdx, u.cenaIdx };

    for (int i = 0; i < 3; i++) {
        if (indices[i] != -1) {
            total_kcal += listaAlimentos[indices[i]].kcal;
            total_prot += listaAlimentos[indices[i]].proteina;
            total_carb += listaAlimentos[indices[i]].carbohidratos;
            total_gras += listaAlimentos[indices[i]].grasa;
        }
    }

    cout << "\n" << BOLD << "--- Balance de Hoy ---\n" << RESET;
    cout << "Consumidas: " << total_kcal << " kcal | Proteina: " << total_prot << "g\n";
    double dif = u.caloriasRecomendadas - total_kcal;
    if (dif > 0) cout << AMARILLO << "Te faltan " << dif << " kcal para tu meta.\n" << RESET;
    else         cout << ROJO << "Te excediste por " << abs(dif) << " kcal.\n" << RESET;

    cout << "\n" << BOLD << "--- Analisis de Habitos ---\n" << RESET;
    bool habitosSaludables = true;

    if (u.desayunoIdx == -1 || u.almuerzoIdx == -1 || u.cenaIdx == -1) {
        cout << AMARILLO << "[ALERTA] Estas omitiendo tiempos de comida.\n" << RESET;
        habitosSaludables = false;
    }

    if (total_kcal > 0) {
        double pctGrasa = (total_gras * 9.0 / total_kcal) * 100;
        cout << "Grasas: " << fixed << setprecision(1) << pctGrasa << "% ";
        if (pctGrasa > 30.0) {
            cout << ROJO << "[ALTO] Recomendado < 30%\n" << RESET;
            habitosSaludables = false;
        }
        else cout << VERDE << "[OK]\n" << RESET;
    }

    if (habitosSaludables && total_kcal > 0)
        cout << VERDE << "\n[EXCELENTE] Buenos habitos detectados.\n" << RESET;

    pausa();
}

// ======================================================================
// MODULO 12: IMPLEMENTACION - MODULO 3: HISTORIAL Y REPORTE
// ======================================================================
void historialYReporte(Usuario& u) {
    dibujarTitulo("3. HISTORIAL Y GENERACION DE REPORTE");

    if (!u.yaTieneCaloriasCalculadas) {
        cout << ROJO << "No hay datos para generar el reporte.\n" << RESET;
        pausa();
        return;
    }

    string nombreArchivo = "ReporteNutricional_" + u.id + ".txt";
    ofstream archivoFisico(nombreArchivo);
    stringstream reporteStr;

    reporteStr << "==================================================\n";
    reporteStr << " REPORTE NUTRICIONAL - SISTEMA 2026\n";
    reporteStr << "==================================================\n";
    reporteStr << "Usuario: " << u.nombreCompleto << " (ID: " << u.id << ")\n";
    reporteStr << "IMC: " << fixed << setprecision(2) << u.imc
        << " | Meta: " << u.caloriasRecomendadas << " kcal\n\n";

    reporteStr << "--- CONSUMO HOY ---\n";
    int    indices[3] = { u.desayunoIdx, u.almuerzoIdx, u.cenaIdx };
    string tiempos[3] = { "Desayuno", "Almuerzo", "Cena" };
    int    totalK = 0, omitidas = 0;

    for (int i = 0; i < 3; i++) {
        reporteStr << tiempos[i] << ": ";
        if (indices[i] == -1) {
            reporteStr << "No registrado\n";
            omitidas++;
        }
        else {
            Alimento al = listaAlimentos[indices[i]];
            reporteStr << al.nombre << " (" << al.kcal << " kcal)\n";
            totalK += al.kcal;
        }
    }

    reporteStr << "\nTOTAL: " << totalK << " kcal\n";
    reporteStr << "\n--- ANALISIS ---\n";
    if (omitidas > 0) reporteStr << "[-] Omitiste " << omitidas << " tiempo(s).\n";
    else              reporteStr << "[+] 3 tiempos completos.\n";
    reporteStr << "==================================================\n";

    cout << reporteStr.str();

    if (archivoFisico.is_open()) {
        archivoFisico << reporteStr.str();
        archivoFisico.close();
        cout << VERDE << "\n[EXITO] Reporte generado: " << nombreArchivo << RESET << "\n";
    }
    else {
        cout << ROJO << "\n[ERROR] No se pudo crear el archivo.\n" << RESET;
    }

    pausa();
}

// ======================================================================
// MODULO 13: IMPLEMENTACION - FUNCIONES AUXILIARES
// ======================================================================
string optimizarEspacios(const string& s) {
    string result;
    bool inSpace = false;
    for (char c : s) {
        if (isspace(c)) { if (!inSpace) { result += ' '; inSpace = true; } }
        else { result += c; inSpace = false; }
    }
    size_t start = result.find_first_not_of(" ");
    size_t end = result.find_last_not_of(" ");
    return (start == string::npos) ? "" : result.substr(start, end - start + 1);
}

string aMinusculas(const string& s) {
    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

bool validarDireccion(const string& dir, string& mensajeError) {
    mensajeError = "";
    vector<string> partes;
    stringstream ss(dir);
    string item;
    while (getline(ss, item, ',')) partes.push_back(optimizarEspacios(item));

    if (partes.size() < 4) {
        mensajeError = "Error: Debe tener 4 elementos separados por comas.\n";
        return false;
    }

    string pCasa = aMinusculas(partes[0]);
    bool tieneNumero = false;
    for (char c : pCasa) { if (isdigit(c)) { tieneNumero = true; break; } }
    if (!tieneNumero) mensajeError += " -> Falta Numero de Casa.\n";

    string pSector = aMinusculas(partes[1]);
    vector<string> sectores = { "zona", "barrio", "colonia", "canton" };
    bool tieneSector = false;
    for (const string& sec : sectores)
        if (pSector.find(sec) != string::npos) { tieneSector = true; break; }
    if (!tieneSector) mensajeError += " -> Falta referencia de Sector.\n";

    string pDepto = aMinusculas(partes[3]);
    if (pDepto.find("chimaltenango") == string::npos)
        mensajeError += " -> Falta 'Chimaltenango'.\n";

    return mensajeError.empty();
}

bool validarPesoEdad(float peso, int anios, string& mensajeError) {
    float  minKg, maxKg;
    string rango;
    if (anios < 1) { minKg = 3.0f; maxKg = 15.0f; rango = "infante"; }
    else if (anios < 18) { minKg = 20.0f; maxKg = 100.0f; rango = "menor"; }
    else if (anios < 60) { minKg = 35.0f; maxKg = 250.0f; rango = "adulto"; }
    else { minKg = 30.0f; maxKg = 200.0f; rango = "adulto mayor"; }

    if (peso < minKg || peso > maxKg) {
        mensajeError = "Peso no realista para " + rango + ". Rango: "
            + to_string((int)minKg) + "-" + to_string((int)maxKg) + " kg";
        return false;
    }
    return true;
}

void evaluacionPesoYMensaje(int edad, double imc) {
    if (edad >= 65) {
        if (imc >= 32.0) cout << ROJO << "Obesidad" << RESET;
        else if (imc >= 28.0) cout << AMARILLO << "Sobrepeso" << RESET;
        else if (imc >= 23.0) cout << VERDE << "Peso Saludable" << RESET;
        else                  cout << ROJO << "Bajo peso" << RESET;
    }
    else if (edad >= 20) {
        if (imc >= 30.0) cout << ROJO << "Obesidad" << RESET;
        else if (imc >= 25.0) cout << AMARILLO << "Sobrepeso" << RESET;
        else if (imc >= 18.5) cout << VERDE << "Peso Saludable" << RESET;
        else                  cout << ROJO << "Bajo peso" << RESET;
    }
    else {
        if (imc >= 27.0) cout << ROJO << "Obesidad" << RESET;
        else if (imc >= 22.0) cout << AMARILLO << "Sobrepeso" << RESET;
        else if (imc >= 17.0) cout << VERDE << "Peso Saludable" << RESET;
        else                  cout << ROJO << "Bajo peso" << RESET;
    }
}