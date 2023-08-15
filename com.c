#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <sys/socket.h>
//#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>

#define IDENTIFIER              -2      // abc
#define STRING                  -3      // "foo" 'bar'
#define EOS                     -4      // 
#define INT                     -5      // 123
#define NUMBER                  -6      // 123.456
#define UNDEFINED               -7      // undefined
#define TRUE                    -8      // true
#define FALSE                   -9      // false
#define L_NULL                  -10     // null
#define EOG                     -11     // )
#define EOA                     -12     // ]
#define EOAP                    -13     // INDEX]
#define SELF                    -14     // self

#define INCREMENT               -15     // ++
#define DECREMENT               -16     // --

#define EOL                     -32     // ;
#define BLOCK                   -33     // {
#define EOB                     -34     // }

#define IF                      -35
#define ELSE                    -36
#define WHILE                   -37
#define DO                      -38
#define FOR                     -39
#define SWITCH                  -40
#define CASE                    -41
#define DEFAULT                 -42
#define REUTRN                  -43
#define BREAK                   -44
#define CONTINUE                -45
#define CLASS                   -46
#define FUNC                    -47
#define VAR                     -48

#define PERIOD                  -64     // .

#define COMMA                   -65     // ,
#define GROUP                   -66     // (
#define DICT                    -67     // [KEY:VALUE,..
#define ARRAY                   -68     // [
#define ARRAYPARAM              -69     // [INDEX
#define ARRAYKEY                -70     // KEY:
#define CONDITIONAL_ELSE        -71     // :
#define ADDITION                -72     // +
#define SUBTRACTION             -73     // -
#define EXPONENTIATION          -74     // **
#define MULTIPLICATION          -75     // *
#define DIVISION                -76     // /
#define REMAINDER               -77     // %
#define LESS_THAN               -78     // <
#define LESS_EQUAL              -79     // <=
#define LEFT_SHIFT              -80     // <<
#define GREATER_THAN            -81     // >
#define GREATER_EQUAL           -82     // >=
#define RIGHT_SHIFT             -83     // >>
#define EQUALITY                -84     // ==
#define INEQUALITY              -85     // !=
#define NOT                     -86     // !
#define AND                     -87     // &&
#define OR                      -88     // ||
#define BITWISE_NOT             -89     // ~
#define BITWISE_AND             -90     // &
#define BITWISE_OR              -91     // |
#define BITWISE_XOR             -92     // ^
#define FUNCTION_END            -93     // )
#define CONDITIONAL             -94     // ?
#define ARROW                   -95     // ->
#define FUNCTION                -96     // (

#define ASSIGNMENT              -97     // =
#define ADDITION_ASSIGN         -98     // +=
#define SUBTRACTION_ASSIGN      -99     // -=
#define EXPONENTIATION_ASSIGN   -100    // **=
#define MULTIPLICATION_ASSIGN   -101    // *=
#define DIVISION_ASSIGN         -102    // /=
#define REMAINDER_ASSIGN        -103    // %=
#define LEFT_SHIFT_ASSIGN       -104    // <<=
#define RIGHT_SHIFT_ASSIGN      -105    // >>=
#define AND_ASSIGN              -106    // &&=
#define OR_ASSIGN               -107    // ||=
#define BITWISE_AND_ASSIGN      -108    // &=
#define BITWISE_OR_ASSIGN       -109    // |=
#define BITWISE_XOR_ASSIGN      -110    // ^=

/**
 * 
 */
#define type_undefined      -1
#define type_null           0
#define type_string         1
#define type_number         2
#define type_boolean        3
#define type_array          4
#define type_dictionary     5
#define type_function       6
#define type_object         7
#define type_file_stream        8
#define type_socket_descriptor  9

#define set_value(__value, __type, __data) __value.data = __data, __value.type = &__type
#define add_ident(__obj, __id, __type, __data) __obj.object[__obj.items].id = __id, __obj.object[__obj.items].value.type = __type, __obj.object[__obj.items].value.data = __data, __obj.items ++

typedef struct value Value;
typedef struct ident Ident;
typedef struct type Type;
typedef struct ctrl Ctrl;

struct type {
    char name;
    // char *id;
    int items;
    Ident *object;
};

struct value {
    Type *type;
    void *data;
};

struct ident {
    char *id;
    Value value;
};

struct ctrl {
    char type;
    Value value;
};

void skip_expression(char **ptr);

void skip_code(char **ptr) {
    switch (**ptr) {
        case BLOCK: {
            (*ptr) ++;
            do skip_code(ptr); while (**ptr != EOB);
            (*ptr) ++;
            return;
        }
        case REUTRN: {
            do (*ptr) ++, skip_expression(ptr); while (**ptr == COMMA);
            if (**ptr == EOL) (*ptr) ++;
            return;
        }
        case WHILE: {
            (*ptr) ++, skip_expression(ptr), skip_code(ptr);
            return;
        }
        case IF: {
            (*ptr) ++, skip_expression(ptr), skip_code(ptr);
            if (**ptr == ELSE) (*ptr) ++, skip_code(ptr);
            return;
        }
        case SWITCH:
        case CASE: {
            (*ptr) ++, skip_expression(ptr), skip_code(ptr);
            if (**ptr == CASE) (*ptr) ++, skip_expression(ptr), skip_code(ptr);
            else if (**ptr == DEFAULT) (*ptr) ++, skip_code(ptr);
            return;
        }
        case ELSE:
        case DEFAULT: {
            (*ptr) ++, skip_code(ptr);
            return;
        }
        case BREAK: 
        case CONTINUE:{
            (*ptr) ++;
            if (**ptr == EOL) (*ptr) ++;
            return;
        }
        default: {
            skip_expression(ptr);
            while (**ptr == COMMA) (*ptr) ++, skip_expression(ptr);
            if (**ptr == EOL) (*ptr) ++;
            return;
        }
    }
}

void skip_assign(char **ptr) {
    if (**ptr <= ASSIGNMENT) {
        (*ptr)++, skip_expression(ptr);
    }
    return;
}

void skip_value(char **ptr) {
    switch (**ptr) {
        case IDENTIFIER: {
            do (*ptr) ++; while (*(*ptr-1) != 0);
            if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) skip_assign(ptr);
            break;
        }
        case NUMBER: case INT: {
            do (*ptr) ++; while (*(*ptr-1) != 0);
            break;
        }
        case STRING: {
            do (*ptr) ++; while (*(*ptr-1) != EOS);
            break;
        }
        case GROUP: case ARRAY: {
            do (*ptr)++, skip_expression(ptr); while (**ptr == COMMA);
            (*ptr)++;
            break;
        }
        case BLOCK: {
            skip_code(ptr);
            break;
        }
        case TRUE: case FALSE: case L_NULL: case UNDEFINED: case SELF: {
            (*ptr)++;
            break;
        }
        case ADDITION: case SUBTRACTION: case INCREMENT: case DECREMENT: {
            (*ptr)++, skip_value(ptr);
            break;
        }
        case FUNCTION: {
            do (*ptr) ++; while (*(*ptr-1) != ARROW);
            skip_code(ptr);
            break;
        }
    }

    while (**ptr == PERIOD || **ptr == GROUP || **ptr == ARRAY) {
        if (**ptr == PERIOD) {
            (*ptr) += 2;
            do (*ptr) ++; while (*(*ptr-1) != 0);
            if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) skip_assign(ptr);
        }
        else if (**ptr == GROUP) {
            do (*ptr)++, skip_expression(ptr); while (**ptr == COMMA);
            (*ptr)++;
        }
        else {
            do (*ptr)++, skip_expression(ptr); while (**ptr == COMMA);
            (*ptr)++;
            if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) skip_assign(ptr);
        }
    }

    return;
}

void skip_expression(char **ptr) {
    skip_value(ptr); 
    while (**ptr <= ADDITION) {
        if (**ptr == CONDITIONAL) (*ptr)++, skip_expression(ptr), (*ptr)++, skip_expression(ptr);
        else (*ptr)++, skip_value(ptr);
    }
    return;
}

Type undefined, null, string, number, boolean, array, dictionary, function, files, socketd;

Value expression(char **ptr, Value *self, Type *storage);

double *numdup(double num) {
    double *res = malloc(sizeof(double));
    *res = num;
    return res;
}

bool *booldup(bool val) {
    bool *res = malloc(sizeof(bool));
    *res = val;
    return res;
}

size_t arrlen(Value *array) {
    size_t length = 0;
    while (array->type->name != type_undefined) array ++, length ++;
    return length;
}

Value valdup(Value val);

Value *arrdup(Value *array) {
    Value *arr = malloc(sizeof(Value) * (arrlen(array) + 1)), *res = arr;
    while (array->type->name != type_undefined) *arr = valdup(*array), arr ++, array ++;
    arr->type = &undefined;
    return res;
}

Value valdup(Value val) {
    switch (val.type->name) {
        case type_string: val.data = strdup(val.data);
        return val;
        case type_number: val.data = numdup(*(double *)val.data);
        return val;
        case type_boolean: val.data = booldup(*(bool *)val.data);
        return val;
        case type_array: val.data = arrdup(val.data);
        return val;
        default: return val;
    }
}

bool valcmp(Value val1, Value val2) {
    if (val1.type->name != val2.type->name) return 0;
    switch (val1.type->name) {
        case type_string: return strcmp(val1.data, val2.data) == 0;
        case type_number: return *(double *)val1.data == *(double *)val2.data;
        case type_boolean: return *(bool *)val1.data == *(bool *)val2.data;
        case type_array: {
            Value *arr1 = val1.data, *arr2 = val2.data;
            while (valcmp(*arr1, *arr2)) {
                if (arr1->type->name == type_undefined) return 1;
                arr1 ++, arr2 ++;
            }
            return 0;
        }
        case type_null:
        case type_undefined: return 1;
        default: return 0;
    }
}

char *strdata(Value val) {
    switch (val.type->name) {
        case type_string: return val.data;
        case type_number: {
            if (__inline_isnand(*(double *)val.data)) return "NaN";
            if (__inline_isinfd(*(double *)val.data)) return *(double *)val.data > 0 ? "Infinity" : "-Infinity";
            char* str = malloc(sizeof(char) * 19), *result = str;

            double value = *(double *)val.data >= 0 ? *(double *)val.data : (*str ++ = '-', -*(double *)val.data);
            long length = value, int_part = value, decimal_part = (value - int_part) * 1e16;
            if (int_part == 0) *str = '0', *(str + 1) = 0;
            else {
                while (length > 0) length /= 10, str ++;
                *str -- = 0;
                while (int_part != 0) *str -- = (int_part % 10) + '0', int_part /= 10, decimal_part /= 10, length ++;
            }

            if (decimal_part != 0) {
                length = 16 - length, str += 17, *(str - length) = '.';
                if (decimal_part % 10 == 9) {
                    do str --, decimal_part /= 10, length --; while (decimal_part % 10 == 9);
                    *(str + 1) = 0, *str -- = (decimal_part % 10) + '1', decimal_part /= 10, length --;
                    while (length != 0) *str -- = (decimal_part % 10) + '0', decimal_part /= 10, length --;
                } else {
                    while (decimal_part % 10 == 0) str --, decimal_part /= 10, length --;
                    *(str + 1) = 0;
                    while (length != 0) *str -- = (decimal_part % 10) + '0', decimal_part /= 10, length --;
                }
            }
            return result;
        }
        case type_boolean: return *(bool *)val.data ? "true" : "false";
        case type_array: {
            size_t length = 0, max = 3;
            char *str = malloc(sizeof(char) * max), *result = str;
            Value *arr = val.data;
            length ++, *str ++ = '[';
            while (arr->type->name != type_undefined) {
                char *item = strdata(*arr);
                size_t len = strlen(item);
                if (arr->type->name == type_string) {
                    max += len + 2, result = realloc(result, sizeof(char) * max), str = result + length;
                    length ++, *str ++ = '"';
                    while (*item != 0) length ++, *str ++ = *item ++;
                    length ++, *str ++ = '"';
                } else {
                    max += len, result = realloc(result, sizeof(char) * max), str = result + length;
                    while (*item != 0) length ++, *str ++ = *item ++;
                }
                length ++, *str ++ = ',', arr ++;
            }
            if (*(str - 1) == ',') str --;
            length ++, *str ++ = ']', *str = 0;
            return result;
        }
        case type_null: return "null";
        default: return "";
    }
}

