#pragma once

#include <mysql.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include <sstream>

#include "Tipos.h"

using namespace std;

// ======================================================
// FUNCION PARA ESCAPAR TEXTO
// Evita errores con apostrofes y caracteres especiales
// ======================================================

inline string escapar(MYSQL* con, const string& valor) {

    string buffer(valor.size() * 2 + 1, '\0');

    unsigned long len = mysql_real_escape_string(
        con,
        &buffer[0],
        valor.c_str(),
        valor.size()
    );

    buffer.resize(len);

    return buffer;
}

// ======================================================
// INSERTAR USUARIO
// Devuelve el ID generado o -1 si falla
// ======================================================

inline int bd_insertarUsuario(
    MYSQL* con,
    const string& codigo,
    const string& nombre,
    int edad,
    const string& genero,
    const string& direccion,
    float peso,
    float altura,
    const string& nivelActividad,
    const string& ocupacion,
    double imc,
    int meta
) {

    ostringstream oss;
    oss << "INSERT INTO usuarios ("
        << "codigo_usuario, "
        << "nombre_completo, "
        << "edad, "
        << "genero, "
        << "direccion, "
        << "peso, "
        << "altura, "
        << "nivel_actividad, "
        << "ocupacion, "
        << "imc, "
        << "meta_seleccionada) VALUES ("
        << "'" << escapar(con, codigo) << "',"
        << "'" << escapar(con, nombre) << "',"
        << edad << ", "
        << "'" << escapar(con, genero) << "',"
        << "'" << escapar(con, direccion) << "',"
        << peso << ", "
        << altura << ", "
        << "'" << escapar(con, nivelActividad) << "',"
        << "'" << escapar(con, ocupacion) << "',"
        << imc << ", "
        << meta << ")";

    string q = oss.str();

    if (mysql_query(con, q.c_str())) {

        cout << "\nERROR AL GUARDAR USUARIO\n";
        cout << mysql_error(con) << "\n\n";

        cout << "CONSULTA SQL:\n";
        cout << q << "\n\n";

        return -1;
    }

    cout << "Usuario guardado correctamente.\n";

    return (int)mysql_insert_id(con);
}

// ======================================================
// ACTUALIZAR CALORIAS Y TMB
// ======================================================

inline void bd_actualizarCaloriasUsuario(
    MYSQL* con,
    int idUsuario,
    double calorias,
    double tmb,
    const string& nivelActividad
) {

    ostringstream oss;
    oss << "UPDATE usuarios SET "
        << "calorias_recomendadas = " << calorias << ", "
        << "tmb_calculada = " << tmb << ", "
        << "nivel_actividad = '" << escapar(con, nivelActividad) << "' "
        << "WHERE id_usuario = " << idUsuario;

    string q = oss.str();

    if (mysql_query(con, q.c_str())) {

        cout << "Error al actualizar calorias: ";
        cout << mysql_error(con) << "\n";
    }
    else {

        cout << "Calorias actualizadas correctamente.\n";
    }
}

// ======================================================
// ACTUALIZAR PESO
// ======================================================

inline void bd_actualizarPeso(
    MYSQL* con,
    const string& codigo,
    float nuevoPeso,
    double nuevoImc
) {

    ostringstream oss;
    oss << "UPDATE usuarios SET "
        << "peso = " << nuevoPeso << ", "
        << "imc = " << nuevoImc << " "
        << "WHERE codigo_usuario = '" << escapar(con, codigo) << "'";

    string q = oss.str();

    if (mysql_query(con, q.c_str())) {

        cout << "Error al actualizar peso: ";
        cout << mysql_error(con) << "\n";
    }
    else {

        cout << "Peso actualizado correctamente.\n";
    }
}

// ======================================================
// ELIMINAR USUARIO
// ======================================================

inline void bd_eliminarUsuario(
    MYSQL* con,
    const string& codigo
) {

    ostringstream oss;
    oss << "DELETE FROM usuarios WHERE codigo_usuario = '" << escapar(con, codigo) << "'";
    string q = oss.str();

    if (mysql_query(con, q.c_str())) {

        cout << "Error al eliminar usuario: ";
        cout << mysql_error(con) << "\n";
    }
    else {

        cout << "Usuario eliminado correctamente.\n";
    }
}

// ======================================================
// BUSCAR ID USUARIO
// ======================================================

