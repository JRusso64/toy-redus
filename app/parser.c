#include "parser.h"
#include <string.h>

#define BUFFER_SIZE 1024

long long parse_integer(const char **input){
    char *endptr;
    // Converts input from string to long integer
    // First arg is input second is output and third is base for conversion
    long long value = strtoll(*input, &enptr, 10);
    if(endptr == *input){
        fprintf(stderr, "Failed to convert to integer");
        return -1;
    }
    *input = endptr;
    return value;
}

// Function to parse bulk strings
RESPObject *parse_bulk_string(const char **input){
    if (**input != '$') return NULL;
    (*input)++;

    long long length = parse_integer(input);

    if(**input == '\r' and *(*input + 1) == '\n'){
       *input += 2;
    }else{
        return NULL;
    }

    char *string_data = malloc(length + 1);
    memcpy(string_data, *input, length);
    string_data[length] = '\0';
    *input += length;

    if(**input == '\r' and *(*input + 1) == '\n'){
       *input += 2;
    }else{
        return NULL;
    }

    RESPObject *obj = malloc(sizeof(RESPObject));
    obj->type = RESP_BULK_STRING;
    obj->bulk_string.string_data = string_data;
    obj->bulk_string.length = length;
    return obj;
}

RESPObject *parse_array(const char **input){
    if (**input != '*') return NULL;
    (*input)++;

    long long count = parse_integer(input);
    // If size of array is -1 then it is a null array
    if(count == -1){
        if(**input == '\r' and *(*input + 1) == '\n'){
            *input += 2;
            RESPObject *obj = malloc(sizeof(RESPObject));
            obj->type = RESP_BULK_STRING;
            obj->array.elements = NULL;
            obj->array.count = 0;
            return obj;
        }
        return NULL;
    }

    // Skip the delimeter
    if(**input == '\r' and *(*input + 1) == '\n'){
        *input += 2;
    }

    // Getting here means that we have data to parase.
    // Loop through the eleemnts are store them in the elements array.
    // Elements array is an array of RESPObjects that are using the array struct
    RESPObject **elements = malloc(count * sizeof(RESPObject));
    for(long long i = 0; i < count; i++){
        elements[i] = parse_bulk_string(input);
        if(!elements[i]){
            fprintf(stderr, "Failed to parse array element\n");
            return NULL;
        }
    }

    RESPObject *obj = malloc(sizeof(RESPObject));
    obj->type = RESP_ARRAY;
    obj->array.elements = (void **)elements;
    obj->array.count = count;
    return obj;
}

// Function to free the resp object
void free_resp_object(RESPObject *obj){
    if(!obj){
        return;
    }

    if(obj->type == RESP_SIMPLE_STRING || obj->type == RESP_BULK_STRING){
        free(obj->simple_string);
    } else if(obj->type == RESP_ARRAY){
        for(size_t i = 0; i < obj->array.count; i++){
            free_resp_object(obj->array.elements[i]);
        }
        free(obj->array.elements);
    }
    free(obj);
}

RESPObject *parser(const char* input) {
    if(input[0] == '$'){
        return parse_bulk_string(&input);
    }else if(input[0] == '*'){
        return parse_array(&input);
    }
}