double numdata(Value val) {
    switch (val.type->name) {
        case type_number: return *(double *)val.data;
        case type_string: {
            char *end;
            double result = strtod(val.data, &end);
            return *end == 0 ? result : NAN;
        }
        case type_boolean: return *(bool *)val.data;
        default: return 0;
    }
}

int intdata(Value val) {
    switch (val.type->name) {
        case type_number: return __inline_isnormald(*(double *)val.data) ? *(double *)val.data : 0;
        case type_string: return atoi(val.data);
        case type_boolean: return *(bool *)val.data;
        default: return 0;
    }
}

bool booldata(Value val) {
    switch (val.type->name) {
        case type_boolean: return *(bool *)val.data;
        #define is_TRUE(c) (c[0] != 'f' || c[1] != 'a' || c[2] != 'l' || c[3] != 's' || c[4] != 'e' || c[5] != 0) && (c[0] != '0' || c[1] != 0)
        case type_string: return is_TRUE(((char *)val.data));
        #undef is_TRUE
        case type_number: return *(double *)val.data;
        case type_null:
        case type_undefined: return false;
        default: return true;
    }
}

#define ctrl_value 1
#define ctrl_return 2
#define ctrl_continue 3
#define ctrl_break 4

Ctrl code(char **ptr, Value *self, Type *storage) {
    Ctrl res;
    switch (**ptr) {
        case BLOCK: {
            int items = storage->items;
            (*ptr) ++;
            do res = code(ptr, self, storage); while (**ptr != EOB && res.type == ctrl_value);
            do (*ptr)++; while (*(*ptr-1) != EOB);
            storage->items = items;
            if (res.type != ctrl_return) res.value.data = NULL, res.value.type = &undefined;
            return res;
        }
        case REUTRN: {
            res.type = ctrl_return;
            do (*ptr) ++, res.value = expression(ptr, self, storage); while (**ptr == COMMA);
            if (**ptr == EOL) (*ptr) ++;
            return res;
        }
        case BREAK: {
            (*ptr) ++, res.type = ctrl_break;
            if (**ptr == EOL) (*ptr) ++;
            return res;
        }
        case CONTINUE: {
            (*ptr) ++, res.type = ctrl_continue;
            if (**ptr == EOL) (*ptr) ++;
            return res;
        }
        case ELSE:
        case DEFAULT: {
            (*ptr) ++, skip_code(ptr), res.type = ctrl_value;
            return res;
        }
        case CASE: {
            (*ptr) ++, skip_expression(ptr), skip_code(ptr), res.type = ctrl_value;
            return res;
        }
        case IF: {
            (*ptr) ++;
            if (booldata(expression(ptr, self, storage))) return code(ptr, self, storage);
            skip_code(ptr);
            if (**ptr == ELSE) {
                (*ptr) ++;
                return code(ptr, self, storage);
            }
            res.type = ctrl_value;
            return res;
        }
        case WHILE: {
            (*ptr) ++;
            char *p = *ptr;
            while (booldata(expression(ptr, self, storage))) {
                res = code(ptr, self, storage);
                if (res.type == ctrl_return) return res;
                else if (res.type == ctrl_break) {
                    res.type = ctrl_value;
                    return res;
                }
                (*ptr) = p;
            }
            skip_code(ptr), res.type = ctrl_value;
            return res;
        }
        case SWITCH: {
            (*ptr) ++;
            Value model = expression(ptr, self, storage);
            while (**ptr == CASE) {
                (*ptr) ++;
                if (valcmp(model, expression(ptr, self, storage))) return code(ptr, self, storage);
                skip_code(ptr);
            }
            if (**ptr == DEFAULT) {
                (*ptr) ++;
                return code(ptr, self, storage);
            }
            res.type = ctrl_value;
            return res;
        }
        case CLASS: {
            #define THIS storage->object[storage->items]
            THIS.id = *ptr+1,
            THIS.value.type = malloc(sizeof(Type)),
            THIS.value.type->items = 0,
            THIS.value.type->name = type_object,
            THIS.value.type->object = malloc(sizeof(Ident) * BUFSIZ),
            THIS.value.data = NULL;
            do (*ptr) ++; while (*(*ptr-1) != 0);
            code(ptr, &THIS.value, THIS.value.type);
            #undef THIS
            res.type = ctrl_value;
            return res;
        }
        default: {
            res.type = ctrl_value, res.value = expression(ptr, self, storage);
            while (**ptr == COMMA) (*ptr) ++, res.value = expression(ptr, self, storage);
            if (**ptr == EOL) (*ptr) ++;
            return res;
        }
    }
}

