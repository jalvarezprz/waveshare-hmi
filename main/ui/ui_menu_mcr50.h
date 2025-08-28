#pragma once

// declaración adelantada; no necesita cJSON aquí
struct cJSON;

cJSON* loadMenuMcr50();
void   printMenuMcr50();

// Carga el menú embebido del MCR50 y lo devuelve como objeto cJSON
// (debes liberar con cJSON_Delete() después de usarlo)
cJSON* loadMenuMcr50();

// Solo para depuración: imprime los menús de primer nivel
void printMenuMcr50();
