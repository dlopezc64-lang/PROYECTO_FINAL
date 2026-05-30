// ConexionBD.h
#pragma once
#include <mysql.h>
#include <iostream>
using namespace std;

class ConexionBD {
private:
    MYSQL* conexion;

public:
    ConexionBD() {
        conexion = mysql_init(0);
    }

    void abrir_conexion() {
        conexion = mysql_real_connect(
            conexion, "localhost", "root",
            "Khali-0723", "BD_sistemaNutricionUmg",
            3306, NULL, 0
        );
        if (conexion) cout << "Conexion Exitosa\n";
        else          cout << "Error de Conexion\n";
    }

    MYSQL* get_conexion() { return conexion; }

    ~ConexionBD() {
        if (conexion) mysql_close(conexion);
    }
};