Value builtin_func(char *id, int items, Value *param, Value *self) {
    Value res;
    switch (id[0]) {

        case type_null: switch (id[1]) {
            case 1: { // print(String)
                puts(items > 0 ? strdata(param[0]) : ""), res.type = &undefined;
                return res;
            }
            case 2: { // typeof(Any) -> String
                res.type = &string;
                if (items == 0) {
                    res.data = "";
                    return res;
                }
                switch (param[0].type->name) {
                    case type_string: res.data = "String";
                        return res;
                    case type_number: res.data = "Number";
                        return res;
                    case type_boolean: res.data = "Boolean";
                        return res;
                    case type_array: res.data = "Array";
                        return res;
                    case type_dictionary: res.data = "Dictionary";
                        return res;
                    case type_function: res.data = "Function";
                        return res;
                    case type_object: res.data = "Object";
                        return res;
                    case type_file_stream: res.data = "FileStream";
                        return res;
                    case type_socket_descriptor: res.data = "SocketDescriptor";
                        return res;
                    default: res.data = ""; return res;
                }
            }
            case 3: { // exit(Number)
                exit(items > 0 ? intdata(param[0]) : 0), res.type = &undefined;
                return res;
            }
            case 4: { // gets(String) -> String
                fputs(items > 0 ? strdata(param[0]) : "", stdout);
                res.type = &string, res.data = malloc(BUFSIZ);
                fgets(res.data, BUFSIZ, stdin), *((char *)res.data + strlen(res.data) - 1) = 0;
                return res;
            }
            case 5: { // puts(String)
                fputs(items > 0 ? strdata(param[0]) : "", stdout), res.type = &undefined;
                return res;
            }
        }

        case type_string: switch (id[1]) {
            case '\x1': { // String.substr(start, length) -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    int length = items > 1 ? intdata(param[1]) : 0;
                    int start = items > 0 ? intdata(param[0]) : 0, selflen = strlen(self->data); 
                    while (start < 0) start += selflen;
                    res.data = malloc(sizeof(char) * (length + 1)), memcpy(res.data, self->data + start, length), *(char *)(res.data + length) = 0;
                } else res.data = "";
                return res;
            }
            case '\x2': { // String.startsWith(prefix) -> Boolean
                res.type = &boolean; 
                if (self->type->name == type_string) {
                    char *prefix = items > 0 ? strdata(param[0]) : "";
                    res.data = booldup(strncmp(self->data, prefix, strlen(prefix)) == 0);
                } else res.data = booldup(0);
                return res;
            }
            case '\x3': { // String.endsWith(prefix) -> Boolean
                res.type = &boolean;
                if (self->type->name == type_string) {
                    char *prefix = items > 0 ? strdata(param[0]) : "";
                    res.data = booldup(strcmp(self->data + strlen(self->data) - strlen(prefix), prefix) == 0);
                } else res.data = booldup(0);
                return res;
            }
            case '\x4': { // String.contains(substr) -> Boolean
                res.type = &boolean, res.data = booldup(self->type->name == type_string ? strstr(self->data, items > 0 ? strdata(param[0]) : "") != NULL : 0); 
                return res;
            }
            case '\x5': { // String.indexOf(substr) -> Number
                res.type = &number;
                if (self->type->name == type_string) {
                    char* str = strstr(self->data, items > 0 ? strdata(param[0]) : "");
                    if (str != NULL) {
                        res.data = numdup(str - (char *)self->data);
                        return res;
                    }
                }
                res.data = numdup(-1);
                return res;
            }
            case '\6': { // String.padStart(length, padstr) -> String
                res.type = &string;
                if (self->type->name != type_string) res.data = "";
                else {
                    int length = items > 0 ? intdata(param[0]) : 0, curlen = strlen(self->data);
                    if (curlen < length) {
                        char *padstr = items > 1 ? strdata(param[1]) : "";
                        if (*padstr != 0) {
                            char *str = res.data = malloc(sizeof(char) * (length + 1));
                            int padlen = strlen(padstr);
                            while (((char *)res.data)[length - curlen] == 0) strcpy(str, padstr), str += padlen;
                            strcpy(&((char *)res.data)[length - curlen], self->data);
                            return res;
                        }
                    }
                    res.data = self->data;
                }
                return res;
            }
            case '\7': { // String.padEnd(length, padstr) -> String
                res.type = &string;
                if (self->type->name != type_string) res.data = "";
                else {
                    int length = items > 0 ? intdata(param[0]) : 0, curlen = strlen(self->data);
                    if (curlen < length) {
                        char *padstr = items > 1 ? strdata(param[1]) : "";
                        if (*padstr != 0) {
                            char *str = res.data = malloc(sizeof(char) * (length + 1));
                            int padlen = strlen(padstr);
                            strcpy(str, self->data), str += curlen;
                            while (((char *)res.data)[length - 1] == 0) strcpy(str, padstr), str += padlen;
                            ((char *)res.data)[length] = 0;
                            return res;
                        }
                    }
                    res.data = self->data;
                }
                return res;
            }
            case '\x8': { // String.repeat(count) -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    int count = items > 0 ? intdata(param[0]) : 0, length = strlen(self->data);
                    if (count > 0) {
                        char *str = res.data = malloc(sizeof(char) * ((length * count) + 1));
                        while (count > 0) strcpy(str, self->data), str += length, count --;
                        *str = 0;
                        return res;
                    }
                }
                res.data = "";
                return res;
            }
            case '\x9': { // String.replace(String, String) -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    char *old = items > 0 ? strdata(param[0]) : "";
                    if (old[0] != 0 && strstr(self->data, old) != NULL) {
                        char *new = items > 1 ? strdata(param[1]) : "";
                        int oldlen = strlen(old), newlen = strlen(new), length = strstr(self->data, old) - (char *)self->data;
                        res.data = malloc(sizeof(char) * (strlen(self->data) + newlen - oldlen + 1)), memcpy(res.data, self->data, length), memcpy(res.data + length, new, newlen), strcpy(res.data + length + newlen, self->data + length + oldlen);
                    } else res.data = self->data;
                } else res.data = "";
                return res;
            }
            case '\xa': { // String.replaceAll(String, String) -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    char *old = items > 0 ? strdata(param[0]) : "";
                    if (old[0] != 0) {
                        char *ptr = self->data, *new = items > 1 ? strdata(param[1]) : "";
                        int oldlen = strlen(old), newlen = strlen(new), length = 0, count = 0;
                        while ((ptr = strstr(ptr, old)) != NULL) ptr += oldlen, count ++;
                        char *str = res.data = malloc(sizeof(char) * (strlen(ptr = self->data) + (newlen - oldlen) * count + 1));
                        while (count > 0) length = strstr(ptr, old) - ptr, memcpy(str, ptr, length), str += length, memcpy(str, new, newlen), str += newlen, ptr += length + oldlen, count --;
                        strcpy(str, ptr);
                    } else res.data = self->data;
                } else res.data = "";
                return res;
            }
            case '\xb': { // String.split(prefix) -> Array
                res.type = &array;
                Value *arr;
                if (self->type->name == type_string) {
                    char *ptr = self->data, *prefix = items > 0 ? strdata(param[0]) : "";
                    int prelen = strlen(prefix), count = 0, length = strlen(ptr);
                    if (*prefix != 0) while ((ptr = strstr(ptr, prefix)) != NULL) ptr += prelen, count ++;
                    arr = res.data = malloc(sizeof(Value) * (count + 2)), ptr = self->data;
                    while (count > 0) length = strstr(ptr, prefix) - ptr, arr->type = &string, arr->data = malloc(sizeof(char) * (length + 1)), memcpy(arr->data, ptr, length), ptr += length + prelen, arr ++, count --;
                    arr->type = &string, arr->data = malloc(sizeof(char) * (length + 1)), memcpy(arr->data, ptr, length), arr ++, arr->type = &undefined;
                } else arr = res.data = malloc(sizeof(Value)), arr->type = &undefined;
                return res;
            }
            case '\xc': { // String.charAt(index) -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    int index = items > 0 ? intdata(param[0]) : 0, length = strlen(self->data);
                    while (index < 0) index += length;
                    res.data = malloc(sizeof(char) * 2), *(char *)res.data = *((char *)self->data + index);
                } else res.data = "";
                return res;
            }
            case '\xd': { // String.charCodeAt(index) -> Number
                res.type = &number;
                if (self->type->name == type_string) {
                    int index = items > 0 ? intdata(param[0]) : 0, length = strlen(self->data);
                    while (index < 0) index += length;
                    res.data = numdup(*((char *)self->data + index));
                } else res.data = numdup(0);
                return res;
            }
        }

        case type_number: switch (id[1]) {
            case '\x1': { // Number.toString(base) -> String
                res.type = &string;
                if (self->type->name == type_number) {
                    int base = items > 0 ? intdata(param[0]) : 0;
                    if (2 <= base && base <= 36) {
                        if (__inline_isnand(*(double *)self->data)) res.data = "NaN";
                        else if (__inline_isinfd(*(double *)self->data)) res.data = *(double *)self->data > 0 ? "Infinity" : "-Infinity";
                        else if (*(double *)self->data == 0) res.data = "0";
                        else {
                            long num = *(double *)self->data, length = num < 0 ? 1 : 0;
                            while (num != 0) num /= base, length ++;
                            char *str = (res.data = malloc(sizeof(char) * (length + 1))) + length - 1;
                            if ((num = *(double *)self->data) < 0) *(str - length + 1) = '-', num = -num;
                            while (num != 0) *str -- = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ"[num % base], num /= base;
                        }
                        return res;
                    }
                }
                res.data = "";
                return res;
            }
        }

        case type_object: switch (id[1]) {
            case 1: switch (id[2]) { // File
                case 1: { // File.open(String, String) -> FileStream | null
                    res.type = &files, res.data = fopen(items > 0 ? strdata(param[0]) : "", items > 1 ? strdata(param[1]) : "");
                    if (res.data == NULL) res.type = &null;
                    return res;
                }
                case 2: { // File.read(String) -> String
                    FILE *fp = fopen(items > 0 ? strdata(param[0]) : "", "r");
                    fseek(fp, 0, SEEK_END);
                    long length = ftell(fp);
                    fseek(fp, 0, SEEK_SET), res.type = &string, res.data = malloc(sizeof(char) * (length + 1)), fread(res.data, sizeof(char), length, fp), fclose(fp);
                    return res;
                }
                case 3: { // File.append(String, String)
                    res.type = &undefined, res.data = NULL;
                    FILE *fp = fopen(items > 0 ? strdata(param[0]) : "", "a");
                    if (fp != NULL && items > 1) {
                        char *str = strdata(param[1]);
                        fwrite(str, sizeof(char), strlen(str), fp), fclose(fp);
                    }
                    return res;
                }
                case 4: { // File.write(String, String)
                    res.type = &undefined, res.data = NULL;           
                    FILE *fp = fopen(items > 0 ? strdata(param[0]) : "", "w");
                    if (fp != NULL && items > 1) {
                        char *str = strdata(param[1]);
                        fwrite(str, sizeof(char), strlen(str), fp), fclose(fp);
                    }
                    return res;
                }
                case 5: { // File.remove(String) -> Boolean
                    res.type = &boolean, res.data = booldup(remove(items > 0 ? strdata(param[0]) : ""));
                    return res;
                }
                case 6: { // File.exist(String) -> Boolean
                    FILE *fp = fopen(items > 0 ? strdata(param[0]) : "", "r");
                    res.type = &boolean, res.data = booldup(fp != NULL);
                    if (*(bool *)res.data) fclose(fp);
                    return res;
                }
                case 7: { // File.rename(String, String) -> Boolean
                    res.type = &boolean, res.data = booldup(rename(items > 0 ? strdata(param[0]) : "", items > 1 ? strdata(param[1]) : ""));
                    return res;
                }
                case '\10': { // File.fullpath(String) -> String
                    res.type = &string, res.data = realpath(items > 0 ? strdata(param[0]) : "", NULL);
                    if (res.data == NULL) res.data = "";
                    return res;
                }
                case '\11': { // File.size(String) -> Number
                    FILE *fp = fopen(items > 0 ? strdata(param[0]) : "", "rb");
                    if (fp != NULL) fseek(fp, 0, SEEK_END), res.type = &number, res.data = numdup(ftell(fp)), fclose(fp);
                    else res.type = &number, res.data = numdup(0);
                    return res;
                }
                case '\12': { // File.stat(String) -> FileStat
                    
                }
            }
            case 2: switch (id[2]) { // Math
                case '\x1': { // Math.abs(Number) -> Number
                    res.type = &number, res.data = numdup(fabs(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x2': { // Math.pow(Number, Number) -> Number
                    res.type = &number, res.data = numdup(pow(items > 0 ? numdata(param[0]) : 0, items > 1 ? numdata(param[1]) : 0));
                    return res;
                }
                case '\x3': { // Math.sqrt(Number) -> Number
                    res.type = &number, res.data = numdup(sqrt(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x4': { // Math.cbrt(Number) -> Number
                    res.type = &number, res.data = numdup(cbrt(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x5': { // Math.hypot(Number, Number) -> Number
                    res.type = &number, res.data = numdup(hypot(items > 0 ? numdata(param[0]) : 0, items > 1 ? numdata(param[1]) : 0));
                    return res;
                }
                case '\x6': { // Math.ceil(Number) -> Number
                    res.type = &number, res.data = numdup(ceil(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x7': { // Math.floor(Number) -> Number
                    res.type = &number, res.data = numdup(floor(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x8': { // Math.round(Number) -> Number
                    res.type = &number, res.data = numdup(round(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }

                case '\xa': { // Math.isFinite(Number) -> Boolean
                    res.type = &boolean, res.data = booldup(__inline_isfinited(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\xc': { // Math.isNaN(Number) -> Boolean
                    res.type = &boolean, res.data = booldup(__inline_isnand(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }

                case '\xf': { // Math.cos(Number) -> Number
                    res.type = &number, res.data = numdup(cos(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x10': { // Math.cosh(Number) -> Number
                    res.type = &number, res.data = numdup(cosh(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x11': { // Math.acos(Number) -> Number
                    res.type = &number, res.data = numdup(acos(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x12': { // Math.acosh(Number) -> Number
                    res.type = &number, res.data = numdup(acosh(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }

                case '\x13': { // Math.sin(Number) -> Number
                    res.type = &number, res.data = numdup(sin(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x14': { // Math.sinh(Number) -> Number
                    res.type = &number, res.data = numdup(sinh(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x15': { // Math.asin(Number) -> Number
                    res.type = &number, res.data = numdup(asin(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x16': { // Math.asinh(Number) -> Number
                    res.type = &number, res.data = numdup(asinh(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }

                case '\x17': { // Math.tan(Number) -> Number
                    res.type = &number, res.data = numdup(tan(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x18': { // Math.tanh(Number) -> Number
                    res.type = &number, res.data = numdup(tanh(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x19': { // Math.atan(Number) -> Number
                    res.type = &number, res.data = numdup(atan(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x1a': { // Math.atanh(Number) -> Number
                    res.type = &number, res.data = numdup(atanh(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x1b': { // Math.atan2(Number, Number) -> Number
                    res.type = &number, res.data = numdup(atan2(items > 0 ? numdata(param[0]) : 0, items > 1 ? numdata(param[1]) : 0));
                    return res;
                }

                case '\x1c': { // Math.exp(Number) -> Number
                    res.type = &number, res.data = numdup(exp(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x1d': { // Math.exp2(Number) -> Number
                    res.type = &number, res.data = numdup(exp2(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x1e': { // Math.expm1(Number) -> Number
                    res.type = &number, res.data = numdup(expm1(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }

                case '\x21': { // Math.log(Number) -> Number
                    res.type = &number, res.data = numdup(log(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x22': { // Math.log10(Number) -> Number
                    res.type = &number, res.data = numdup(log10(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x23': { // Math.log1p(Number) -> Number
                    res.type = &number, res.data = numdup(log1p(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x24': { // Math.log2(Number) -> Number
                    res.type = &number, res.data = numdup(log2(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }

                case '\x25': { // Math.erf(Number) -> Number
                    res.type = &number, res.data = numdup(erf(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x26': { // Math.erfc(Number) -> Number
                    res.type = &number, res.data = numdup(erfc(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x27': { // Math.gamma(Number) -> Number
                    res.type = &number, res.data = numdup(tgamma(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
                case '\x28': { // Math.lgamma(Number) -> Number
                    res.type = &number, res.data = numdup(lgamma(items > 0 ? numdata(param[0]) : 0));
                    return res;
                }
            }
            case 3: switch (id[2]) { // Socket
                case '\1': { // Socket.connect(hostname) -> SocketDescriptor | null
                    struct addrinfo hints, *result;
                    memset(&hints, 0, sizeof(hints)), hints.ai_family = AF_INET, hints.ai_socktype = SOCK_STREAM;
                    char *host = items > 0 ? strdata(param[0]) : "", *serv = "", *p = host;
                    while (*p != 0) {
                        p ++;
                        if (*p == ':') {
                            *p = 0;
                            if (*(p+1) == '/' && *(p+2) == '/') serv = host, host = p + 3;
                            else serv = p + 1;
                            break;
                        }
                    }
                    if (getaddrinfo(host, serv, &hints, &result) == 0) {
                        int sockfd = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
                        if (connect(sockfd, result->ai_addr, result->ai_addrlen) == 0) {
                            freeaddrinfo(result);
                            res.type = &socketd, res.data = malloc(sizeof(int)), *(int *)res.data = sockfd;
                            return res;
                        }
                    }
                    res.type = &null, res.data = NULL;
                    return res;
                }
                case '\2': { // Socket.open(port) -> SocketDescriptor | null
                    int port = items > 0 ? intdata(param[0]) : 0;
                    if (0 <= port && port <= 65535) {
                        struct sockaddr_in addr = {
                            .sin_family = AF_INET,
                            .sin_addr.s_addr = INADDR_ANY,
                            .sin_port = htons(port)
                        };
                        int sockfd = socket(AF_INET, SOCK_STREAM, 0), reuse = 1;
                        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == 0) {
                            if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == 0) {
                                if (listen(sockfd, 5) == 0) {
                                    res.type = &socketd, res.data = malloc(sizeof(int)), *(int *)res.data = sockfd;
                                    return res;
                                }
                            }
                        }
                    }
                    res.type = &null;
                    return res;
                }
            }
        }

        case type_file_stream: switch (id[1]) {
            case 1: { // FileStream.write(String)
                res.type = &undefined;
                if (self->type->name == type_file_stream) {
                    char *str = items > 0 ? strdata(param[0]) : "";
                    fwrite(str, sizeof(char), strlen(str), self->data);
                }
                return res;
            }
            case 2: { // FileStream.read(Number) -> String
                res.type = &string, res.data = "";
                if (self->type->name == type_file_stream) {
                    int length = items > 0 ? intdata(param[0]) : 0;
                    res.data = malloc(sizeof(char) * (length + 1)), fread(res.data, sizeof(char), length, self->data);
                }
                return res;
            }
            case 3: { // FileStream.seek(Number, Number) -> Boolean
                res.type = &boolean, res.data = booldup(self->type->name == type_file_stream ? fseek(self->data, items > 0 ? intdata(param[0]) : 0, items > 1 ? intdata(param[1]) : 0) : false);
                return res;
            }
        }

        case type_socket_descriptor: switch (id[1]) {
            case '\1': { // SocketDescriptor.send(String)
                res.type = &undefined;
                if (self->type->name == type_socket_descriptor) {
                    char *str = items > 0 ? strdata(param[0]) : "";
                    send(*(int *)self->data, str, strlen(str), 0);
                }
                return res;
            }
        }
    }
    return res;
}

Value builtin_depl(char *id, Value *self) {
    Value res;
    switch (id[0]) {

        case type_string: switch (id[1]) {
            case '\x1': { // String.length -> Number
                res.type = &number, res.data = numdup(self->type->name == type_string ? strlen(self->data) : 0);
                return res;
            }
            case '\x2': { // String.lower -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    char *str = res.data = strdup(self->data);
                    while (*str != 0) {
                        if (*str >= 'A' && *str <= 'Z') *str += 32;
                        str ++;
                    }
                } else res.data = "";
                return res;
            }
            case '\x3': { // String.upper -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    char *str = res.data = strdup(self->data);
                    while (*str != 0) {
                        if (*str >= 'a' && *str <= 'z') *str -= 32;
                        str ++;
                    }
                } else res.data = "";
                return res;
            }
            case '\x4': { // String.trim -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    char *str = self->data;
                    while (*str == ' ' || *str == '\n' || *str == '\r') str ++;
                    size_t length = strlen(str);
                    char *ptr = str + length - 1;
                    while (*ptr == ' ' || *ptr == '\n' || *ptr == '\r') ptr --, length --;
                    res.data = malloc(sizeof(char) * (length + 1)), memcpy(res.data, str, length);
                } else res.data = "";
                return res;
            }
            case '\x5': { // String.trimStart -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    res.data = self->data;
                    while (*(char *)res.data == ' ' || *(char *)res.data == '\n' || *(char *)res.data == '\r') res.data ++;
                } else res.data = "";
                return res;
            }
            case '\x6': { // String.trimEnd -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    size_t length = strlen(self->data);
                    char *ptr = self->data + length - 1;
                    while (*ptr == ' ' || *ptr == '\n' || *ptr == '\r') ptr --, length --;
                    res.data = malloc(sizeof(char) * (length + 1)), memcpy(res.data, self->data, length);
                } else res.data = "";
                return res;
            }
            case '\x7': { // String.reverse -> String
                res.type = &string;
                if (self->type->name == type_string) {
                    size_t length = strlen(self->data);
                    char *str = res.data = malloc(sizeof(char) * length), *ptr = self->data + length - 1;
                    while (*ptr != 0) *str ++ = *ptr --;
                } else res.data = "";
                return res;
            }
            case '\x8': { // String.chars -> Array
                res.type = &array;
                if (self->type->name == type_string) {
                    Value *arr = res.data = malloc(sizeof(Value) * (strlen(self->data) + 1));
                    char *str = self->data;
                    while (*str != 0) arr->type = &string, arr->data = malloc(sizeof(char) * 2), ((char *)arr->data)[0] = *str ++, ((char *)arr->data)[1] = 0, arr ++;
                    arr->type = &undefined;
                } else res.data = malloc(sizeof(Value)), (*(Value *)res.data).type = &undefined;
                return res;
            }
        }

        case type_number: switch (id[1]) {
            case '\x1': { // Number.isFinite -> Boolean
                res.type = &boolean, res.data = booldup(self->type->name == type_number ? __inline_isfinited(*(double *)self->data) : 0);
                return res;
            }
            case '\x2': { // Number.isNaN -> Boolean
                res.type = &boolean, res.data = booldup(self->type->name == type_number ? __inline_isnand(*(double *)self->data) : 0);
                return res;
            }
        }

        case type_object: switch (id[1]) {
        }

        case type_file_stream: switch (id[1]) {
            case 1: { // FileStream.close
                res.type = &undefined, res.data = NULL;
                if (self->type->name == type_file_stream) fclose(self->data), self->type = &null;
                return res;
            }
            case 2: { // FileStream.tell -> Number
                res.type = &number, res.data = numdup(self->type->name == type_file_stream ? ftell(self->data) : 0);
                return res;
            }
        }

        case type_socket_descriptor: switch (id[1]) {
            case '\1': { // SocketDescriptor.close
                res.type = &undefined;
                if (self->type->name == type_socket_descriptor) close(*(int *)self->data), self->type = &null;
                return res;
            }
            case '\2': { // SocketDescriptor.accept -> SocketDescriptor | null
                if (self->type->name == type_socket_descriptor) res.type = &socketd, res.data = malloc(sizeof(int)), *(int *)res.data = accept(*(int *)self->data, NULL, NULL);
                else res.type = &undefined;
                return res;
            }
            case '\3': { // SocketDescriptor.response -> String
                res.type = &string, res.data = "";
                if (self->type->name == type_socket_descriptor) {
                    char *str = res.data = malloc(sizeof(char) * BUFSIZ * 2);
                    while (recv(*(int *)self->data, str, BUFSIZ, 0) == BUFSIZ) str += BUFSIZ;
                }
                return res;
            }
        }
    }
    return res;
}

Value assign_value(Value *var, char **ptr, Value *self, Type *storage) {
    switch (**ptr) {
        case ASSIGNMENT: {
            (*ptr)++;
            *var = expression(ptr, self, storage);
            if (var->type->name == type_undefined) var->type = &null;
            return *var;
        }
        case ADDITION_ASSIGN: {
            (*ptr)++;
            if (var->type->name == type_number) *(double *)var->data += numdata(expression(ptr, self, storage));
            if (var->type->name == type_string) {
                char *str = strdata(expression(ptr, self, storage));
                var->data = realloc(var->data, sizeof(char) * (strlen(var->data) + strlen(str) + 1)), strcat((char *)var->data, str);
            }
            else var->data = numdup(numdata(*var) + numdata(expression(ptr, self, storage))), var->type = &number;
            return *var;
        }
        case SUBTRACTION_ASSIGN: {
            (*ptr)++;
            if (var->type->name == type_number) *(double *)var->data -= numdata(expression(ptr, self, storage));
            else var->data = numdup(numdata(*var) - numdata(expression(ptr, self, storage))), var->type = &number;
            return *var;
        }
        case EXPONENTIATION_ASSIGN: {
            (*ptr)++;
            if (var->type->name == type_number) *(double *)var->data = pow(*(double *)var->data, numdata(expression(ptr, self, storage)));
            else var->data = numdup(pow(numdata(*var), numdata(expression(ptr, self, storage)))), var->type = &number;
            return *var;
        }
        case MULTIPLICATION_ASSIGN: {
            (*ptr)++;
            if (var->type->name == type_number) *(double *)var->data *= numdata(expression(ptr, self, storage));
            else var->data = numdup(numdata(*var) * numdata(expression(ptr, self, storage))), var->type = &number;
            return *var;
        }
        case INCREMENT: {
            (*ptr)++;
            if (var->type->name != type_number) var->data = numdup(numdata(*var)), var->type = &number;
            (*(double *)var->data) ++;
            return *var;
        }
        case DECREMENT: {
            (*ptr)++;
            if (var->type->name != type_number) var->data = numdup(numdata(*var)), var->type = &number;
            (*(double *)var->data) --;
            return *var;
        }
    }
    return *var;
}

Value single_value(char **ptr, Value *self, Type *storage) {
    Value res, *var;

    switch (**ptr) {
        case IDENTIFIER: {
            int i = storage->items - 1;
            char *id = *ptr + 1;
            while (i >= 0 && strcmp(id, storage->object[i].id) != EXIT_SUCCESS) i --;
            while (**ptr != 0) (*ptr) ++;
            (*ptr) ++;

            if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) {
                if (i != -1) return assign_value(&storage->object[i].value, ptr, self, storage);
                add_ident((*storage), id, &null, NULL);
                return assign_value(&storage->object[storage->items - 1].value, ptr, self, storage);
            }
            if (i == -1) res.data = NULL, res.type = &undefined, var = &res;
            else if (storage->object[i].value.type->name == type_function) {
                if (*(char *)storage->object[i].value.data == ARROW) {
                    char *p = storage->object[i].value.data;
                    res = code(&p, self, storage).value, var = &res;
                }
                else if (*(char *)storage->object[i].value.data == '\x20') res = builtin_depl((char *)storage->object[i].value.data + 1, self), var = &res;
                else var = &storage->object[i].value;
            }
            else var = &storage->object[i].value;
            break;
        }
        case SELF: {
            (*ptr)++, var = self;
            if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) return assign_value(var, ptr, self, storage);
            break;
        }
        case GROUP: {
            do (*ptr)++, res = expression(ptr, self, storage); while (**ptr == COMMA);
            (*ptr)++, var = &res;
            break;
        }
        case ARRAY: {
            size_t items = 0;
            char *bptr = *ptr;
            do bptr++, skip_expression(&bptr), items ++; while (*bptr == COMMA);
            Value *arrptr = (res.data = malloc(sizeof(Value) * (items + 1))), val;
            do {
                (*ptr)++, val = expression(ptr, self, storage);
                if (val.type->name != type_undefined) *arrptr = valdup(val), arrptr ++;
            } while (**ptr == COMMA);
            arrptr->type = &undefined, res.type = &array, (*ptr)++, var = &res;
            break;
        }
        case DICT: {
            size_t items = 0;
            char *bptr = *ptr;
            do bptr++, skip_expression(&bptr), bptr++, skip_expression(&bptr), items ++; while (*bptr == COMMA);
            Ident *dicptr = res.data = malloc(sizeof(Ident) * (items + 1));
            Value id, val;
            do {
                (*ptr)++, id = expression(ptr, self, storage), (*ptr)++, val = expression(ptr, self, storage);
                if (val.type->name != type_undefined) dicptr->id = strdata(id), dicptr->value = valdup(val), dicptr ++;
            } while (**ptr == COMMA);
            res.type = &dictionary, (*ptr)++, var = &res;
            break;
        }
        case STRING: {
            res.data = strdup(*ptr + 1), res.type = &string, var = &res;
            while (*(*ptr)++ != EOS);
            break;
        }
        case NUMBER:
        case INT: {
            res.data = malloc(sizeof(double)), *(double *)res.data = atof(*ptr + 1), res.type = &number, var = &res;
            while (*(*ptr)++ != 0);
            break;
        }
        case ADDITION: {
            (*ptr) ++, res.data = numdup(+numdata(single_value(ptr, self, storage))), res.type = &number, var = &res;
            break;
        }
        case SUBTRACTION: {
            (*ptr) ++, res.data = numdup(-numdata(single_value(ptr, self, storage))), res.type = &number, var = &res;
            break;
        }
        case INCREMENT: {
        }
        case DECREMENT: {
        }
        case ARROW: {
            res.data = *ptr, res.type = &function, (*ptr) ++, skip_code(ptr), var = &res;
            break;
        }
        case FUNCTION: {
            res.data = *ptr, res.type = &function, var = &res;
            while (**ptr != FUNCTION_END) (*ptr) ++;
            (*ptr) ++, skip_code(ptr);
            break;
        }
        case TRUE: {
            res.data = booldup(true), res.type = &boolean, (*ptr) ++, var = &res;
            break;
        }
        case FALSE: {
            res.data = booldup(false), res.type = &boolean, (*ptr) ++, var = &res;
            break;
        }
        case NOT: {
            (*ptr) ++, res.data = booldup(!booldata(single_value(ptr, self, storage))), res.type = &boolean, var = &res;
            break;
        }
        case BITWISE_NOT: {
            (*ptr) ++, res.data = numdup(~intdata(single_value(ptr, self, storage))), res.type = &number, var = &res;
            break;
        }
        case L_NULL: {
            res.data = NULL, res.type = &null, (*ptr)++, var = &res;
            break;
        }
        case UNDEFINED: (*ptr)++;
        default: {
            res.data = NULL, res.type = &undefined, var = &res;
            break;
        }
    }

    while (**ptr == PERIOD || **ptr == GROUP || **ptr == ARRAYPARAM) {
        if (**ptr == GROUP) {
            size_t items = 0;
            Value *param = malloc(sizeof(Value) * BUFSIZ);
            do (*ptr)++, param[items] = expression(ptr, self, storage), items ++; while (**ptr == COMMA);

            if (var->type->name == type_function) {
                char *fptr = var->data;
                if (*fptr == '\x40') res = builtin_func(fptr + 1, items, param, self);
                else {
                    int length = storage->items;
                    fptr ++;
                    while (*fptr != FUNCTION_END) {
                        storage->object[storage->items].id = fptr + 1, storage->object[storage->items].value = valdup(*param), param ++, items --, storage->items ++;
                        while (*fptr != 0) fptr ++;
                        fptr ++;
                        if (*fptr == COMMA) fptr ++;
                        if (items == 0) while (*fptr != FUNCTION_END) fptr ++;
                    }
                    fptr ++, res = code(&fptr, self, storage).value, storage->items = length;
                }
            }
            else if (var->type->name == type_object) {

            }
            else if (var->type->name == type_string) res.type = &string, res.data = items > 0 ? strdata(param[0]) : "";
            else if (var->type->name == type_number) res.type = &number, res.data = numdup(items > 0 ? numdata(param[0]) : 0);
            else if (var->type->name == type_boolean) res.type = &boolean, res.data = booldup(items > 0 ? booldata(param[0]) : false);
            else res.type = &undefined;

            (*ptr) ++, var = &res;
        }
        else if (**ptr == PERIOD) {
            (*ptr) ++, self = var;

            int i = self->type->items - 1;
            char *id = *ptr + 1;
            while (i >= 0 && strcmp(id, self->type->object[i].id) != EXIT_SUCCESS) i --;
            while (**ptr != 0) (*ptr) ++;
            (*ptr) ++;
            if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) {
                if (i != -1) return assign_value(&self->type->object[i].value, ptr, self, self->type);
                add_ident((*self->type), id, &null, NULL);
                return assign_value(&self->type->object[self->type->items - 1].value, ptr, self, self->type);
            }
            if (i == -1) res.data = NULL, res.type = &undefined, var = &res;
            else if (self->type->object[i].value.type->name == type_function) {
                if (*(char *)self->type->object[i].value.data == ARROW) {
                    char *p = self->type->object[i].value.data;
                    res = code(&p, self, self->type).value, var = &res;
                }
                else if (*(char *)self->type->object[i].value.data == '\x20') res = builtin_depl((char *)self->type->object[i].value.data + 1, self), var = &res;
                else var = &self->type->object[i].value;
            }
            else var = &self->type->object[i].value;
        }
        else {
            Value index;
            do (*ptr)++, index = expression(ptr, self, storage); while (**ptr == COMMA);
            (*ptr) ++;

            if (var->type->name == type_array) {
                size_t i = intdata(index), items = arrlen(var->data);
                if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) {
                    if (items <= i) {
                        var->data = realloc(var->data, sizeof(Value) * (i + 2));
                        while (items <= i) ((Value *)var->data)[items].type = &null, items ++;
                        ((Value *)var->data)[items].type = &undefined;
                    }
                    return assign_value((Value *)var->data + i, ptr, self, storage);
                }
                var = items > i ? (Value *)var->data + i : (res.type = &undefined, &res);
            } else if (var->type->name == type_dictionary) {
                char *id = strdata(index);
                for (int i = 0;; i ++) {
                    if (((Ident *)var->data)[i].id == NULL) {
                        if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) {
                            var->data = realloc(var->data, sizeof(Ident) * (i + 2)), ((Ident *)var->data)[i].id = id, ((Ident *)var->data)[i].value.type = &undefined, ((Ident *)var->data)[i].value.data = NULL;
                            return assign_value(&((Ident *)var->data)[i].value, ptr, self, storage);
                        }
                        res.type = &undefined, var = &res;
                        break;
                    }
                    if (strcmp(id, ((Ident *)var->data)[i].id) == 0) {
                        if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) return assign_value(&((Ident *)var->data)[i].value, ptr, self, storage);
                        var = &((Ident *)var->data)[i].value;
                        break;
                    }
                }
            } else {
                res.type = &undefined, var = &res;
                if (**ptr <= ASSIGNMENT || **ptr == INCREMENT || **ptr == DECREMENT) return assign_value(var, ptr, self, storage);
            }
        }
    }

    return *var;
}

Value exponential_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = single_value(ptr, self, storage);
    if (**ptr == EXPONENTIATION) (*ptr) ++, res.data = numdup(pow(numdata(res), numdata(exponential_operation_value(ptr, self, storage)))), res.type = &number;
    return res;
}

Value arithmetic_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = exponential_operation_value(ptr, self, storage);
    while (**ptr == MULTIPLICATION || **ptr == DIVISION || **ptr == REMAINDER) {
        char c = **ptr;
        (*ptr) ++;
        double left = numdata(res), right = numdata(exponential_operation_value(ptr, self, storage));
        set_value(res, number, numdup(c == MULTIPLICATION ? left * right : c == DIVISION ? left / right : left - (long)(left / right) * right));
    }
    return res;
}

Value concatenation_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = arithmetic_operation_value(ptr, self, storage);
    while (**ptr == ADDITION || **ptr == SUBTRACTION) {
        bool c = **ptr == ADDITION;
        (*ptr) ++;
        if (res.type->name == type_string && c) {
            char *str1 = strdata(arithmetic_operation_value(ptr, self, storage)), *str = malloc(sizeof(char) * (strlen(res.data) + strlen(str1) + 1));
            strcpy(str, res.data), strcat(str, str1), res.data = str;
        }
        else if (res.type->name == type_array && c) {
            Value *arr1 = res.data, *arr2 = arithmetic_operation_value(ptr, self, storage).data, *arr = malloc(sizeof(Value) * (arrlen(arr1) + arrlen(arr2) + 1)), *p = arr;
            while (arr1->type->name != type_undefined) *arr = valdup(*arr1), arr ++, arr1 ++;
            while (arr2->type->name != type_undefined) *arr = valdup(*arr2), arr ++, arr2 ++; 
            arr->type = &undefined;
            res.data = p;
        }
        else {
            double left = numdata(res), right = numdata(arithmetic_operation_value(ptr, self, storage));
            res.data = numdup(c ? left + right: left - right), res.type = &number;
        }
    }
    return res;
}

Value bit_shift_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = concatenation_operation_value(ptr, self, storage);
    while (**ptr == LEFT_SHIFT || **ptr == RIGHT_SHIFT) {
        char c = **ptr;
        (*ptr) ++;
        int left = intdata(res), right = intdata(concatenation_operation_value(ptr, self, storage));
        set_value(res, number, numdup(c == LEFT_SHIFT ? left << right : left >> right));
    }
    return res;
}

Value relational_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = bit_shift_operation_value(ptr, self, storage);
    while (**ptr == LESS_THAN || **ptr == LESS_EQUAL || **ptr == GREATER_THAN || **ptr == GREATER_EQUAL) {
        char c = **ptr;
        (*ptr) ++;
        double left = numdata(res), right = numdata(bit_shift_operation_value(ptr, self, storage));
        set_value(res, number, numdup(c == LESS_THAN ? left < right : c == GREATER_THAN ? left > right : c == LESS_EQUAL ? left <= right : left >= right));
    }
    return res;
}

Value equivalence_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = relational_operation_value(ptr, self, storage);
    while (**ptr == EQUALITY || **ptr == INEQUALITY) {
        bool c = **ptr == EQUALITY;
        (*ptr) ++;
        res.data = booldup(valcmp(res, relational_operation_value(ptr, self, storage)) == c), res.type = &boolean;
    }
    return res;
}

Value bit_AND_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = equivalence_operation_value(ptr, self, storage);
    while (**ptr == BITWISE_AND) (*ptr) ++, set_value(res, number, numdup(intdata(res) & intdata(equivalence_operation_value(ptr, self, storage))));
    return res;
}

Value bit_OR_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = bit_AND_operation_value(ptr, self, storage);
    while (**ptr == BITWISE_OR) (*ptr) ++, set_value(res, number, numdup(intdata(res) | intdata(bit_AND_operation_value(ptr, self, storage))));
    return res;
}

Value bit_XOR_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = bit_OR_operation_value(ptr, self, storage);
    while (**ptr == BITWISE_XOR) (*ptr) ++, set_value(res, number, numdup(intdata(res) ^ intdata(bit_OR_operation_value(ptr, self, storage))));
    return res;
}

Value AND_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = bit_XOR_operation_value(ptr, self, storage);
    while (**ptr == AND) (*ptr) ++, set_value(res, boolean, booldup(booldata(res) && booldata(bit_XOR_operation_value(ptr, self, storage))));
    return res;
}

Value OR_operation_value(char **ptr, Value *self, Type *storage) {
    Value res = AND_operation_value(ptr, self, storage);
    while (**ptr == OR) (*ptr) ++, set_value(res, boolean, booldup(booldata(res) || booldata(AND_operation_value(ptr, self, storage))));
    return res;
}

Value expression(char **ptr, Value *self, Type *storage) {
    Value res = OR_operation_value(ptr, self, storage);
    if (**ptr == CONDITIONAL) {
        if (booldata(res)) (*ptr) ++, res = expression(ptr, self, storage), skip_expression(ptr);
        else (*ptr) ++, skip_expression(ptr), res = expression(ptr, self, storage);
    }
    return res;
}

void exec(char **ptr) {
    null.name = type_null, null.items = 0, null.object = NULL;
    undefined.name = type_undefined, undefined.items = 0, undefined.object = NULL;

    string.name = type_string, string.items = 0, string.object = malloc(sizeof(Ident) * BUFSIZ);
        add_ident(string, "length", &function, "\x20\x1\x1");
        add_ident(string, "lower", &function, "\x20\x1\x2");
        add_ident(string, "upper", &function, "\x20\x1\x3");
        add_ident(string, "trim", &function, "\x20\x1\x4");
        add_ident(string, "trimStart", &function, "\x20\x1\x5");
        add_ident(string, "trimEnd", &function, "\x20\x1\x6");
        add_ident(string, "reverse", &function, "\x20\x1\x7");
        add_ident(string, "chars", &function, "\x20\x1\x8");
        add_ident(string, "charCodes", &function, "\x20\x1\x9");
        add_ident(string, "substr", &function, "\x40\x1\x1");
        add_ident(string, "startsWith", &function, "\x40\x1\x2");
        add_ident(string, "endsWith", &function, "\x40\x1\x3");
        add_ident(string, "contains", &function, "\x40\x1\x4");
        add_ident(string, "indexOf", &function, "\x40\x1\x5");
        add_ident(string, "padStart", &function, "\x40\x1\x6");
        add_ident(string, "padEnd", &function, "\x40\x1\x7");
        add_ident(string, "repeat", &function, "\x40\x1\x8");
        add_ident(string, "replace", &function, "\x40\x1\x9");
        add_ident(string, "replaceAll", &function, "\x40\x1\xa");
        add_ident(string, "split", &function, "\x40\x1\xb");
        add_ident(string, "charAt", &function, "\x40\x1\xc");
        add_ident(string, "charCodeAt", &function, "\x40\x1\xd");
        add_ident(string, "fromCharCode", &function, "\x40\x1\xe");
        add_ident(string, "toInt", &function, "\x40\x1\xf");

    number.name = type_number, number.items = 0, number.object = malloc(sizeof(Ident) * BUFSIZ);
        add_ident(number, "isFinite", &function, "\x20\x2\x1");
        add_ident(number, "isNaN", &function, "\x20\x2\x2");
        add_ident(number, "toString", &function, "\x40\x2\x1");

    boolean.name = type_boolean, boolean.items = 0, boolean.object = NULL;

    array.name = type_array, array.items = 0, array.object = NULL;
    dictionary.name = type_dictionary, dictionary.items = 0, dictionary.object = NULL;

    function.name = type_function, function.items = 0, function.object = NULL;

    files.name = type_file_stream, files.items = 0, files.object = malloc(sizeof(Ident) * BUFSIZ);
        add_ident(files, "write", &function, "\x40\x8\x1");
        add_ident(files, "read", &function, "\x40\x8\x2");
        add_ident(files, "seek", &function, "\x40\x8\x3");
        add_ident(files, "close", &function, "\x20\x8\x1");
        add_ident(files, "tell", &function, "\x20\x8\x2");
    
    socketd.name = type_socket_descriptor, socketd.items = 0, socketd.object = malloc(sizeof(Ident) * BUFSIZ);
        add_ident(socketd, "send", &function, "\x40\x9\x1");
        add_ident(socketd, "close", &function, "\x20\x9\x1");
        add_ident(socketd, "accept", &function, "\x20\x9\x2");
        add_ident(socketd, "response", &function, "\x20\x9\x3");

    Type object_file = { .name = type_object, .object = malloc(sizeof(Ident) * BUFSIZ)};
        add_ident(object_file, "open", &function, "\x40\x7\x1\x1");
        add_ident(object_file, "read", &function, "\x40\x7\x1\x2");
        add_ident(object_file, "append", &function, "\x40\x7\x1\x3");
        add_ident(object_file, "write", &function, "\x40\x7\x1\x4");
        add_ident(object_file, "remove", &function, "\x40\x7\x1\x5");
        add_ident(object_file, "exist", &function, "\x40\x7\x1\x6");
        add_ident(object_file, "rename", &function, "\x40\x7\x1\x7");
        add_ident(object_file, "fullpath", &function, "\x40\x7\x1\x8");
        add_ident(object_file, "size", &function, "\x40\x7\x1\x9");
        add_ident(object_file, "stat", &function, "\x40\x7\x1\xa");
        add_ident(object_file, "chmod", &function, "\x40\x7\x1\xb");

    Type object_math = { .name = type_object, .object = malloc(sizeof(Ident) * BUFSIZ)};
        add_ident(object_math, "PI", &number, numdup(M_PI));
        add_ident(object_math, "E", &number, numdup(M_E));
        add_ident(object_math, "LN2", &number, numdup(M_LN2));
        add_ident(object_math, "LN10", &number, numdup(M_LN10));
        add_ident(object_math, "LOG2E", &number, numdup(M_LOG2E));
        add_ident(object_math, "LOG10E", &number, numdup(M_LOG10E));
        add_ident(object_math, "SQRT1_2", &number, numdup(M_SQRT1_2));
        add_ident(object_math, "SQRT2", &number, numdup(M_SQRT2));
        add_ident(object_math, "abs", &function, "\x40\x7\x2\x1");
        add_ident(object_math, "pow", &function, "\x40\x7\x2\x2");
        add_ident(object_math, "sqrt", &function, "\x40\x7\x2\x3");
        add_ident(object_math, "cbrt", &function, "\x40\x7\x2\x4");
        add_ident(object_math, "hypot", &function, "\x40\x7\x2\x5");
        add_ident(object_math, "ceil", &function, "\x40\x7\x2\x6");
        add_ident(object_math, "floor", &function, "\x40\x7\x2\x7");
        add_ident(object_math, "round", &function, "\x40\x7\x2\x8");
        add_ident(object_math, "cos", &function, "\x40\x7\x2\xf");
        add_ident(object_math, "cosh", &function, "\x40\x7\x2\x10");
        add_ident(object_math, "acos", &function, "\x40\x7\x2\x11");
        add_ident(object_math, "acosh", &function, "\x40\x7\x2\x12");
        add_ident(object_math, "sin", &function, "\x40\x7\x2\x13");
        add_ident(object_math, "sinh", &function, "\x40\x7\x2\x14");
        add_ident(object_math, "asin", &function, "\x40\x7\x2\x15");
        add_ident(object_math, "asinh", &function, "\x40\x7\x2\x16");
        add_ident(object_math, "tan", &function, "\x40\x7\x2\x17");
        add_ident(object_math, "tanh", &function, "\x40\x7\x2\x18");
        add_ident(object_math, "atan", &function, "\x40\x7\x2\x19");
        add_ident(object_math, "atanh", &function, "\x40\x7\x2\x1a");
        add_ident(object_math, "atan2", &function, "\x40\x7\x2\x1b");
        add_ident(object_math, "exp", &function, "\x40\x7\x2\x1c");
        add_ident(object_math, "exp2", &function, "\x40\x7\x2\x1d");
        add_ident(object_math, "expm1", &function, "\x40\x7\x2\x1e");
        add_ident(object_math, "log", &function, "\x40\x7\x2\x21");
        add_ident(object_math, "log10", &function, "\x40\x7\x2\x22");
        add_ident(object_math, "log1p", &function, "\x40\x7\x2\x23");
        add_ident(object_math, "log2", &function, "\x40\x7\x2\x24");
        add_ident(object_math, "erf", &function, "\x40\x7\x2\x25");
        add_ident(object_math, "erfc", &function, "\x40\x7\x2\x26");
        add_ident(object_math, "gamma", &function, "\x40\x7\x2\x27");
        add_ident(object_math, "lgamma", &function, "\x40\x7\x2\x28");

    Type object_socket = { .name = type_object, .items = 0, .object = malloc(sizeof(Ident) * BUFSIZ)};
        add_ident(object_socket, "connect", &function, "\x40\x7\x3\x1");
        add_ident(object_socket, "open", &function, "\x40\x7\x3\x2");

    Type main_object = { .name = type_object, .items = 0, .object = malloc(sizeof(Ident) * BUFSIZ)};
        add_ident(main_object, "print", &function, "\x40\x0\x1");
        add_ident(main_object, "typeof", &function, "\x40\x0\x2");
        add_ident(main_object, "exit", &function, "\x40\x0\x3");
        add_ident(main_object, "gets", &function, "\x40\x0\x4");
        add_ident(main_object, "puts", &function, "\x40\x0\x5");
        add_ident(main_object, "Main", &main_object, NULL);
        add_ident(main_object, "Socket", &object_socket, NULL);
        add_ident(main_object, "File", &object_file, NULL);
        add_ident(main_object, "Math", &object_math, NULL);
        add_ident(main_object, "String", &string, "");
        add_ident(main_object, "Number", &number, numdup(0));
        add_ident(main_object, "Boolean", &boolean, booldup(true));
        //add_ident(storage, "Array", array, malloc(sizeof(Value))); Value val = {.type = &undefined,.data = NULL}; *(Value *)storage.object[storage.items].value.data = val;

    Value this = {.type = &main_object};
    while (**ptr != 0) code(ptr, &this, &main_object);
}








/**
 * This process checks the syntax and creates data for execution.
 * This data can also be written to a binary file by specifying it with command line options.
 */

#define is_identifier(C) ((C >= 'A' && C <= 'Z') || (C >= 'a' && C <= 'z') || C == '_')
#define is_number(c) ((c >= '0' && c <= '9') || c == '.')
#define is_empty(c) (c == ' ' || c == '\n' || c == '\r')
#define is_assignment(c) (c <= ASSIGNMENT)
#define is_operator(c) (c <= COMMA)
#define is_single(c) (c >= DECREMENT)
#define slide ((*ptr) ++, (*length) ++)
#define md_code 0
#define md_expresson 1
#define md_dictionary 2
#define md_single 3

typedef struct posison {
    char path[256];
    FILE *fp;
    long pos;
    int column;
    int line;
} Pos;

void syntax_error(char *message, int length, Pos file) {
    char buffer[BUFSIZ];
    fseek(file.fp, file.pos, SEEK_SET), fgets(buffer, sizeof(buffer), file.fp);
    int last = strlen(buffer);
    if (buffer[last - 1] == '\n') buffer[last - 1] = 0;
    printf("\033[1m%s\033[0m line %d\n %s\n\033[92m", file.path, file.line, buffer);
    while (file.column - length > 0) fputc(' ', stdout), file.column --;
    if (length == 0) fputc('^', stdout);
    else while (length > 0) fputc('^', stdout), length --;
    printf("\033[0m\nSyntaxError: %s\n", message), exit(EXIT_FAILURE);
}

bool reading(char end, char **ptr, long *length, Pos *file, char mode);

void set_symbol(char **ptr, long *length, Pos *file);

void lang(char **ptr, long *length, Pos *file) {
    if (*(*ptr-10) != PERIOD && *(*ptr-9) == IDENTIFIER && *(*ptr-8) == 'u' && *(*ptr-7) == 'n' && *(*ptr-6) == 'd' && *(*ptr-5) == 'e' && *(*ptr-4) == 'f' && *(*ptr-3) == 'i' && *(*ptr-2) == 'n' && *(*ptr-1) == 'e' && **ptr == 'd') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = UNDEFINED;
    }
    else if (*(*ptr-10) != PERIOD && *(*ptr-4) == IDENTIFIER && *(*ptr-3) == 't' && *(*ptr-2) == 'r' && *(*ptr-1) == 'u' && **ptr == 'e') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = TRUE;
    }
    else if (*(*ptr-10) != PERIOD && *(*ptr-5) == IDENTIFIER && *(*ptr-4) == 'f' && *(*ptr-3) == 'a' && *(*ptr-2) == 'l' && *(*ptr-1) == 's' && **ptr == 'e') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = FALSE;
    }
    else if (*(*ptr-5) != PERIOD && *(*ptr-4) == IDENTIFIER && *(*ptr-3) == 'v' && *(*ptr-2) == 'o' && *(*ptr-1) == 'i' && **ptr == 'd') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = UNDEFINED;
    }
    else if (*(*ptr-5) != PERIOD && *(*ptr-4) == IDENTIFIER && *(*ptr-3) == 'n' && *(*ptr-2) == 'u' && *(*ptr-1) == 'l' && **ptr == 'l') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = L_NULL;
    }
    else if (*(*ptr-5) != PERIOD && *(*ptr-4) == IDENTIFIER && *(*ptr-3) == 's' && *(*ptr-2) == 'e' && *(*ptr-1) == 'l' && **ptr == 'f') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = SELF;
    }

    else if (*(*ptr-2) == IDENTIFIER && *(*ptr-1) == 'i' && **ptr == 'f') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = IF;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 2, *file);
    }
    else if (*(*ptr-4) == IDENTIFIER && *(*ptr-3) == 'e' && *(*ptr-2) == 'l' && *(*ptr-1) == 's' && **ptr == 'e') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = ELSE;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 4, *file);
    }
    else if (*(*ptr-5) == IDENTIFIER && *(*ptr-4) == 'w' && *(*ptr-3) == 'h' && *(*ptr-2) == 'i' && *(*ptr-1) == 'l' && **ptr == 'e') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = WHILE;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 5, *file);
    }
    else if (*(*ptr-2) == IDENTIFIER && *(*ptr-1) == 'd' && **ptr == 'o') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = DO;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 2, *file);
    }
    else if (*(*ptr-3) == IDENTIFIER && *(*ptr-2) == 'f' && *(*ptr-1) == 'o' && **ptr == 'r') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --;  **ptr = FOR;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 3, *file);
    }
    else if (*(*ptr-6) == IDENTIFIER && *(*ptr-5) == 's' && *(*ptr-4) == 'w' && *(*ptr-3) == 'i' && *(*ptr-2) == 't' && *(*ptr-1) == 'c' && **ptr == 'h') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = SWITCH;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 6, *file);
    }
    else if (*(*ptr-4) == IDENTIFIER && *(*ptr-3) == 'c' && *(*ptr-2) == 'a' && *(*ptr-1) == 's' && **ptr == 'e') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = CASE;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 4, *file);
    }
    else if (*(*ptr-7) == IDENTIFIER && *(*ptr-6) == 'd' && *(*ptr-5) == 'e' && *(*ptr-4) == 'f' && *(*ptr-3) == 'a' && *(*ptr-2) == 'u' && *(*ptr-1) == 'l' && **ptr == 't') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = DEFAULT;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 7, *file);
    }
    else if (*(*ptr-6) == IDENTIFIER && *(*ptr-5) == 'r' && *(*ptr-4) == 'e' && *(*ptr-3) == 't' && *(*ptr-2) == 'u' && *(*ptr-1) == 'r' && **ptr == 'n') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = REUTRN;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 6, *file);
    }
    else if (*(*ptr-5) == IDENTIFIER && *(*ptr-4) == 'b' && *(*ptr-3) == 'r' && *(*ptr-2) == 'e' && *(*ptr-1) == 'a' && **ptr == 'k') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = BREAK;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 5, *file);
    }
    else if (*(*ptr-8) == IDENTIFIER && *(*ptr-7) == 'c' && *(*ptr-6) == 'o' && *(*ptr-5) == 'n' && *(*ptr-4) == 't' && *(*ptr-3) == 'i' && *(*ptr-2) == 'n' && *(*ptr-1) == 'u' && **ptr == 'e') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = CONTINUE;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 8, *file);
    }
    else if (*(*ptr-5) == IDENTIFIER && *(*ptr-4) == 'c' && *(*ptr-3) == 'l' && *(*ptr-2) == 'a' && *(*ptr-1) == 's' && **ptr == 's') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = CLASS;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 5, *file);
        char c;
        do if ((file->column ++, c = fgetc(file -> fp)) == '\n') file -> pos = ftell(file -> fp), file -> line ++, file->column = 0; while (is_empty(c));
        if (is_identifier(c)) {
            slide, **ptr = IDENTIFIER;
            do slide, **ptr = c, file->column ++, c = fgetc(file -> fp); while (is_identifier(c) || (c >= '0' && c <= '9'));
            while (is_empty(c)) if ((file->column ++, c = fgetc(file -> fp)) == '\n') file -> pos = ftell(file -> fp), file -> line ++, file->column = 0;
            if (c == '{') {
                slide, **ptr = BLOCK;
                Pos back = *file;
                slide, reading('}', ptr, length, file, md_code) ? **ptr = EOB : syntax_error("Unclosed '{'", 0, back);
            }
            else syntax_error("Invalid syntax", 0, *file);
        }
        else syntax_error("Missing identifier", 0, *file);
    }
    else if (*(*ptr-4) == IDENTIFIER && *(*ptr-3) == 'f' && *(*ptr-2) == 'u' && *(*ptr-1) == 'n' && **ptr == 'c') {
        while (**ptr != IDENTIFIER) **ptr = 0, (*ptr) --, (*length) --; **ptr = FUNC;
        if (is_operator(*(*ptr-1))) syntax_error("Invalid syntax", 4, *file);
        char c;
        do if ((file->column ++, c = fgetc(file -> fp)) == '\n') file -> pos = ftell(file -> fp), file -> line ++, file->column = 0; while (is_empty(c));
        if (is_identifier(c)) {
            slide, **ptr = IDENTIFIER;
            do slide, **ptr = c, file->column ++, c = fgetc(file -> fp); while (is_identifier(c) || (c >= '0' && c <= '9'));
            while (is_empty(c)) if ((file->column ++, c = fgetc(file -> fp)) == '\n') file -> pos = ftell(file -> fp), file -> line ++, file->column = 0;
            if (c == '(') {
                set_symbol(ptr, length, file), **ptr = GROUP, slide;
                Pos back = *file;
                reading(')', ptr, length, file, md_expresson) ? **ptr = EOG : **ptr == EOF ? syntax_error("Unclosed '('", 0, back) : syntax_error("Invalid syntax", 0, *file);

            }
            else if (c == '-') {

            }
        }
        else syntax_error("Missing identifier", 0, *file);
    }
    else (*ptr) ++, (*length) ++;
}

void assign(char **ptr, long *length, Pos *file) {
    char *p = *ptr-1;
    if (*p == 0) while (*p != IDENTIFIER && *p != NUMBER && *p != INT) p --;
    if (*p != IDENTIFIER && *p != EOAP && *p != SELF) syntax_error("Invalid left-hand side in assignment", 1, *file);
}

void set_symbol(char **ptr, long *length, Pos *file) {
    if (**ptr != 0) {
        if (is_number(**ptr)) slide;
        else if (is_identifier(**ptr)) lang(ptr, length, file);
        else if (is_assignment(**ptr)) assign(ptr, length, file);
        slide;
    }
}

bool reading(char end, char **ptr, long *length, Pos *file, char mode) {
    char c;
    while ((c = fgetc(file -> fp)) != end) {
        file->column ++;
        if (c == ' ') set_symbol(ptr, length, file);
        else if (is_identifier(c)) {
            if (is_identifier(**ptr)) slide, **ptr = c;
            else if (is_number(**ptr)) {
                char *p = *ptr;
                while (*p != IDENTIFIER && *p != NUMBER && *p != INT) p --;
                if (*p != IDENTIFIER) syntax_error("Invalid numeric literal", 0, *file);
                slide, **ptr = c;
            }
            else {
                if (**ptr != 0) {
                    if (**ptr == '.') slide;
                    else if (is_assignment(**ptr)) assign(ptr, length, file);
                    slide;
                }
                if (is_single(*(*ptr-1)) && mode != md_code) syntax_error("Missing operator", 0, *file);
                **ptr = IDENTIFIER, slide, **ptr = c;
            }
        }
        else if (c >= '0' && c <= '9') {
            if (is_identifier(**ptr) || is_number(**ptr)) slide, **ptr = c;
            else {
                if (**ptr != 0) {
                    if (is_assignment(**ptr)) assign(ptr, length, file);
                    slide;
                }
                if (is_single(*(*ptr-1)) && mode != md_code) syntax_error("Missing operator", 0, *file);
                **ptr = INT, slide, **ptr = c;
            }
        }
        else if (c == '\n') set_symbol(ptr, length, file), file -> pos = ftell(file -> fp), file -> line ++, file->column = 0;
        else if (c == ',') {
            set_symbol(ptr, length, file), **ptr = COMMA;
            if (mode == md_single) return 0;
            else if (mode == md_dictionary) {
                char *p = *ptr-1;
                while (*p != COMMA && *p != ARRAYKEY) p --;
                if (*p != ARRAYKEY) syntax_error("need ':'", 0, *file);
            }
        }
        else if (c == ';') {
            set_symbol(ptr, length, file), **ptr = EOL;
            if (mode != md_code) return 0;
        }
        else if (c == '[') {
            set_symbol(ptr, length, file);
            Pos back = *file;
            if (is_single(*(*ptr-1))) **ptr = ARRAYPARAM, slide, reading(']', ptr, length, file, md_expresson) ? **ptr = EOAP : **ptr == EOF ? syntax_error("Unclosed '['", 0, back) : syntax_error("Invalid syntax", 0, *file);
            else **ptr = ARRAY, slide, reading(']', ptr, length, file, md_expresson) ? **ptr = EOA : **ptr == EOF ? syntax_error("Unclosed '['", 0, back) : syntax_error("Invalid syntax", 0, *file);
        }
        else if (c == '(') {
            set_symbol(ptr, length, file), **ptr = GROUP, slide;
            Pos back = *file;
            reading(')', ptr, length, file, md_expresson) ? **ptr = EOG : **ptr == EOF ? syntax_error("Unclosed '('", 0, back) : syntax_error("Invalid syntax", 0, *file);
        }
        else if (c == '{') {
            set_symbol(ptr, length, file), **ptr = BLOCK;
            if ((is_operator(*(*ptr-1)) && *(*ptr-1) != FUNCTION_END && *(*ptr-1) != ARROW) || (is_single(*(*ptr-1)) && mode != md_code)) syntax_error("Invalid syntax", 0, *file);
            Pos back = *file;
            slide, reading('}', ptr, length, file, md_code) ? **ptr = EOB : syntax_error("Unclosed '{'", 0, back);
        }
        else if (c == '=') {
            if (**ptr == 0) **ptr = ASSIGNMENT;
            else switch (**ptr) {
                case ASSIGNMENT: **ptr = EQUALITY; break;
                case NOT: **ptr = INEQUALITY; break;
                case LESS_THAN: **ptr = LESS_EQUAL; break;
                case GREATER_THAN: **ptr = GREATER_EQUAL; break;
                case ADDITION: **ptr = ADDITION_ASSIGN; break;
                case SUBTRACTION: **ptr = SUBTRACTION_ASSIGN; break;
                case EXPONENTIATION: **ptr = EXPONENTIATION_ASSIGN; break;
                case MULTIPLICATION: **ptr = MULTIPLICATION_ASSIGN; break;
                case DIVISION: **ptr = DIVISION_ASSIGN; break;
                case REMAINDER: **ptr = REMAINDER_ASSIGN; break;
                case LEFT_SHIFT: **ptr = LEFT_SHIFT_ASSIGN; break;
                case RIGHT_SHIFT: **ptr = RIGHT_SHIFT_ASSIGN; break;
                case AND: **ptr = AND_ASSIGN; break;
                case OR: **ptr = OR_ASSIGN; break;
                case BITWISE_AND: **ptr = BITWISE_AND_ASSIGN; break;
                case BITWISE_OR: **ptr = BITWISE_OR_ASSIGN; break;
                case BITWISE_XOR: **ptr = BITWISE_XOR_ASSIGN; break;
                default: {
                    if (is_number(**ptr)) slide;
                    else if (is_identifier(**ptr)) lang(ptr, length, file);
                    else if (is_assignment(**ptr)) assign(ptr, length, file);
                    slide, **ptr = ASSIGNMENT;
                    break;
                }
            }
        }
        else if (c == '.') {
            if (is_identifier(**ptr) || is_number(**ptr)) {
                char *p = *ptr;
                while (*p != INT && *p != NUMBER && *p != IDENTIFIER) p --;
                if (*p == INT) *p = NUMBER, slide, **ptr = c; 
                else if (*p == IDENTIFIER) lang(ptr, length, file), slide, **ptr = PERIOD;
                else (*ptr) += 2, (*length) += 2, **ptr = PERIOD;
            }
            else {
                if (**ptr != 0) slide;
                if (*(*ptr-1) <= EOL) syntax_error("Invalid left-hand side expression", 0, *file);
                **ptr = PERIOD;
            }
            if (**ptr == PERIOD) {
                do if ((file->column ++, c = fgetc(file -> fp)) == '\n') file -> pos = ftell(file -> fp), file -> line ++; while (is_empty(c));
                if (c == '.') /* スプレッド構文 */;
                else if (is_identifier(c)) slide, **ptr = IDENTIFIER, slide, **ptr = c;
                else syntax_error("Missing identifier", 0, *file);
            }
        }
        else if (c == '"' || c == '\'') {
            set_symbol(ptr, length, file), **ptr = STRING;
            if (is_single(*(*ptr-1)) && mode != md_code) syntax_error("Missing operator", 0, *file);
            Pos back = *file;
            char end = c;
            while ((file->column ++, c = fgetc(file -> fp)) != end) {
                if (c == '\n' || c == EOF) syntax_error("Unterminated string literal", 0, back);
                else if (c == '\\') {
                    file->column ++, c = fgetc(file -> fp);
                    if (c == '\n' || c == EOF) syntax_error("Unterminated string literal", 0, back);
                    else if (c == 'a') c = '\a';
                    else if (c == 'b') c = '\b';
                    else if (c == 't') c = '\t';
                    else if (c == 'n') c = '\n';
                    else if (c == 'v') c = '\v';
                    else if (c == 'f') c = '\f';
                    else if (c == 'r') c = '\r';
                    else if (c == 'e') c = '\e';
                    else if (c == 'x' || c == 'u' || (c >= '0' && c <= '7')) {
                        int d = c, len = (d == 'u') ? 5 : 3, num = len;
                        do {
                            num --, slide, **ptr = c;
                            if (num > 0) {
                                file->column ++, c = fgetc(file -> fp);
                                if (c == '\n' || c == EOF) syntax_error("Unterminated string literal", 0, back);
                            } else {
                                c = strtol(*ptr - (d == 'u' ? 3 : d == 'x' ? 1 : 2), NULL, d == 'u' || d == 'x' ? 16 : 8);
                                while (num < len) num ++, **ptr = 0, (*ptr) --, (*length) --;
                                break;
                            }
                        } while (d == 'u' || d == 'x' ? (c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F') : c >= '0' && c <= '7');
                    }
                }
                slide, **ptr = c;
            }
            slide, slide, **ptr = EOS;
        }
        else if (c == '+') {
            if (**ptr == ADDITION && *(*ptr-1) != INCREMENT) {
                **ptr = INCREMENT;
                char *p = *ptr-1;
                if (*p == 0) while (*p != IDENTIFIER && *p != NUMBER && *p != INT) p --;
                if (*p != IDENTIFIER && *p != EOAP && *p != SELF) {
                    do if ((file->column ++, c = fgetc(file -> fp)) == '\n') file -> pos = ftell(file -> fp), file -> line ++; while (is_empty(c));
                    if (is_identifier(c)) slide, **ptr = IDENTIFIER, slide, **ptr = c;
                    else syntax_error("Invalid left-hand side expression in prefix operation", 0, *file);
                }
            }
            else set_symbol(ptr, length, file), **ptr = ADDITION;
        }
        else if (c == '-') {
            if (**ptr == SUBTRACTION && *(*ptr-1) != DECREMENT) {
                **ptr = DECREMENT;
                char *p = *ptr-1;
                if (*p == 0) while (*p != IDENTIFIER && *p != NUMBER && *p != INT) p --;
                if (*p != IDENTIFIER && *p != EOAP && *p != SELF) {
                    do if ((file->column ++, c = fgetc(file -> fp)) == '\n') file -> pos = ftell(file -> fp), file -> line ++; while (is_empty(c));
                    if (is_identifier(c)) slide, **ptr = IDENTIFIER, slide, **ptr = c;
                    else syntax_error("Invalid left-hand side expression in prefix operation", 0, *file);
                }
            }
            else set_symbol(ptr, length, file), **ptr = SUBTRACTION;
        }
        else if (c == '/') {
            if (**ptr == DIVISION) {
                do file->column ++, c = fgetc(file->fp); while (c != '\n' && c != EOF);
                **ptr = 0, file->pos = ftell(file->fp), file->line ++;
            }
            else set_symbol(ptr, length, file), **ptr = DIVISION;
        }
        else if (c == '*') {
            if (**ptr == DIVISION) {
                Pos back = *file;
                while ((file->column ++, c = fgetc(file->fp)) != '*' ? 1 : (file->column ++, c = fgetc(file->fp)) != '/') {
                    if (c == '\n') file -> pos = ftell(file -> fp), file -> line ++;
                    if (c == EOF) syntax_error("Unterminated comment block", 0, back);
                }
                **ptr = 0;
            }
            else if (**ptr == MULTIPLICATION) **ptr = EXPONENTIATION;
            else set_symbol(ptr, length, file), **ptr = MULTIPLICATION;
        }
        else if (c == '%') set_symbol(ptr, length, file), **ptr = REMAINDER;
        else if (c == '<') {
            if (**ptr == LESS_THAN) **ptr = LEFT_SHIFT;
            else set_symbol(ptr, length, file), **ptr = LESS_THAN;
        }
        else if (c == '>') {
            if (**ptr == GREATER_THAN) **ptr = RIGHT_SHIFT;
            else if (**ptr == SUBTRACTION) {
                char *p = *ptr;
                if (*(p-1) == EOG) {
                    *p-- = 0, *p-- = FUNCTION_END;
                    while (*p != GROUP) {
                        if (*p != 0) syntax_error("perror", 1, *file);
                        while (*p != IDENTIFIER && *p != NUMBER && *p != INT) p --;
                        if (*p == IDENTIFIER) p --;
                        if (*p == COMMA) p --;
                    }
                    *p = FUNCTION;
                } else *p = ARROW;
                if (*(p-1) > PERIOD && mode != md_code) syntax_error("Missing operator", 0, *file);
            }
            else set_symbol(ptr, length, file), **ptr = GREATER_THAN;
        }
        else if (c == '!') {
            set_symbol(ptr, length, file), **ptr = NOT;
            if (is_single(*(*ptr-1)) && mode != md_code) syntax_error("Missing operator", 0, *file);
        }
        else if (c == '~') {
            set_symbol(ptr, length, file), **ptr = BITWISE_NOT;
            if (is_single(*(*ptr-1)) && mode != md_code) syntax_error("Missing operator", 0, *file);
        }
        else if (c == '&') {
            if (**ptr == BITWISE_AND) **ptr = AND;
            else set_symbol(ptr, length, file), **ptr = BITWISE_AND;
        }
        else if (c == '|') {
            if (**ptr == BITWISE_OR) **ptr = OR;
            else set_symbol(ptr, length, file), **ptr = BITWISE_OR;
        }
        else if (c == '^') set_symbol(ptr, length, file), **ptr = BITWISE_XOR;
        else if (c == '?') {
            set_symbol(ptr, length, file), **ptr = CONDITIONAL;
            reading(':', ptr, length, file, md_single) ? **ptr = CONDITIONAL_ELSE : syntax_error(**ptr == EOF ? "need ':'" : "Invalid syntax", 0, *file);
        }
        else if (c == ':') {
            set_symbol(ptr, length, file), **ptr = ARRAYKEY;
            if (mode != md_dictionary) {
                if (mode != md_expresson) file->column ++, syntax_error("Unmatched ':'", 0, *file);
                char *p = *ptr-1;
                while (*p != ARRAY && *p != COMMA && *p != GROUP) p --;
                if (*p != ARRAY) syntax_error("Unmatched ':'", 0, *file);
                *p = DICT, mode = md_dictionary;
            }
            else {
                char *p = *ptr-1;
                while (*p != COMMA && *p != ARRAYKEY) p --;
                if (*p == ARRAYKEY) syntax_error("Unmatched ':'", 0, *file);
            }
        }
        else if (c == '}') syntax_error("Unmatched '}'", 0, *file);
        else if (c == ')') syntax_error("Unmatched ')'", 0, *file);
        else if (c == ']') syntax_error("Unmatched ']'", 0, *file);
        else if (c == EOF) {
            set_symbol(ptr, length, file), **ptr = EOF;
            return 0;
        }
        else syntax_error("Invalid character", 0, *file);
    }

    if (**ptr != 0) {
        if (is_number(**ptr)) slide;
        else if (is_identifier(**ptr)) lang(ptr, length, file);
        else if (is_assignment(**ptr)) assign(ptr, length, file);
        slide;
    }
    if (mode == md_dictionary) {
        char *p = *ptr-1;
        while (*p != COMMA && *p != ARRAYKEY) p --;
        if (*p != ARRAYKEY) file->column ++, syntax_error("need ':'", 0, *file);
    }
    return 1;
}

int main(int argc, char **argv) {

    Pos file;
    if (realpath(argv[1], file.path) == NULL) {
        puts("error");
        return EXIT_FAILURE;
    }
    file.fp = fopen(argv[1], "r"), file.pos = 0L, file.line = 1, file.column = 0;

    char *ptr = malloc(sizeof(char) * BUFSIZ), *str = ptr + 1;
    long length = 0L;

    *ptr ++ = EOL, reading(EOF, &ptr, &length, &file, md_code);

    //fwrite(str, sizeof(char), length, fopen("test", "wb"));

    exec(&str);
    //printf("%d\n", str[0]);

    return EXIT_SUCCESS;
}