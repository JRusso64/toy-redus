#ifndef PARSER_H
#define PARSER_H

#include <stdlib.h>
#include <stdio.h>

// Enum for RESP data types
typedef enum {
    RESP_SIMPLE_STRING,
    RESP_ERROR,
    RESP_INTEGER,
    RESP_BULK_STRING,
    RESP_ARRAY,
    RESP_INVALID
} RESPType;

// RESP Object struct
typedef struct {
    RESPType type;
    union {
        char *simple_string;
        char *error_message;
        long long integer_value;
        struct {
            char *string_data;
            size_t length;
        } bulk_string;
        struct {
            void **elements;
            size_t count;
        } array;
    };
} RESPObject;

// Function prototypes
RESPObject *parser(const char *input);
RESPObject *parse_bulk_string(const char **input);
RESPObject *parse_array(const char **input);
void free_resp_object(RESPObject *obj);

#endif // PARSER_H
