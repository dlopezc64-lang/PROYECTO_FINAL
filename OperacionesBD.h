// OperacionesBD.h
#pragma once
#include <mysql.h>
#include <string>
#include <iostream>
#include <vector>
#include <ctime>
#include "Tipos.h"
using namespace std;

// Escapa strings para evitar errores con apostrofes y caracteres especiales
inline string escapar(MYSQL* con, const string& valor) {
    string buf(valor.size() * 2 + 1, '\0');
    unsigned long len = mysql_real_escape_string(con, &buf[0], valor.c_str(), valor.size());
    buf.resize(len);
    return buf;
}

// ─────────────────────────────────────────────────────────────
// USUARIOS
// ─────────────────────────────────────────────────────────────

// Devuelve el id_usuario generado, o -1 si falla
inline int bd_insertarUsuario(MYSQL* con, const string& codigo, const string& nombre,
    int edad, const string& genero, const string& direccion,
    float peso, float altura, const string& nivelActividad,
    const string& ocupacion, double imc, int meta) {

    string q =
        "INSERT INTO usuarios "
        "(codigo_usuario, nombre_completo, edad, genero, direccion, "
        " peso, altura, nivel_actividad, ocupacion, imc, meta_seleccionada) "
        "VALUES ('" +
        escapar(con, codigo) + "','" +
        escapar(con, nombre) + "'," +
        to_string(edad) + ",'" +
        escapar(con, genero) + "','" +
        escapar(con, direccion) + "'," +
        to_string(peso) + "," +
        to_string(altura) + ",'" +
        escapar(con, nivelActividad) + "','" +
        escapar(con, ocupacion) + "'," +
        to_string(imc) + "," +
        to_string(meta) + ")";

    if (mysql_query(con, q.c_str())) {
        cout << "Error al guardar usuario: " << mysql_error(con) << "\n";
        return -1;
    }
    return (int)mysql_insert_id(con);
}

inline void bd_actualizarCaloriasUsuario(MYSQL* con, int idUsuario,
    double calorias, double tmb, int nivelActividadSel) {
    string q =
        "UPDATE usuarios SET "
        "calorias_recomendadas=" + to_string(calorias) + ", "
        "tmb_calculada=" + to_string(tmb) + ", "
        "nivel_actividad='" + to_string(nivelActividadSel) + "' "
        "WHERE id_usuario=" + to_string(idUsuario);

    if (mysql_query(con, q.c_str()))
        cout << "Error al actualizar calorias: " << mysql_error(con) << "\n";
}

inline void bd_actualizarPeso(MYSQL* con, const string& codigo, float nuevoPeso, double nuevoImc) {
    string q =
        "UPDATE usuarios SET peso=" + to_string(nuevoPeso) +
        ", imc=" + to_string(nuevoImc) +
        " WHERE codigo_usuario='" + escapar(con, codigo) + "'";

    if (mysql_query(con, q.c_str()))
        cout << "Error al actualizar peso: " << mysql_error(con) << "\n";
    else
        cout << "Peso actualizado en la base de datos.\n";
}

inline void bd_eliminarUsuario(MYSQL* con, const string& codigo) {
    string q = "DELETE FROM usuarios WHERE codigo_usuario='" + escapar(con, codigo) + "'";
    if (mysql_query(con, q.c_str()))
        cout << "Error al eliminar usuario: " << mysql_error(con) << "\n";
    else
        cout << "Usuario eliminado de la base de datos.\n";
}

// Devuelve el id_usuario de un codigo+nombre, o -1 si no existe
inline int bd_buscarIdUsuario(MYSQL* con, const string& codigo, const string& nombre) {
    string q =
        "SELECT id_usuario FROM usuarios WHERE "
        "codigo_usuario='" + escapar(con, codigo) + "' AND "
        "nombre_completo='" + escapar(con, nombre) + "' LIMIT 1";

    if (mysql_query(con, q.c_str())) return -1;

    MYSQL_RES* res = mysql_store_result(con);
    if (!res) return -1;

    MYSQL_ROW fila = mysql_fetch_row(res);
    int id = fila ? stoi(fila[0]) : -1;
    mysql_free_result(res);
    return id;
}

// ─────────────────────────────────────────────────────────────
// ALIMENTOS — obtener id por nombre (para registro_dieta)
// ─────────────────────────────────────────────────────────────
inline int bd_idAlimento(MYSQL* con, const string& nombre) {
    string q =
        "SELECT id_alimento FROM alimentos WHERE nombre='"
        + escapar(con, nombre) + "' LIMIT 1";

    if (mysql_query(con, q.c_str())) return -1;

    MYSQL_RES* res = mysql_store_result(con);
    if (!res) return -1;

    MYSQL_ROW fila = mysql_fetch_row(res);
    int id = fila ? stoi(fila[0]) : -1;
    mysql_free_result(res);
    return id;
}

// ─────────────────────────────────────────────────────────────
// REGISTRO DIETA
// ─────────────────────────────────────────────────────────────
inline void bd_guardarDieta(MYSQL* con, int idUsuario,
    int desayunoIdx, int almuerzoIdx, int cenaIdx,
    int totalKcal, double totalProt, double totalCarb, double totalGras,
    const vector<Alimento>& listaAlimentos) {

    // Obtener IDs reales de la BD
    string des = (desayunoIdx != -1)
        ? to_string(bd_idAlimento(con, listaAlimentos[desayunoIdx].nombre)) : "NULL";
    string alm = (almuerzoIdx != -1)
        ? to_string(bd_idAlimento(con, listaAlimentos[almuerzoIdx].nombre)) : "NULL";
    string cen = (cenaIdx != -1)
        ? to_string(bd_idAlimento(con, listaAlimentos[cenaIdx].nombre)) : "NULL";

    // Fecha de hoy
    time_t t = time(nullptr);
    char fecha[11];
    struct tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &t);
#else
    localtime_r(&t, &tm_info);
#endif
    strftime(fecha, sizeof(fecha), "%Y-%m-%d", &tm_info);

    // Upsert: si ya existe registro del dia, actualiza; si no, inserta
    string q =
        "INSERT INTO registro_dieta "
        "(id_usuario, desayuno_id, almuerzo_id, cena_id, fecha, "
        " total_kcal, total_proteina, total_carbohidratos, total_grasa) "
        "VALUES (" +
        to_string(idUsuario) + "," + des + "," + alm + "," + cen + ", '" +
        string(fecha) + "'," +
        to_string(totalKcal) + "," + to_string(totalProt) + "," +
        to_string(totalCarb) + "," + to_string(totalGras) +
        ") ON DUPLICATE KEY UPDATE "
        "desayuno_id=VALUES(desayuno_id), almuerzo_id=VALUES(almuerzo_id), "
        "cena_id=VALUES(cena_id), total_kcal=VALUES(total_kcal), "
        "total_proteina=VALUES(total_proteina), "
        "total_carbohidratos=VALUES(total_carbohidratos), "
        "total_grasa=VALUES(total_grasa)";

    if (mysql_query(con, q.c_str()))
        cout << "Error al guardar dieta: " << mysql_error(con) << "\n";
    else
        cout << "Dieta guardada en la base de datos.\n";
}