inline int bd_buscarIdUsuario(
    MYSQL* con,
    const string& codigo,
    const string& nombre
) {

    ostringstream oss;
    oss << "SELECT id_usuario FROM usuarios WHERE codigo_usuario = '"
        << escapar(con, codigo) << "' AND nombre_completo = '" << escapar(con, nombre)
        << "' LIMIT 1";

    string q = oss.str();

    if (mysql_query(con, q.c_str())) {

        cout << "Error al buscar usuario: ";
        cout << mysql_error(con) << "\n";

        return -1;
    }

    MYSQL_RES* res = mysql_store_result(con);

    if (!res)
        return -1;

    MYSQL_ROW fila = mysql_fetch_row(res);

    int id = -1;

    if (fila && fila[0])
        id = stoi(fila[0]);

    mysql_free_result(res);

    return id;
}

// ======================================================
// OBTENER ID DE ALIMENTO
// ======================================================

inline int bd_idAlimento(
    MYSQL* con,
    const string& nombre
) {

    ostringstream oss;
    oss << "SELECT id_alimento FROM alimentos WHERE nombre = '" << escapar(con, nombre) << "' LIMIT 1";
    string q = oss.str();

    if (mysql_query(con, q.c_str())) {

        cout << "Error al buscar alimento: ";
        cout << mysql_error(con) << "\n";

        return -1;
    }

    MYSQL_RES* res = mysql_store_result(con);

    if (!res)
        return -1;

    MYSQL_ROW fila = mysql_fetch_row(res);

    int id = -1;

    if (fila && fila[0])
        id = stoi(fila[0]);

    mysql_free_result(res);

    return id;
}

// ======================================================
// GUARDAR DIETA
// ======================================================

inline void bd_guardarDieta(
    MYSQL* con,
    int idUsuario,
    int desayunoIdx,
    int almuerzoIdx,
    int cenaIdx,
    int totalKcal,
    double totalProt,
    double totalCarb,
    double totalGras,
    const vector<Alimento>& listaAlimentos
) {

    // ==========================================
    // OBTENER IDS REALES DE LA BASE DE DATOS
    // ==========================================

    string desayuno = "NULL";
    string almuerzo = "NULL";
    string cena = "NULL";

    if (desayunoIdx != -1) {

        int id = bd_idAlimento(
            con,
            listaAlimentos[desayunoIdx].nombre
        );

        if (id != -1)
            desayuno = to_string(id);
    }

    if (almuerzoIdx != -1) {

        int id = bd_idAlimento(
            con,
            listaAlimentos[almuerzoIdx].nombre
        );

        if (id != -1)
            almuerzo = to_string(id);
    }

    if (cenaIdx != -1) {

        int id = bd_idAlimento(
            con,
            listaAlimentos[cenaIdx].nombre
        );

        if (id != -1)
            cena = to_string(id);
    }

    // ==========================================
    // FECHA ACTUAL
    // ==========================================

    time_t t = time(nullptr);

    struct tm tiempo;

#ifdef _WIN32
    localtime_s(&tiempo, &t);
#else
    localtime_r(&t, &tiempo);
#endif

    char fecha[11];

    strftime(
        fecha,
        sizeof(fecha),
        "%Y-%m-%d",
        &tiempo
    );

    // ==========================================
    // INSERTAR O ACTUALIZAR REGISTRO
    // ==========================================

    ostringstream oss;
    oss << "INSERT INTO registro_dieta ("
        << "id_usuario, desayuno_id, almuerzo_id, cena_id, fecha, total_kcal, total_proteina, total_carbohidratos, total_grasa) VALUES ("
        << idUsuario << ","
        << desayuno << ","
        << almuerzo << ","
        << cena << ",'" << string(fecha) << "',"
        << totalKcal << ","
        << totalProt << ","
        << totalCarb << ","
        << totalGras << ") ON DUPLICATE KEY UPDATE "
        << "desayuno_id = VALUES(desayuno_id), "
        << "almuerzo_id = VALUES(almuerzo_id), "
        << "cena_id = VALUES(cena_id), "
        << "total_kcal = VALUES(total_kcal), "
        << "total_proteina = VALUES(total_proteina), "
        << "total_carbohidratos = VALUES(total_carbohidratos), "
        << "total_grasa = VALUES(total_grasa)";

    string q = oss.str();

    if (mysql_query(con, q.c_str())) {

        cout << "\nERROR AL GUARDAR DIETA\n";
        cout << mysql_error(con) << "\n\n";

        cout << "CONSULTA SQL:\n";
        cout << q << "\n\n";
    }
    else {

        cout << "Dieta guardada correctamente.\n";
    }
}
