//Tipos.h
#pragma once
#include <string>

struct Alimento {
    std::string nombre;
    int kcal;
    double proteina;
    double carbohidratos;
    double grasa;

    // Default constructor
    Alimento() noexcept
        : nombre(""), kcal(0), proteina(0.0), carbohidratos(0.0), grasa(0.0) {}

    // Parameterized constructor used when creating with brace-init like {"name", 100, 5.0, 20.0, 1.0}
    Alimento(const std::string& nombre_, int kcal_, double proteina_, double carbohidratos_, double grasa_) noexcept
        : nombre(nombre_), kcal(kcal_), proteina(proteina_), carbohidratos(carbohidratos_), grasa(grasa_) {}
};
