#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <sstream>
#include <ctime>
#include <cstring>
#include <iomanip> // <-- Añadido para setw, left, etc.
#include <math.h>
#include "ConexionBD.h" // Asegúrate de que este archivo esté en el mismo directorio o ajusta la ruta

using namespace std;

//Utilidad: Trimming
string trim(const string& s)
{
    size_t inicio = s.find_first_not_of(" \t\r\n");
    if (inicio == string::npos) return "";
    size_t fin = s.find_last_not_of(" \t\r\n");
    return s.substr(inicio, fin - inicio + 1);
}

string aMinusculas(const string& s)
{

    string r = s;
    transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

//Estructura de datos
struct Usuario
{
    string nombreCompleto;
    struct tm fechaNacimiento = {};
    string genero;
    string direccion;
    float peso = 0.0f;
    float altura = 0.0f;
    string nivelActividad;
    string ocupacion;
};

// estructura para alimentos consumidos
struct Alimento {
    string nombre;
    int kcal;
    double proteina;
    double carbohidratos;
    double grasa;
};

void obeso();
void sobrepeso();
void normal();
void bajo();  // para imprimir los mensajes de los diferentes pesos que tenga segun el imc
void calorias(float& peso, float& altura, int& tA, string& genero, string& nombreCompleto);
void menu(int& opcion);
void comprobacion();
bool validarDireccion(const string& dir, string& mensajeError);
void calcularEdad(struct tm nac, int& dias, int& meses, int& anios);
bool validarPesoEdad(float peso, int anios, string& mensajeError);

// Portable, safe localtime wrapper: uses localtime_s on MSVC, localtime_r on POSIX, falls back to localtime otherwise
bool localtime_safe(const time_t* t, struct tm* out) {
#ifdef _MSC_VER
    return localtime_s(out, t) == 0;
#elif defined(__unix__) || defined(__APPLE__)
    return localtime_r(t, out) != nullptr;
#else
    struct tm* tmp = localtime(t);
    if (tmp) { *out = *tmp; return true; }
    memset(out, 0, sizeof(*out));
    return false;
#endif
}

bool validarDireccion(const string& dir, string& mensajeError) {
    string d = aMinusculas(dir);
    mensajeError = "";

    // Correcion for: Se asegura compatibilidad C++11
    bool tieneNumero = false;
    for (size_t i = 0; i < dir.length(); ++i)
    {
        if (isdigit(dir[i])) { tieneNumero = true; break; }
    }
    if (!tieneNumero) mensajeError += "  - Falta numero de casa (ej: 5-32)\n";

    // Correcion Corchetes: Definicion de vectores
    vector<string> sectores;
    sectores.push_back("zona"); sectores.push_back("barrio");
    sectores.push_back("colonia"); sectores.push_back("canton");

    bool tieneSector = false;
    for (size_t i = 0; i < sectores.size(); ++i) {
        if (d.find(sectores[i]) != string::npos) { tieneSector = true; break; }
    }
    if (!tieneSector) mensajeError += "  - Falta referencia (zona/barrio/colonia/canton)\n";

    const char* munArray[] = { "chimaltenango", "san jose poaquil", "san martin jilotepeque", "comalapa", "santa apolonia", "tecpan", "patzun", "pochuta", "patzicia", "santa cruz balanya", "acatenango", "yepocapa", "san andres itzapa", "parramos", "zaragoza", "el tejar" };
    bool tieneMunicipio = false;
    for (int i = 0; i < 16; ++i) {
        if (d.find(munArray[i]) != string::npos) { tieneMunicipio = true; break; }
    }
    if (!tieneMunicipio) mensajeError += "  - No es un municipio de Chimaltenango\n";

    return mensajeError.empty();
}

void calcularEdad(struct tm nac, int& dias, int& meses, int& anios)
{
    time_t ahora = time(0);
    struct tm hoy;

    if (!localtime_safe(&ahora, &hoy)) {
        // If obtaining local time failed, zero the struct to avoid undefined data
        memset(&hoy, 0, sizeof(hoy));
    }

    anios = hoy.tm_year - nac.tm_year;
    meses = hoy.tm_mon - nac.tm_mon;
    dias = hoy.tm_mday - nac.tm_mday;

    if (dias < 0) { meses--; dias += 30; }
    if (meses < 0) { anios--; meses += 12; }
}

bool validarPesoEdad(float peso, int anios, string& mensajeError) {
    float minKg, maxKg;
    string rango;
    if (anios < 1) { minKg = 3.0f; maxKg = 15.0f; rango = "infante"; }
    else if (anios < 18) { minKg = 20.0f; maxKg = 100.0f; rango = "menor de edad"; }
    else if (anios < 60) { minKg = 35.0f; maxKg = 250.0f; rango = "adulto"; }
    else { minKg = 30.0f; maxKg = 200.0f; rango = "adulto mayor"; }

    if (peso < minKg || peso > maxKg) {
        mensajeError = "Peso poco realista para " + rango + " (" + to_string((int)minKg) + "-" + to_string((int)maxKg) + " kg)";
        return false;
    }
    return true;
}
int main() {
    int opcion = 0;
    double imc;
    ConexionBD cn = ConexionBD();
    cn.abrir_conexion();
    if (cn.get_conexion()) {
        cout << "Conexion a base de datos exitosa.\n";
        // Aquí podrías agregar código para interactuar con la base de datos
    }
    else {
        cout << "Error al conectar a la base de datos.\n";
    }

    cout << "\n-----------------------------------------------------------------------------------" << endl;
    cout << "  \t\t   SISTEMA DE GESTION NUTRICIONAL          \n";
    cout << "\t\t Tu salud comienza con lo que comes       \n";
    cout << "===================================================================================\n";

    Usuario nuevo;
    string buf;
    cin.ignore();

    cout << "Nombre completo: ";
    getline(cin, buf);
    nuevo.nombreCompleto = trim(buf);

    cout << "Fecha de nacimiento (DD MM AAAA): ";
    int d, m, a;
    cin >> d >> m >> a;
    memset(&nuevo.fechaNacimiento, 0, sizeof(nuevo.fechaNacimiento));
    nuevo.fechaNacimiento.tm_mday = d;
    nuevo.fechaNacimiento.tm_mon = m - 1;
    nuevo.fechaNacimiento.tm_year = a - 1900;

    int tD, tM, tA;
    calcularEdad(nuevo.fechaNacimiento, tD, tM, tA);
    cout << "Edad: " << tA << " anos." << endl;

    cin.ignore();
    cout << "Genero (M/F): ";
    getline(cin, buf);
    nuevo.genero = trim(buf);

    string errDir;
    do {
        cout << "Direccion (incluya zona, municipio y Chimaltenango): ";
        getline(cin, buf);
        nuevo.direccion = trim(buf);
        if (!validarDireccion(nuevo.direccion, errDir)) cout << "Error:\n" << errDir;
    } while (!errDir.empty());

    string errPeso;
    do {
        cout << "Peso (kg): ";
        if (!(cin >> nuevo.peso)) { cin.clear(); cin.ignore(1000, '\n'); }
    } while (!validarPesoEdad(nuevo.peso, tA, errPeso) && (cout << errPeso << endl));

    cout << "Altura (m): ";
    cin >> nuevo.altura;
    imc = nuevo.peso / pow(nuevo.altura, 2);
    cin.ignore();
    cout << "Actividad: ";
    getline(cin, buf);
    nuevo.nivelActividad = trim(buf);

    cout << "Ocupacion: ";
    getline(cin, buf);
    nuevo.ocupacion = trim(buf);

    cout << "\n--- REGISTRO EXITOSO ---" << endl;
    cout << "Nombre: " << nuevo.nombreCompleto << endl;
    cout << "Fecha de nacimiento: " << d << "/" << m << "/" << a << " (Edad: " << tA << " anos)" << endl;
    cout << "Genero: " << nuevo.genero << endl;
    cout << "Direccion: " << nuevo.direccion << endl;
    cout << "Peso: " << nuevo.peso << " kg" << endl;
    cout << "Altura: " << nuevo.altura << " m" << endl;
    cout << "Nivel de actividad: " << nuevo.nivelActividad << endl;
    cout << "Ocupacion: " << nuevo.ocupacion << endl;
    cout << "Su Indice de Masa Corporal es: " << imc << endl;
    cout << "\n\n ----------------------------------------------------------------------------------" << endl;

    if (tA >= 65) {
        cout << "Cuida tu salud y tu alimentacion" << endl;
        if (imc >= 32) { cout << "Obesidad" << endl; obeso(); }
        else if (imc >= 28) { cout << "Sobrepeso" << endl; sobrepeso(); }
        else if (imc >= 23) { cout << "Peso Saludable" << endl; normal(); }
        else { cout << "Delgadez" << endl; bajo(); }
    }
    else if (tA >= 20) {
        cout << "Cuida tu salud y tu alimentacion" << endl;
        if (imc >= 30) { cout << "Obesidad" << endl; obeso(); }
        else if (imc >= 25) { cout << "Sobrepeso" << endl; sobrepeso(); }
        else if (imc >= 18.5) { cout << "Peso Saludable" << endl; normal(); }
        else { cout << "Bajo peso" << endl; bajo(); }
    }
    else {
        if (imc >= 27) { cout << "Obesidad" << endl; obeso(); }
        else if (imc >= 22) { cout << "Sobrepeso" << endl; sobrepeso(); }
        else if (imc >= 17) { cout << "Peso Saludable" << endl; normal(); }
        else { cout << "Bajo peso" << endl; bajo(); }
    }
    system("pause");
    system("cls");

    do {

        menu(opcion);

        switch (opcion)
        {
        case 1: calorias(nuevo.peso, nuevo.altura, tA, nuevo.genero, nuevo.nombreCompleto);
            break;
        case 2: comprobacion();
            system("pause");
            break;
        case 3: cout << "Saliendo del sistema..." << endl;
            break;
        default: cout << "Opcion no valida." << endl; system("pause"); break;
        }
    } while (opcion != 3);
    return 0;
}
void obeso() {
    cout << "\033[31m" << "\n\n ¡Tu cuerpo te sostiene todos los días, y cuidarlo es un acto de amor propio, no un castigo. No se trata de buscar la perfección," << endl;
    cout << "\033[31m" << "sino de ganar salud, energía y bienestar para disfrutar la vida al máximo.tu alimentacion y haz ejercicio!" << endl;
    cout << "\033[0m";
}
void sobrepeso() {
    cout << "\n\n El sobrepeso es simplemente una señal de alerta de tu cuerpo que te invita a hacer una pausa y ajustar el camino. No necesitas " << endl;
    cout << " cambios drásticos ni dietas extremas; pequeños ajustes diarios en tu alimentación y un poco más de movimiento son suficientes para" << endl;
    cout << "marcar una gran diferencia en cómo te sientes. ";
}
void normal() {
    cout << "\n\n¡Felicidades! Mantenerte en un peso saludable es un gran logro y el reflejo directo del amor, tiempo y cuidado que le dedicas a " << endl;
    cout << "tu cuerpo cada día.Estar en tu peso ideal significa que le estás dando a tu organismo el equilibrio que necesita para funcionar al" << endl;
    cout << " máximo, proteger tus defensas y llenarte de energía para disfrutar la vida.";
}
void bajo() {
    cout << "\n\n No se trata solo de comer más, sino de nutrirte mejor, con paciencia y de forma saludable. Cada paso que des para cuidar tu " << endl;
    cout << " alimentación es una inversión en tu bienestar, tus defensas y tu vitalidad.¡Tu cuerpo merece estar fuerte y lleno de vida," << endl;
    cout << "camina hacia esa meta con constancia!";
}

void calorias(float& peso, float& altura, int& tA, string& genero, string& nombreCompleto) {

    double tmb, cal, factor;
    int nivel;

    cout << "=== Calculadora de Calorias Diarias segun la formula de la OMS ===\n";
    cout << "\nNivel de actividad fisica:\n";
    cout << " 1. Sedentario (poco o nada de ejercicio)\n";
    cout << " 2. Ligero (1-3 dias/semana)\n";
    cout << " 3. Moderado (3-5 dias/semana)\n";
    cout << " 4. Activo (6-7 dias/semana)\n";
    cout << " 5. Muy activo (ejercicio intenso diario)\n";
    cout << "Opcion: ";
    cin >> nivel;

    // Formula Harris-Benedict revisada (Mifflin-St Jeor)
    double alturaCm = altura * 100;
    if (genero == "M" || genero == "m")

        tmb = 10 * peso + 6.25 * alturaCm - 5 * tA + 5;
    else
        tmb = 10 * peso + 6.25 * altura - 5 * tA - 161;

    switch (nivel) {
    case 1: factor = 1.2;   break;
    case 2: factor = 1.375; break;
    case 3: factor = 1.55;  break;
    case 4: factor = 1.725; break;
    case 5: factor = 1.9;   break;
    default:
        cout << "Nivel invalido.\n";
    }
    cal = tmb * factor;

    cout << "\n--- Resultado ---\n";
    cout << "TMB (metabolismo basal): " << tmb << " kcal\n";
    cout << "Calorias diarias recomendadas:" << cal << " kcal\n";
    system("pause");
    system("cls");

    vector<Alimento> alimentos = {
        // --- COMIDAS RÁPIDAS ---
        {"Hamburguesa con queso", 303, 15.0, 30.0, 14.0},
        {"Piza de pepperoni (1 porcion)", 290, 12.0, 32.0, 12.0},
        {"Papas fritas (medianas)", 365, 4.0, 48.0, 17.0},
        {"Nuggets de pollo (6 pzas)", 270, 13.0, 16.0, 16.0},
        {"Hot Dog sencillo", 290, 10.0, 24.0, 16.0},

        // --- BEBIDAS ---
        {"Refresco de cola (355ml)", 150, 0.0, 39.0, 0.0},
        {"Cerveza clara (355ml)", 153, 1.6, 13.0, 0.0},
        {"Jugo de naranja natural", 110, 2.0, 26.0, 0.2},
        {"Cafe Latte entero", 120, 6.0, 12.0, 6.0},
        {"Bebida energetica (250ml)", 110, 0.0, 28.0, 0.0},

        // --- COMIDAS COMPLETAS (Porciones estándar de consumo) ---
        {"Pechuga de pollo asada", 165, 31.0, 0.0, 3.6},
        {"Carne para asar (res)", 250, 26.0, 0.0, 15.0},
        {"Filete de salmon", 208, 20.0, 0.0, 13.0},
        {"Arroz blanco cocido", 130, 2.4, 28.0, 0.3},
        {"Frijoles negros cocidos", 132, 9.0, 23.0, 0.5},
        {"Pasta cocida (espagueti)", 158, 5.8, 31.0, 0.9},
        {"Ensalada Cesar con pollo", 350, 18.0, 15.0, 24.0},
        {"Sushi Filadelfia (8 pzas)", 320, 8.0, 52.0, 8.0}
    };

    vector<string> comidas = { "Desayuno", "Almuerzo", "Cena" };// Matriz dinámica: 3 posiciones (una por comida), cada una empieza vacía
    vector<vector<int>> selecciones_por_comida(3);
    int opcion_comida, opcion_alimento; // Variables de control para el menú

    do {
        // Mostrar menú de tiempos de comida
        cout << "\n=== ¿A QUE TIEMPO DE COMIDA DESEAS AGREGAR ALIMENTOS? ===\n";
        for (size_t i = 0; i < comidas.size(); i++) {
            cout << i + 1 << ". " << comidas[i] << " (" << selecciones_por_comida[i].size() << " agregados)\n";
        }
        cout << "4. Terminar y ver Resumen Total de Nutrientes\n";
        cout << "Seleccione una opcion: ";
        cin >> opcion_comida;

        if (opcion_comida >= 1 && opcion_comida <= 3) {
            int indice_comida = opcion_comida - 1; // Ajustar a índice 0, 1 o 2

            // Mostrar la tabla de alimentos disponibles
            cout << "\n=== MENU DE ALIMENTOS DISPONIBLES ===\n";
            cout << left << setw(4) << "#" << setw(12) << "Alimento"
                << setw(8) << "Kcal" << setw(10) << "Proteina"
                << setw(14) << "Carbohidratos" << "Grasa\n";
            cout << string(50, '-') << "\n";

            for (size_t i = 0; i < alimentos.size(); i++) {
                cout << left << setw(4) << i + 1
                    << setw(12) << alimentos[i].nombre
                    << setw(8) << alimentos[i].kcal
                    << setw(10) << to_string(alimentos[i].proteina) + "g"
                    << setw(14) << to_string(alimentos[i].carbohidratos) + "g"
                    << alimentos[i].grasa << "g\n";
            }

            cout << "0. Volver al menu de comidas\n";
            cout << "Seleccione el numero del alimento para el " << comidas[indice_comida] << ": ";
            cin >> opcion_alimento;

            // Validar que el alimento exista y agregarlo a esa comida específica
            if (opcion_alimento >= 1 && opcion_alimento <= (int)alimentos.size()) {
                selecciones_por_comida[indice_comida].push_back(opcion_alimento - 1);
                cout << "\x1B[32m¡" << alimentos[opcion_alimento - 1].nombre << " agregado al " << comidas[indice_comida] << "!\x1B[0m\n";
            }
        }
    } while (opcion_comida != 4);

    cout << "\n\n==================================================\n";
    cout << "          RESUMEN NUTRICIONAL DEL DIA             \n";
    cout << "==================================================\n";

    // Variables globales para la suma de todo el día
    int total_kcal_dia = 0;
    double total_prot_dia = 0, total_carb_dia = 0, total_gras_dia = 0;

    // Recorrer cada tiempo de comida (Desayuno, Almuerzo, Cena)
    for (size_t i = 0; i < comidas.size(); i++) {
        cout << "\n> " << comidas[i] << ":\n";

        if (selecciones_por_comida[i].empty()) {
            cout << "  No se registraron alimentos.\n";
            continue;
        }

        // Variables locales para sumar solo esta comida
        int kcal_comida = 0;
        double prot_comida = 0, carb_comida = 0, gras_comida = 0;

        // Recorrer los alimentos que el usuario guardó en esta comida
        for (int idx_alimento : selecciones_por_comida[i]) {
            Alimento al = alimentos[idx_alimento];
            cout << "  - " << al.nombre << " (" << al.kcal << " Kcal)\n";

            // Sumar a la comida actual
            kcal_comida += al.kcal;
            prot_comida += al.proteina;
            carb_comida += al.carbohidratos;
            gras_comida += al.grasa;
        }

        // Mostrar subtotales de la comida
        cout << fixed << setprecision(1); // Formatear decimales a un solo dígito
        cout << "  \x1B[36mSubtotal " << comidas[i] << " -> Kcal: " << kcal_comida
            << " | Prot: " << prot_comida << "g | Carb: " << carb_comida
            << "g | Gras: " << gras_comida << "g\x1B[0m\n";

        // Acumular en los totales generales del día
        total_kcal_dia += kcal_comida;
        total_prot_dia += prot_comida;
        total_carb_dia += carb_comida;
        total_gras_dia += gras_comida;
    }

    // Mostrar el Gran Total Final en pantalla
    cout << "\n==================================================\n";
    cout << " \x1B[33mTOTAL CONSUMIDO EN EL DIA:\x1B[0m\n";
    cout << " - Calorias Totales:  " << total_kcal_dia << " Kcal\n";
    cout << " - Proteinas Totales: " << total_prot_dia << " g\n";
    cout << " - Carbohidratos:     " << total_carb_dia << " g\n";
    cout << " - Grasas Totales:    " << total_gras_dia << " g\n";
    cout << "==================================================\n";


    if (total_kcal_dia = cal) {
        cout << "\n¡ FELICIDADES  " << nombreCompleto << " HAZ MANTENIDO UNA ALIMETACION BALANCEADA SEGUN TU TIPO DE ACTIVIDAD FISICA!" << endl;
        cout << "\n\n__________________________________________________\n";
    }
    else if (total_kcal_dia > cal) {
        cout << "\033[31m" << "\n ¡CUIDA TU ALIMENTACION! " << endl;
        cout << "\033[31m" << nombreCompleto << "Estas consumiendo demasiada calorias, eso puede afectar tu salud..." << endl;
        cout << "\n\n__________________________________________________\n";
        cout << "\033[0m";
    }
    else {
        cout << nombreCompleto << "\n puede que te falte alimentos con mas calorias, puede afectar en perdida de masa muscular, perdida de cabello, ¡alimentate bien!" << endl;
        cout << "\n\n__________________________________________________\n";
    }
}

void menu(int& opcion) {
    cout << "==================================================================================================================" << endl;
    cout << "   \t\t\t\tCONSULTAS DE MI ALIMENTACION " << endl;
    cout << "==================================================================================================================" << endl << endl;
    cout << "1. CONSULTA DE MI REQUERIMIENTO CALORICO Y SUGERENCIAS" << endl;
    cout << "2. COMPROBACION DE ALIMETOS CONSUMIDOS/ RECOMENDADOS" << endl;
    cout << "3. Salir" << endl;
    cout << "------------------------------------------------------------------------------------------------------------------" << endl;
    cout << "\t\t\t\t\tSeleccione una opcion: " << endl;
    cout << "------------------------------------------------------------------------------------------------------------------" << endl;
    if (!(cin >> opcion)) { // Validacion para evitar bucle infinito si meten letras
        cin.clear();
        cin.ignore(1000, '\n');
        opcion = 0;
    }
}


void comprobacion() {

}