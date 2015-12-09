/*
 * MoneyBox 11Paths (c)Nov2015
 * 
 * Copyright (C) 2013 Eleven Paths
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef MB_JSON_H
#define MB_JSON_H

#include <aJSON.h>

#define FILE_BUFFER_SIZE 1024*128 // json file read buffer

#define FILE_JSON "/home/root/moneybox.json"

#define JSON_ROOT                  "moneybox"
#define JSON_ID                    "id"
#define JSON_LATCH_ACCOUNTS_ARRAY  "latch_accounts"
#define JSON_LATCH_ACCOUNT         "latch_id"
#define JSON_LATCH_OPER            "operation"
#define JSON_LATCH_MONEY           "money"

// lee el archivo json y lo parsea, si todo esta ok, devuelve un puntero al objeto json, si algo falla devuelve NULL
aJsonObject* read_json_file(char* file_name);

// devuelve el número de ids que contiene el json.
int num_ids(aJsonObject* jsonObject);

// devuelve el número de indice del array que contiene las ids de la id dada; si no esta o algo falla, devuelve un valor negativo.
int id_index(aJsonObject* jsonObject,int id);

// devuelve el numero de latchAccounts que tiene definida una id por su index
int num_latchids(aJsonObject* jsonObject,int index);

// devuelte el LatchAccount de indice index_latch del array de accounts, de la id de index index_id
char* get_latchid(aJsonObject* jsonObject,int index_id, int index_latch);

// devuelte true si la operation del elemento de indice index es AND
boolean latch_and(aJsonObject* jsonObject,int index);

// devuelve el valor string que enviar a la moneybox para dispensar
char* get_money(aJsonObject* jsonObject,int index_id);

// No usada
// devuelve verdadero si el json de respuesta a la operacion_id está a on
boolean latch_state(aJsonObject* jsonResponse,char* operation_id);

#endif //MB_JSON
