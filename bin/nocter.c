#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "nocter.h"

typedef struct object Object;
typedef struct value Value;
typedef struct key Key;

struct value {
    Object *obj;
    void *data;
};

struct key {
    char *id;
    Value val;
};

struct object {
    Key *list;
    Key set[256];
};

typedef struct log {
    Object *obj;
    Key *set;
} Log;

Object vars, string, number, integrate, boolean, function;

#define undefined (Object *)-1
#define null (Object *)0
#define any (Object *)1



void ex_start_scope(Object *obj, Log **log) {
    *((*log) ++) = (Log){obj, obj->list};
    for (Key *list = obj->list; list != obj->set; list --) {
        if (list->val.obj != null && list->val.obj != any ? list->val.obj->set->id == (char *)1 : 0) ex_start_scope(list->val.obj, log), ex_start_scope(list->val.data, log);
    }
}

void ex_free(Value val) {
    if (val.obj == &string || val.obj == &integrate || val.obj == &number || val.obj == &boolean) free(val.data);
    else if (val.obj != null && val.obj != any ? val.obj->set->id == (char *)1 : 0) {
        while (val.obj->list != val.obj->set) ex_free(val.obj->list->val), val.obj->list --;
        while (((Object *)val.data)->list != ((Object *)val.data)->set) ex_free(((Object *)val.data)->list->val), ((Object *)val.data)->list --; 
        free(val.obj), free(val.data);
    }
}

void ex_end_scope(Object *obj, Log *log) {
    Log *this = log;
    while (this->obj != obj) this ++;
    while (this->set != obj->list) ex_free(obj->list->val), obj->list --;
    for (Key *list = obj->list; list != obj->set; list --) {
        if (list->val.obj != null && list->val.obj != any ? list->val.obj->set->id == (char *)1 : 0) ex_end_scope(list->val.obj, log), ex_end_scope(list->val.data, log);
    }
}

long *intdup(long data) {
    long *res = malloc(sizeof(long *));
    *res = data;
    return res;
}
double *numdup(double data) {
    double *res = malloc(sizeof(double *));
    *res = data;
    return res;
}
char *chardup(char data) {
    char *res = malloc(sizeof(char *));
    *res = data;
    return res;
}

Value valdup(Value val) {
    if (val.obj == &string) val.data = strdup(val.data);
    else if (val.obj == &integrate) val.data = intdup(*(long *)val.data);
    else if (val.obj == &number) val.data = numdup(*(double *)val.data);
    return val;
}

void strcast(Value *val) {
    char *data;
    if (val->obj == null) data = "void";
    else if (val->obj == &integrate) {
        if (*(long *)val->data == 0) data = "0";
        else {
            long num = *(long *)val->data;
            char str[19], minus = 0;
            if (num < 0) minus = 1, num = -num;
            data = str, data += 18, *data = 0;
            while (num != 0) *-- data = (num % 10) + '0', num /= 10;
            if (minus) *-- data = '-';
        }
        free(val->data);
    }
    else if (val->obj == &number) {
        if (isnan(*(double *)val->data)) data = "nan";
        else if (isinf(*(double *)val->data)) data = *(double *)val->data > 0 ? "infinity" : "-infinity";
        else {
            long num1 = *(double *)val->data, num2 = (*(double *)val->data - num1) * 1e16;
            char str[19], minus = 0;
            if (num1 < 0) minus = 1, num1 = -num1, num2 = -num2;
            data = str, data += 18, *data = 0;
            if (num2 != 0) {
                while (num2 % 10 == 0) num2 /= 10;
                while (num2 != 0) *-- data = (num2 % 10) + '0', num2 /= 10;
                *-- data = '.';
            }
            if (num1 == 0) *-- data = '0';
            else while (num1 != 0) *-- data = (num1 % 10) + '0', num1 /= 10;
            if (minus) *-- data = '-';
        }
        free(val->data);
    }
    else if (val->obj == &boolean) data = *(char *)val->data ? "true" : "false", free(val->data);
    else data = "";

    if (val->obj != &string) val->obj = &string, val->data = strdup(data);
}

void intcast(Value *val) {
    long data;
    if (val->obj == null) data = 0;
    else if (val->obj == &string) data = atol(val->data), free(val->data);
    else if (val->obj == &boolean) data = *(char *)val->data, free(val->data);
    else if (val->obj == &number) data = *(double *)val->data, free(val->data);
    else data = -1;
    
    if (val->obj != &integrate) val->obj = &integrate, val->data = intdup(data);
}

void numcast(Value *val) {
    double data;
    if (val->obj == null) data = 0;
    else if (val->obj == &string) data = atof(val->data), free(val->data);
    else if (val->obj == &boolean) data = *(char *)val->data, free(val->data);
    else if (val->obj == &integrate) data = *(long *)val->data, free(val->data);
    else data = -1;
    
    if (val->obj != &number) val->obj = &number, val->data = numdup(data);
}

void boolcast(Value *val) {
    char data;
    if (val->obj == null) data = 0;
    else if (val->obj == &string) data = !is_false((char *)val->data) && *(char *)val->data != 0, free(val->data);
    else if (val->obj == &integrate) data = *(long *)val->data != 0, free(val->data);
    else if (val->obj == &number) data = isnan(*(double *)val->data) ? 0 : isinf(*(double *)val->data) ? 1 : *(double *)val->data != 0, free(val->data);
    else data = 1;

    if (val->obj != &boolean) val->obj = &boolean, val->data = chardup(data);
}

void skip_expr(char **code) {
    for (;;) {
        if (**code == PLUS || **code == MINUS || **code == NOT || **code == BITWISE_NOT) (*code) ++;

        if (**code == STRING || **code == IDENTIFIER || **code == NUMBER || **code == INT) while (*(*code) ++ != 0);
        else if (**code == TRUE || **code == FALSE || **code == VOID || **code == ANY || **code == NAN_ || **code == INFINITY_) (*code) ++;
        else if (**code == GROUP || **code == ARRAY) {
            (*code) ++;
            do skip_expr(code); while (*(*code) ++ == COMMA);
        }

        for (;;) {
            if (**code == INCREMENT_A || **code == INCREMENT_B || **code == DECREMENT_A || **code == DECREMENT_B) (*code) ++;
            else if (**code == GROUP || **code == ARRAY) {
                (*code) ++;
                do skip_expr(code); while (*(*code) ++ == COMMA);
            }
            else if (**code == PERIOD) while (*(*code) ++ != 0);
            else break;
        }

        if (**code <= ADDITION) (*code) ++;
        else break;
    }
}

void skip_code(char **code) {
    if (**code == '{') {
        (*code) ++;
        while (**code != BLOCK_E) skip_code(code);
        (*code) ++;
    }
    else if (**code == VAR) {
        (*code) ++;
        while (**code != EOL) {
            while (*(*code) ++ != 0);
            if (**code == ASSIGNMENT) (*code) ++, skip_expr(code);
        }
        (*code) ++;
    }

    else if (**code == RETURN) (*code) ++, skip_expr(code), (*code) ++;
    else {
        do skip_expr(code);
        while (*(*code) ++ == COMMA);
    }
}

Value ex_expr(char **code, Value self);
Value ex_code(char **code, Value self, char *md);

Value ex_inexpr(char **code, Value self) {
    for (;;) {
        Value res = ex_expr(code, self);
        if (**code == COMMA) (*code) ++;//, ex_free(res);
        else return res;
    }
}

Value ex_value(char **code, Value self) {
    Value res;
    if (**code == IDENTIFIER) {
        (*code) ++;
        Key *ptr = vars.list;
        while (ptr != vars.set) {
            if (strcmp(*code, ptr->id) == 0) break;
            else ptr --;
        }
        res = ptr->val;
        while (*(*code) ++ != 0);
        if (**code == ASSIGNMENT) (*code) ++, ex_free(res), res.data = ex_expr(code, self).data;
        res = valdup(res);
    }
    else if (**code == STRING) {
        res.obj = &string, res.data = strdup(++ (*code));
        while (*(*code) ++ != 0);
    }
    else if (**code == INT) {
        res.obj = &integrate, res.data = intdup(atol(++ (*code)));
        while (*(*code) ++ != 0);
    }
    else if (**code == NUMBER) {
        res.obj = &number, res.data = numdup(atof(++ (*code)));
        while (*(*code) ++ != 0);
    }
    else if (**code == TRUE) res.obj = &boolean, res.data = chardup(1), (*code) ++;
    else if (**code == FALSE) res.obj = &boolean, res.data = chardup(0), (*code) ++;
    else if (**code == VOID) res.obj = null, (*code) ++;
    else if (**code == ANY) res.obj = any, (*code) ++;
    else if (**code == SELF) {
        (*code) ++, res = self;
        if (**code == ASSIGNMENT) (*code) ++, ex_free(res), res.data = ex_expr(code, self).data;
        res = valdup(res);
    }
    // else if (**code == NAN_) res.obj = &number, res.data = numdup(NAN), (*code) ++;
    // else if (**code == INFINITY_) res.obj = &number, res.data = numdup(INFINITY), (*code) ++;
    else if (**code == FUNCTION) {
        res.obj = &function, res.data = *code;
        while (*(*code) ++ != FUNCTION_E);

        if (**code == BLOCK) {
            (*code) ++;
            while (**code != BLOCK_E) skip_code(code);
            (*code) ++;
        }
        else {
            do skip_expr(code);
            while (*(*code) ++ == COMMA);
        }
    }
    else if (**code == GROUP) {
        for (;;) {
            (*code) ++, res = ex_expr(code, self);
            if (*(*code) == COMMA) ;//ex_free(res);
            else break;
        }
        (*code) ++;
    }
    else if (**code == PLUS) {
        (*code) ++, res = ex_value(code, self);
        if (res.obj == &integrate) *(long *)res.data = +*(long *)res.data;
        else if (res.obj == &number) *(double *)res.data = +*(double *)res.data;
    }
    else if (**code == MINUS) {
        (*code) ++, res = ex_value(code, self);
        if (res.obj == &integrate) *(long *)res.data = -*(long *)res.data;
        else if (res.obj == &number) *(double *)res.data = -*(double *)res.data;
    }
    else if (**code == NOT) (*code) ++, res = ex_value(code, self), *(char *)res.data = !*(char *)res.data;
    else res.obj = null;

    return res;
}

Value ex_value_1(char **code, Value self) {
    Value res = ex_value(code, self);
    for (;;) {
        if (**code == PERIOD) {
            (*code) ++, self = res;
            Key *ptr = self.obj->list;
            while (ptr != self.obj->set) {
                if(strcmp(*code, ptr->id) == 0) break;
                else ptr --;
            }
            res = ptr->val;
            while (*(*code) ++ != 0);
            if (**code == ASSIGNMENT) (*code) ++, ex_free(res), res.data = ex_expr(code, self).data;
            res = valdup(res);
        }
        else if (**code == GROUP) {
            char *fn = res.obj == &function ? res.data : res.obj->set->val.data;
            if (*fn == '@') {
                Value arg[64], *ptr = arg;
                if (*++ (*code) == GROUP_E) (*code) ++;
                else do *ptr ++ = ex_expr(code, self); while (*(*code) ++ != GROUP_E);
                switch (fn[1]) {
                    case 0: switch (fn[2]) {
                        case 1: // puts
                        puts(arg[0].data), res.obj = null;
                        break;
                    }
                    break;
                    case 1: switch (fn[2]) {
                        case 0: // String()
                        strcast(&(arg[0])), res = valdup(arg[0]);
                        break;
                    }
                    break;
                    case 2: switch (fn[2]) {
                        case 0: // Int()
                        intcast(&(arg[0])), res = valdup(arg[0]);
                        break;
                    }
                }
                do ex_free(*ptr), ptr --; while (ptr != arg);
            }
            else  {
                int cls = 0;
                if (res.obj != &function) self = (Value){res.data}, cls = 1;

                Log log[256], *logp = log;
                ex_start_scope(&vars, &logp), fn ++;
                if (*++ (*code) == GROUP_E) (*code) ++;
                else do {
                    vars.list ++, vars.list->id = fn, vars.list->val = ex_expr(code, self);
                    while (*fn ++ != 0);
                }
                while (*(*code) ++ != GROUP_E);
                fn ++, res = (Value){null};
                
                if (*fn == BLOCK) {
                    char md = 0;
                    fn ++;
                    while (*fn != BLOCK_E) {
                        if (md == 0) res = ex_code(&fn, self, &md);
                        else skip_code(&fn);
                    }
                    fn ++;
                }
                else res = ex_inexpr(&fn, self);

                ex_end_scope(&vars, log);

                if (cls) res = self;
            }
        }
        else break;
    }
    return res;
}

// Value ex_value_2(char **code, Value self) {
//     Value res = ex_value_1(code, self);
//     if (**code == EXPONENTIATION) {
//         (*code) ++;
//         Value val = ex_value_2(code, self);
//         if (res.obj == &integrate && val.obj == &integrate) *(long *)res.data = pow(*(long *)res.data, *(long *)val.data);
//         else {
//             double l = res.obj == &number ? *(double *)res.data : *(long *)res.data, r = val.obj == &number ? *(double *)val.data : *(long *)val.data;
//             res.obj = &number, free(res.data), res.data = malloc(sizeof(double *)), *(double *)res.data = pow(l, r);
//         }
//         free(val.data);
//     }
//     return res;
// }

Value ex_value_3(char **code, Value self) {
    Value res = ex_value_1(code, self);
    for (;;) {
        if (**code == MULTIPLICATION) {
            (*code) ++;
            Value val = ex_value_1(code, self);
            if (res.obj == &integrate && val.obj == &integrate) *(long *)res.data = *(long *)res.data * *(long *)val.data;
            else {
                double l = res.obj == &number ? *(double *)res.data : *(long *)res.data, r = val.obj == &number ? *(double *)val.data : *(long *)val.data;
                res.obj = &number, free(res.data), res.data = malloc(sizeof(double *)), *(double *)res.data = l * r;
            }
            free(val.data);
        }
        else if (**code == DIVISION) {
            (*code) ++;
            Value val = ex_value_1(code, self);
            double l = res.obj == &number ? *(double *)res.data : *(long *)res.data, r = val.obj == &number ? *(double *)val.data : *(long *)val.data;
            res.obj = &number, free(res.data), res.data = malloc(sizeof(double *)), *(double *)res.data = l / r, free(val.data);
        }
        else if (**code == REMAINDER) {
            (*code) ++;
            Value val = ex_value_1(code, self);
            double l = res.obj == &number ? *(double *)res.data : *(long *)res.data, r = val.obj == &number ? *(double *)val.data : *(long *)val.data;
            res.obj = &number, free(res.data), res.data = malloc(sizeof(double *)), *(double *)res.data = l - (long)(l / r) * r, free(val.data);
        }
        else break;
    }
    return res;
}

Value ex_value_4(char **code, Value self) {
    Value res = ex_value_3(code, self);
    for (;;) {
        if (**code == ADDITION) {
            (*code) ++;
            Value val = ex_value_3(code, self);
            if (res.obj == &string || val.obj == &string) {
                strcast(&res), strcast(&val);
                int len1 = strlen(res.data), len2 = strlen(val.data);
                char *str = malloc(sizeof(char) * (len1 + len2 + 1));
                memcpy(str, res.data, sizeof(char) * len1), memcpy(str + len1, val.data, sizeof(char) * len2), *(str + len1 + len2) = 0, free(res.data), res.data = str;
            }
            else if (res.obj == &integrate && val.obj == &integrate) *(long *)res.data = *(long *)res.data + *(long *)val.data;
            else {
                double l = res.obj == &number ? *(double *)res.data : *(long *)res.data, r = val.obj == &number ? *(double *)val.data : *(long *)val.data;
                res.obj = &number, free(res.data), res.data = malloc(sizeof(double *)), *(double *)res.data = l + r;
            }
            free(val.data);
        }
        else if (**code == SUBTRACTION) {
            (*code) ++;
            Value val = ex_value_3(code, self);
            if (res.obj == &integrate && val.obj == &integrate) *(long *)res.data = *(long *)res.data - *(long *)val.data;
            else {
                double l = res.obj == &number ? *(double *)res.data : *(long *)res.data, r = val.obj == &number ? *(double *)val.data : *(long *)val.data;
                res.obj = &number, free(res.data), res.data = malloc(sizeof(double *)), *(double *)res.data = l - r;
            }
            free(val.data);
        }
        else break;
    }
    return res;
}

// Value value_5(char **code) {
//     Value res = value_4(code);
//     for (;;) {
//         if (**code == LEFT_SHIFT) {
//             (*code) ++;
//             Value val = value_4(code);
//             numcast(&res), numcast(&val), *(double *)res.data = (long)*(double *)res.data << (long)*(double *)val.data, free(val.data);
//         }
//         else if (**code == RIGHT_SHIFT) {
//             (*code) ++;
//             Value val = value_4(code);
//             numcast(&res), numcast(&val), *(double *)res.data = (long)*(double *)res.data >> (long)*(double *)val.data, free(val.data);
//         }
//         else break;
//     }
//     return res;
// }
// Value value_6(char **code) {
//     Value res = value_5(code);
//     for (;;) {
//         if (**code == LESS_THAN) {
//             (*code) ++;
//             Value val = value_5(code);
//             char tf;
//             numcast(&res), numcast(&val), tf = *(double *)res.data < *(double *)val.data, free(res.data), free(val.data), res.obj = &boolean, res.data = chardup(tf);
//         }
//         else if (**code == GREATER_THAN) {
//             (*code) ++;
//             Value val = value_5(code);
//             char tf;
//             numcast(&res), numcast(&val), tf = *(double *)res.data > *(double *)val.data, free(res.data), free(val.data), res.obj = &boolean, res.data = chardup(tf);
//         }
//         else if (**code == LESS_EQUAL) {
//             (*code) ++;
//             Value val = value_5(code);
//             char tf;
//             numcast(&res), numcast(&val), tf = *(double *)res.data <= *(double *)val.data, free(res.data), free(val.data), res.obj = &boolean, res.data = chardup(tf);
//         }
//         else if (**code == GREATER_EQUAL) {
//             (*code) ++;
//             Value val = value_5(code);
//             char tf;
//             numcast(&res), numcast(&val), tf = *(double *)res.data >= *(double *)val.data, free(res.data), free(val.data), res.obj = &boolean, res.data = chardup(tf);
//         }
//         else break;
//     }
//     return res;
// }
// Value value_7(char **code) {
//     Value res = value_6(code);
//     for (;;) {
//         if (**code == EQUALITY) {
//             (*code) ++;
//             Value val = value_6(code);
//             char tf = res.obj == val.obj ? res.obj == null ? 1 : res.obj == &string ? strcmp(res.data, val.data) == 0 : res.obj == &number ? *(double *)res.data == *(double *)val.data : res.obj == &boolean ? *(char *)res.data == *(char *)val.data : 0 : 0;
//             free(res.data), free(val.data), res.obj = &boolean, res.data = chardup(tf);
//         }
//         else if (**code == INEQUALITY) {
//             (*code) ++;
//             Value val = value_6(code);
//             char tf = res.obj == val.obj ? res.obj == null ? 1 : res.obj == &string ? strcmp(res.data, val.data) == 0 : res.obj == &number ? *(double *)res.data == *(double *)val.data : res.obj == &boolean ? *(char *)res.data == *(char *)val.data : 0 : 0;
//             free(res.data), free(val.data), res.obj = &boolean, res.data = chardup(!tf);
//         }
//         else break;
//     }
//     return res;
// }

Value ex_expr(char **code, Value self) {
    Value res = ex_value_4(code, self);
    return res;
}

/**
 * md:
 * 1 return
 * 2 continue
 * 3 break
 * 4 class
 * 5 static
 */
Value ex_code(char **code, Value self, char *md) {
    Value res = (Value){null};

    if (**code == '{') {
        Log log[256], *logp = log;
        (*code) ++, ex_start_scope(&vars, &logp);
        while (**code != BLOCK_E) {
            if (*md == 0) res = ex_code(code, self, md);
            else skip_code(code);
        }
        (*code) ++, ex_end_scope(&vars, log);
    }
    else if (**code == VAR) {
        Object *obj;
        if (*md == 4) obj = self.data, self = (Value){obj};
        else if (*md == 5) obj = self.obj;
        else obj = &vars;

        (*code) ++;
        while (**code != EOL) {
            obj->list ++, obj->list->id = *code;
            while (*(*code) ++ != 0);
            if (**code == ASSIGNMENT) (*code) ++, obj->list->val = ex_expr(code, self);
            else obj->list->val.obj = null;
        }
        (*code) ++;
    }
    else if (**code == FUNC) {
        Object *obj;
        if (*md == 4) obj = self.data, self = (Value){obj};
        else if (*md == 5) obj = self.obj;
        else obj = &vars;

        (*code) ++, obj->list ++, obj->list->id = *code;
        while (*(*code) ++ != 0);
        obj->list->val.obj = &function, obj->list->val.data = *code;
        while (*(*code) ++ != FUNCTION_E);
        if (**code == BLOCK) {
            (*code) ++;
            while (**code != BLOCK_E) skip_code(code);
            (*code) ++;
        }
        else {
            do skip_expr(code);
            while (*(*code) ++ == COMMA);
        }
        (*code) ++;
    }
    else if (**code == CLASS) {
        Object *obj;
        if (*md == 4) obj = self.data, self = (Value){obj};
        else if (*md == 5) obj = self.obj;
        else obj = &vars;

        Object *stc = malloc(sizeof(Object)), *newobj = malloc(sizeof(Object));
        stc->list = stc->set, stc->list->id = (char *)1, stc->list->val.data = (char [4]){FUNCTION, FUNCTION_E, BLOCK, BLOCK_E}, newobj->list = newobj->set;

        (*code) ++, obj->list ++, obj->list->id = *code, obj->list->val = (Value){stc, newobj};
        while (*(*code) ++ != 0);
        (*code) ++;
        char clmd = 4;
        while (**code != BLOCK_E) ex_code(code, obj->list->val, &clmd);
        (*code) ++;
    }
    else if (**code == STATIC) {
        char stcmd = 5;
        (*code) ++, ex_code(code, self, &stcmd);
    }
    else if (**code == INIT) {
        (*code) ++, self.obj->set->val.data = *code;
        while (*(*code) ++ != FUNCTION_E);
        if (**code == BLOCK) {
            (*code) ++;
            while (**code != BLOCK_E) skip_code(code);
            (*code) ++;
        }
        else {
            do skip_expr(code);
            while (*(*code) ++ == COMMA);
        }
        (*code) ++;
    }
    else if (**code == EXTEND) {
        Object *obj;
        if (*md == 4) obj = self.data, self = (Value){obj};
        else if (*md == 5) obj = self.obj;
        else obj = &vars;

        Key *list = obj->list;
        (*code) ++;
        while (strcmp(*code, list->id) != 0) list --;
        while (*(*code) ++ != 0);
        (*code) ++;
        char clmd = 4;
        while (**code != BLOCK_E) ex_code(code, list->val, &clmd);
        (*code) ++;
    }

    else if (**code == RETURN) (*code) ++, res = ex_expr(code, self), (*code) ++, *md = 1;
    else {
        // do ex_free(ex_expr(code, self));
        do ex_expr(code, self);
        while (*(*code)++ == COMMA);
    }

    return res;
}






int typecmp(Value val1, Value val2) {
    if (val1.obj != val2.obj) return 0;
    if (val1.obj == &function) {
        Value *arg1 = val1.data, *arg2 = val2.data;
        for (;;) {
            if (!typecmp(*arg1, *arg2)) return 0;
            if (arg1->obj == undefined) break;
            arg1 ++, arg2 ++;
        }
    }
    return 1;
}

void error(char *file, char *ptr, char *text, int count) {
    int column = 0, line = 1;
    char *lptr = ptr;
    while (*lptr != 0) {
        lptr --;
        if (*lptr == '\n') line ++;
    }
    do ptr --, column ++; while (*ptr != '\n' && *ptr != 0);
    printf("%s line %d, column %d\n ", realpath(file, NULL), line, column), ptr ++;
    while (*ptr == ' ') ptr ++, column --;
    while (*ptr != '\n' && *ptr != 0) fputc(*ptr, stdout), ptr ++;
    puts("\e[92m");
    while (column > 0) fputc(' ', stdout), column --;
    while (count > 0) fputc('^', stdout), count --;
    puts("\e[0m"), puts(text), exit(EXIT_FAILURE);
}

Value traversal(char **ptr, char **code, char *file, Object *obj) {
    if (!is_id(**ptr)) error(file, *ptr, ERROR2, 1);
    if (is_if(*ptr) || is_do(*ptr)) error(file, *ptr, ERROR6, 2);
    if (is_for(*ptr) || is_var(*ptr) || is_nan(*ptr) || is_any(*ptr)) error(file, *ptr, ERROR6, 3);
    if (is_true(*ptr) || is_self(*ptr) || is_void(*ptr) || is_else(*ptr) || is_case(*ptr) || is_func(*ptr) || is_init(*ptr)) error(file, *ptr, ERROR6, 4);
    if (is_false(*ptr) || is_class(*ptr) || is_while(*ptr) || is_break(*ptr)) error(file, *ptr, ERROR6, 5);
    if (is_switch(*ptr) || is_return(*ptr)) error(file, *ptr, ERROR6, 6);
    if (is_default(*ptr)) error(file, *ptr, ERROR6, 7);
    if (is_continue(*ptr) || is_infinity(*ptr)) error(file, *ptr, ERROR6, 8);

    char *id = *code;
    do *(*code)++ = *(*ptr) ++; while (is_id(**ptr) || is_number(**ptr)); *(*code) ++ = 0;
    for (Key *list = obj->list; list != obj->set; list --) if (strcmp(id, list->id) == 0) return list->val;
    return (Value){undefined};
}

void slide(char **ptr, char *file, int line) {
    for (;;) {
        if (**ptr == ' ') (*ptr) ++;
        else if (line && **ptr == '\n') (*ptr) ++;
        else if (**ptr == '/' && (*ptr)[1] == '/') {
            if (line) {
                (*ptr) += 2;
                while (**ptr != '\n' && **ptr != 0) (*ptr) ++;
            }
            else error(file, *ptr, "SyntaxError: Invalid line comment", 2);
        }
        else if (**ptr == '/' && (*ptr)[1] == '*') {
            char *tmp_ptr = *ptr;
            (*ptr) += 2;
            while ((*ptr)[0] != '*' || (*ptr)[1] != '/') {
                if (**ptr == 0) error(file, tmp_ptr, "SyntaxError: Unterminated comment block", 2);
                (*ptr) ++;
            }
            (*ptr) += 2;
        }
        else break;
    }
}


void ch_start_scope(Object *obj, Log **log) {
    *((*log) ++) = (Log){obj, obj->list};
    for (Key *list = obj->list; list != obj->set; list --) {
        if (list->val.obj != null && list->val.obj != any ? list->val.obj->set->id == (char *)1 : 0) ch_start_scope(list->val.obj, log), ch_start_scope(list->val.data, log);
    }
}

void ch_free(Value val) {
    if (val.obj == &function) free(val.data);
    else if (val.obj != null && val.obj != any ? val.obj->set->id == (char *)1 : 0) {
        while (val.obj->list != val.obj->set) ch_free(val.obj->list->val), val.obj->list --;
        while (((Object *)val.data)->list != ((Object *)val.data)->set) ch_free(((Object *)val.data)->list->val), ((Object *)val.data)->list --;
        free(val.obj), free(val.data);
    }
}

void ch_end_scope(Object *obj, Log *log) {
    Log *this = log;
    while (this->obj != obj) this ++;
    while (this->set != obj->list) ch_free(obj->list->val), obj->list --;
    for (Key *list = obj->list; list != obj->set; list --) {
        if (list->val.obj != null && list->val.obj != any ? list->val.obj->set->id == (char *)1 : 0) ch_end_scope(list->val.obj, log), ch_end_scope(list->val.data, log);
    }
}

Value ch_type(char **ptr, char **code, char *file, Value self) {
    Value res;
    if (is_any(*ptr)) (*ptr) += 3,       res.obj = any, slide(ptr, file, 0);
    else if (is_void(*ptr)) (*ptr) += 4, res.obj = null, slide(ptr, file, 0);
    else if (is_func(*ptr)) error(file, *ptr, "SyntaxError: 開発中の機能", 4);
    // else if (is_func(*ptr)) {
    //     (*ptr) += 4, slide(ptr, file, 0), ch_start_scope(&vars), inparam(ptr, code, file, self);
    //     res.obj = &function, res.data = malloc(sizeof(Value) * (vars.list - vars.save + 2));
    //     Value *ret = res.data, *arg = ret + 1;
    //     Key* list = vars.save + 1;
    //     do *arg ++ = (list ++)->val; while (list != vars.list + 1);
    //     arg->obj = undefined;
    //     if (**ptr == ':') (*ptr) ++, slide(ptr, file, 0), *ret = ch_type(ptr, code, file, self);
    //     else ret->obj = null;
    //     ch_end_scope(&vars);
    // }
    else {
        res.obj = &vars;
        for (;;) {
            char *nm_ptr = *ptr, id[256], *idp = id;
            Value val = traversal(ptr, &idp, file, res.obj);
            if (val.obj == undefined) error(file, nm_ptr, "SyntaxError: Undeclared identifier", (*ptr) - nm_ptr);
            if (val.obj != null && val.obj != any ? val.obj->set->id != (char *)1 : 1) error(file, nm_ptr, "SyntaxError: Invalid value", (*ptr) - nm_ptr);

            res.obj = val.data, slide(ptr, file, 0);
            if (**ptr == '.') (*ptr) ++, slide(ptr, file, 0);
            else break;
        }
    }
    return res;
}

Value ch_expr(char **ptr, char **code, char *file, Value self);

Value inexpr(char **ptr, char **code, char *file, Value self) {
    for (;;) {
        Value res = ch_expr(ptr, code, file, self);
        if (**ptr == ',') {
            (*ptr) ++, *(*code)++ = COMMA, slide(ptr, file, 1);
        }
        else return res;
    }
}

int ch_code(char **ptr, char **code, char *file, Value *func, int md, Value self);

void in_args(char **ptr, char **code, char *file, Value self) {
    if (**ptr != '(') error(file, *ptr, ERROR2, 1);
    char *gst_ptr = *ptr;
    (*ptr) ++, *(*code)++ = FUNCTION, slide(ptr, file, 1);
    if (**ptr != ')') {
        Value val;
        for (;;) {
            if (**ptr == 0) error(file, gst_ptr, "SyntaxError: Unclosed '('", 1);
            char *nm_ptr = *ptr, *id = *code;
            if (traversal(ptr, code, file, &vars).obj != undefined) error(file, nm_ptr, "SyntaxError: Declared identifier", (*ptr) - nm_ptr);
            slide(ptr, file, 0);
            if (**ptr != ':') error(file, *ptr, ERROR2, 1);
            (*ptr) ++, slide(ptr, file, 0), val = ch_type(ptr, code, file, self), vars.list ++, vars.list->id = id, vars.list->val = val;
            if (**ptr == ',') (*ptr) ++, slide(ptr, file, 1);
            else break;
        }
        slide(ptr, file, 1);
        if (**ptr == 0) error(file, gst_ptr, "SyntaxError: Unclosed '('", 1);
        else if (**ptr != ')') error(file, *ptr, ERROR2, 1);
    }
    (*ptr) ++, slide(ptr, file, 0), *(*code)++ = FUNCTION_E;
}

void in_func(char **ptr, char **code, char *file, Value self, Value *ret) {
    slide(ptr, file, 1);
    if (**ptr == '{') {
        char *tmp_ptr = *ptr, set = 0;
        (*ptr) ++, *(*code)++ = BLOCK, slide(ptr, file, 1);
        while (**ptr != '}') {
            if (**ptr == 0) error(file, tmp_ptr, "SyntaxError: Unclosed '{'", 1);
            else if (ch_code(ptr, code, file, ret, 0, self)) set = 1;
        }
        (*ptr) ++, *(*code)++ = BLOCK_E;
        if (ret != NULL && set == 0) {
            if (ret->obj == undefined) ret->obj = null;
            else if (ret->obj != null) error(file, tmp_ptr, "SyntaxError: 戻り値がない", 1);
        }
    }
    else if (**ptr == '-' && (*ptr)[1] == '>') {
        (*ptr) += 2, slide(ptr, file, 0);
        if (ret == NULL) inexpr(ptr, code, file, self);
        else {
            char *tmp_ptr = *ptr;
            Value val = inexpr(ptr, code, file, self);
            if (ret->obj == undefined) *ret = val;
            else if (ret->obj != any && !typecmp(*ret, val)) error(file, tmp_ptr, "SyntaxError: Type mismatch", (*ptr) - tmp_ptr);
        }
    }
    else if (**ptr == '\n' || **ptr == 0) error(file, *ptr, "SyntaxError: Invalid termination", 1);
    else error(file, *ptr, ERROR2, 1);
}

Value ch_valdup(Value val) {
    if (val.obj == &function) {
        Value *arg = val.data;
        char len = 1;
        while (arg->obj != undefined) len ++, arg ++;
        Value *arg1 = val.data, *arg2 = malloc(sizeof(Value) * len);
        val.data = arg2;
        while (arg1->obj != undefined) *arg2 ++ = *arg1 ++;
        arg2->obj = undefined;
    }
    return val;
}

Value ch_value(char **ptr, char **code, char *file, Value self) {

    Value res;
    char prefix = 0, *preptr, var = 0;
    if (**ptr == '+' && (*ptr)[1] == '+')      preptr = *ptr, (*ptr) += 2, prefix = INCREMENT_B, slide(ptr, file, 0);
    else if (**ptr == '-' && (*ptr)[1] == '-') preptr = *ptr, (*ptr) += 2, prefix = DECREMENT_B, slide(ptr, file, 0);

    if (is_number(**ptr)) {
        char *str = *code;
        res.obj = &integrate, *(*code)++ = INT;
        do *(*code)++ = *(*ptr)++; while (is_number(**ptr));
        if ((*ptr)[0] == '.' && is_number((*ptr)[1])) {
            res.obj = &number, *str = NUMBER;
            do *(*code)++ = *(*ptr)++; while (is_number(**ptr));
        }
        if (is_id(**ptr)) error(file, *ptr, "SyntaxError: Invalid numeric literal", 1);
        *(*code)++ = 0;
    }
    else if (**ptr == '"' || **ptr == '\'') {
        char end = **ptr;
        res.obj = &string, *(*code)++ = STRING, (*ptr) ++;
        while (**ptr != end) {
            if (**ptr == '\n' || **ptr == 0) error(file, *ptr, "SyntaxError: Unterminated string literal", 1);
            else if (**ptr == '\\') {
                if ((*ptr)[1] == 'a') *(*code)++ = '\a', (*ptr) += 2;
                else if ((*ptr)[1] == 'b') *(*code)++ = '\b', (*ptr) += 2;
                else if ((*ptr)[1] == 't') *(*code)++ = '\t', (*ptr) += 2;
                else if ((*ptr)[1] == 'n') *(*code)++ = '\n', (*ptr) += 2;
                else if ((*ptr)[1] == 'v') *(*code)++ = '\v', (*ptr) += 2;
                else if ((*ptr)[1] == 'f') *(*code)++ = '\f', (*ptr) += 2;
                else if ((*ptr)[1] == 'r') *(*code)++ = '\r', (*ptr) += 2;
                else if ((*ptr)[1] == 'e') *(*code)++ = '\e', (*ptr) += 2;
                else if ((*ptr)[1] == 'x' && is_hexadecimal((*ptr)[2]) && is_hexadecimal((*ptr)[3])) *(*code)++ = (atox((*ptr)[2]) * 0x10 + atox((*ptr)[3])), (*ptr) += 4;
                else if ((*ptr)[1] == 'u' && is_hexadecimal((*ptr)[2]) && is_hexadecimal((*ptr)[3]) && is_hexadecimal((*ptr)[4]) && is_hexadecimal((*ptr)[5])) *(*code)++ = (atox((*ptr)[2]) * 0x1000 + atox((*ptr)[3]) * 0x100 + atox((*ptr)[4]) * 0x10 + atox((*ptr)[5])), (*ptr) += 6;
                else if (is_octadecimal((*ptr)[1]) && is_octadecimal((*ptr)[2]) && is_octadecimal((*ptr)[3])) *(*code)++ = (((*ptr)[1] - 48) * 64 + ((*ptr)[2] - 48) * 8 + ((*ptr)[3] - 48)), (*ptr) += 4;
                else *(*code)++ = ((*ptr)[1]), (*ptr) += 2;
            }
            else *(*code)++ = *(*ptr) ++;
        }
        (*ptr) ++, *(*code)++ = 0;
    }
    else if (is_id(**ptr)) {
        if (is_any(*ptr)) (*ptr) += 3,              *(*code)++ = ANY;
        else if (is_true(*ptr)) (*ptr) += 4,        *(*code)++ = TRUE,      res.obj = &boolean;
        else if (is_false(*ptr)) (*ptr) += 5,       *(*code)++ = FALSE,     res.obj = &boolean;
        else if (is_void(*ptr)) (*ptr) += 4,        *(*code)++ = VOID;
        else if (is_nan(*ptr)) (*ptr) += 3,         *(*code)++ = NAN_,      res.obj = &number;
        else if (is_infinity(*ptr)) (*ptr) += 8,    *(*code)++ = INFINITY_, res.obj = &number;
        else if (is_func(*ptr)) {
            Key *list = vars.list;
            Value *ret, *arg;
            Log log[256], *logp = log;
            ch_start_scope(&vars, &logp), (*ptr) += 4, slide(ptr, file, 0), in_args(ptr, code, file, self), res.obj = &function, ret = res.data = malloc(sizeof(Value) * (vars.list - list + 3)), *ret ++ = self, arg = ret + 1;
            while (list != vars.list) list ++, *arg ++ = list->val;
            arg->obj = undefined;

            if (**ptr == ':') (*ptr) ++, slide(ptr, file, 0), *ret = ch_type(ptr, code, file, self);
            else ret->obj = undefined;

            in_func(ptr, code, file, self, ret), ch_end_scope(&vars, log);
        }
        else if (is_self(*ptr)) {
            if (self.obj == null) error(file, *ptr, ERROR1, 4);
            (*ptr) += 4, *(*code)++ = SELF, res = self;
        }
        else if (is_if(*ptr) || is_do(*ptr))                            error(file, *ptr, ERROR1, 2);
        else if (is_for(*ptr) || is_var(*ptr))                          error(file, *ptr, ERROR1, 3);
        else if (is_else(*ptr) || is_case(*ptr))                        error(file, *ptr, ERROR1, 4);
        else if (is_while(*ptr) || is_break(*ptr) || is_class(*ptr))    error(file, *ptr, ERROR1, 5);
        else if (is_switch(*ptr) || is_return(*ptr))                    error(file, *ptr, ERROR1, 6);
        else if (is_default(*ptr))                                      error(file, *ptr, ERROR1, 7);
        else if (is_continue(*ptr))                                     error(file, *ptr, ERROR1, 8);
        else {
            *(*code) ++ = IDENTIFIER;
            char *nm_ptr = *ptr, *id = *code;
            do *(*code)++ = *(*ptr) ++; while (is_id(**ptr) || is_number(**ptr)); *(*code)++ = 0;
            Key *list = vars.list;
            for (;;) {
                if (list == vars.set) error(file, nm_ptr, "SyntaxError: Undeclared identifier", (*ptr) - nm_ptr);
                else if (strcmp(id, list->id) == 0) {
                    res = list->val, var = 1;
                    break;
                }
                else list --;
            }
        }
    }
    else if (**ptr == '(') {
        char *gst_ptr = *ptr;
        (*ptr) ++, *(*code)++ = GROUP, slide(ptr, file, 1);
        if (**ptr == ')') res.obj = null;
        else {
            for (;;) {
                if (**ptr == 0) error(file, gst_ptr, "SyntaxError: Unclosed '('", 1);
                res = ch_expr(ptr, code, file, self);
                if (**ptr == ',') (*ptr) ++, *(*code)++ = COMMA, slide(ptr, file, 1);
                else {
                    slide(ptr, file, 1);
                    if (**ptr == 0) error(file, gst_ptr, "SyntaxError: Unclosed '('", 1);
                    if (**ptr != ')') error(file, *ptr, ERROR2, 1);
                    break;
                }
            }
        }
        (*ptr) ++, *(*code)++ = GROUP_E;
    }
    else if (**ptr == '[') error(file, *ptr, "配列は作り途中", 1);
    else if (**ptr == '!') {
        char *tmp_ptr = *ptr;
        (*ptr) ++, *(*code)++ = NOT, slide(ptr, file, 0), res = ch_value(ptr, code, file, self);
        if (res.obj != &boolean) error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
    }
    else if (**ptr == '-') {
        char *tmp_ptr = *ptr;
        (*ptr) ++, *(*code)++ = MINUS, slide(ptr, file, 0), res = ch_value(ptr, code, file, self);
        if (res.obj != &integrate && res.obj != &number) error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
    }
    else if (**ptr == '+') {
        char *tmp_ptr = *ptr;
        (*ptr) ++, *(*code)++ = PLUS, slide(ptr, file, 0), res = ch_value(ptr, code, file, self);
        if (res.obj != &integrate && res.obj != &number) error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
    }
    else if (**ptr == '}') error(file, *ptr, "SyntaxError: Unmatched '}'", 1);
    else if (**ptr == ')') error(file, *ptr, "SyntaxError: Unmatched ')'", 1);
    else if (**ptr == ']') error(file, *ptr, "SyntaxError: Unmatched ']'", 1);
    else if (**ptr == '\n' || **ptr == 0) error(file, *ptr, "SyntaxError: Invalid termination", 1);
    else error(file, *ptr, ERROR2, 1);

    for (;;) {
        slide(ptr, file, 0);
        if (**ptr == '.') {
            if (res.obj == null || res.obj == any) error(file, *ptr, "SyntaxError: Invalid operator", 1);
            (*ptr)++, *(*code)++ = PERIOD, slide(ptr, file, 0), self = res, var = 1;
            char *nm_ptr = *ptr;
            Value val = traversal(ptr, code, file, self.obj);
            if (val.obj == undefined) error(file, nm_ptr, "SyntaxError: Undeclared identifier", (*ptr) - nm_ptr);
            else res = val;
        }
        else if (**ptr == '(') {
            Value *param;
            if (res.obj == &function) {
                if (((Value *)res.data)->obj != any && !typecmp(*(Value *)res.data, self)) error(file, *ptr, "SyntaxError: self", 1);
                param = (Value *)res.data + 2, res = *((Value *)res.data + 1);
            }
            else if (res.obj != null && res.obj != any ? res.obj->set->id == (char *)1 : 0) param = (Value *)res.obj->set->val.data, res = (Value){res.data};
            else error(file, *ptr, "SyntaxError: Invalid operator", 1);

            char *gst_ptr = *ptr;
            (*ptr) ++, *(*code) ++ = GROUP, slide(ptr, file, 1);
            if (**ptr != ')') for (;;) {
                char *val_ptr = *ptr;
                if (**ptr == 0) error(file, gst_ptr, "SyntaxError: Unclosed '('", 1);
                if (param->obj == undefined) inexpr(ptr, code, file, self), error(file, val_ptr, "SyntaxError: Extra argument", (*ptr) - val_ptr);
                Value val = ch_expr(ptr, code, file, self);
                if (param->obj != any && !typecmp(*param, val)) error(file, val_ptr, "SyntaxError: Type mismatch", (*ptr) - val_ptr);
                if (**ptr == ',') (*ptr) ++, *(*code)++ = COMMA, slide(ptr, file, 1), param ++;
                else {
                    slide(ptr, file, 1), param ++;
                    if (**ptr == 0) error(file, gst_ptr, "SyntaxError: Unclosed '('", 1);
                    if (**ptr != ')') error(file, *ptr, ERROR2, 1);
                    break;
                }
            }
            if (param->obj != undefined) error(file, *ptr, "SyntaxError: Missing argument", 1);
            (*ptr) ++, *(*code)++ = GROUP_E, var = 0;
        }
        else if (**ptr == '[') {
        }
        else break;
    }

    if (prefix != 0) {
        if ((res.obj != &number && res.obj != any) || var == 0) error(file, preptr, "SyntaxError: Invalid operator", 2);
        *(*code) ++ = prefix;
    }

    if (**ptr == '+' && (*ptr)[1] == '+') {
        if ((res.obj != &number && res.obj != any) || prefix != 0 || var == 0) error(file, *ptr,  "SyntaxError: Invalid operator", 2);
        (*ptr) += 2, *(*code)++ = INCREMENT_A;
    }
    else if (**ptr == '-' && (*ptr)[1] == '-') {
        if ((res.obj != &number && res.obj != any) || prefix != 0 || var == 0) error(file, *ptr,  "SyntaxError: Invalid operator", 2);
        (*ptr) += 2, *(*code)++ = DECREMENT_A;
    }
    else if (**ptr == '=' && (*ptr)[1] != '=') {
        if (prefix != 0) error(file, *ptr, "SyntaxError: Invalid operator", 1);
        (*ptr) ++, *(*code)++ = ASSIGNMENT, slide(ptr, file, 0);
        char *tmp_ptr = *ptr;
        Value val = ch_expr(ptr, code, file, self);
        if (res.obj != any && !typecmp(res, val)) error(file, tmp_ptr, "TypeError: Cant assign value (type mismatch)", (*ptr) - tmp_ptr);
    }

    return res;
}

Value ch_value_1(char **ptr, char **code, char *file, Value self) {
    Value res = ch_value(ptr, code, file, self);
    if ((*ptr)[0] == '*' && (*ptr)[1] == '*') {
        char *tmp_ptr = *ptr;
        (*ptr) += 2, *(*code)++ = EXPONENTIATION, slide(ptr, file, 0);
        Value val = ch_value_1(ptr, code, file, self);
        if (res.obj == &integrate && val.obj == &integrate) res.obj = &integrate;
        else if ((res.obj == &integrate || res.obj == &number) && (val.obj == &integrate || val.obj == &number)) res.obj = &number;
        else error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
    }
    return res;
}

Value ch_value_2(char **ptr, char **code, char *file, Value self) {
    Value res = ch_value_1(ptr, code, file, self);
    for (;;) {
        if (**ptr == '*') {
            char *tmp_ptr = *ptr;
            (*ptr) ++, *(*code)++ = MULTIPLICATION, slide(ptr, file, 0);
            Value val = ch_value_1(ptr, code, file, self);
            if (res.obj == &integrate && val.obj == &integrate) res.obj = &integrate;
            else if ((res.obj == &integrate || res.obj == &number) && (val.obj == &integrate || val.obj == &number)) res.obj = &number;
            else error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
        }
        else if (**ptr == '/') {
            char *tmp_ptr = *ptr;
            (*ptr) ++, *(*code)++ = DIVISION, slide(ptr, file, 0);
            Value val = ch_value_1(ptr, code, file, self);
            if ((res.obj == &integrate || res.obj == &number) && (val.obj == &integrate || val.obj == &number)) res.obj = &number;
            else error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
        }
        else if (**ptr == '%') {
            char *tmp_ptr = *ptr;
            (*ptr) ++, *(*code)++ = REMAINDER, slide(ptr, file, 0);
            Value val = ch_value_1(ptr, code, file, self);
            if ((res.obj == &integrate || res.obj == &number) && (val.obj == &integrate || val.obj == &number)) res.obj = &number;
            else error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
        }
        else break;
    }
    return res;
}

Value ch_value_3(char **ptr, char **code, char *file, Value self) {
    Value res = ch_value_2(ptr, code, file, self);
    for (;;) {
        if (**ptr == '+') {
            char *tmp_ptr = *ptr;
            (*ptr) ++, *(*code)++ = ADDITION, slide(ptr, file, 0);
            Value val = ch_value_2(ptr, code, file, self);
            if (res.obj == &string || val.obj == &string) res.obj = &string;
            else if (res.obj == &integrate && val.obj == &integrate) res.obj = &integrate;
            else if ((res.obj == &integrate || res.obj == &number) && (val.obj == &integrate || val.obj == &number)) res.obj = &number;
            else error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
        }
        else if (**ptr == '-') {
            char *tmp_ptr = *ptr;
            (*ptr) ++, *(*code)++ = SUBTRACTION, slide(ptr, file, 0);
            Value val = ch_value_2(ptr, code, file, self);
            if (res.obj == &integrate && val.obj == &integrate) res.obj = &integrate;
            else if ((res.obj == &integrate || res.obj == &number) && (val.obj == &integrate || val.obj == &number)) res.obj = &number;
            else error(file, tmp_ptr, "SyntaxError: Invalid operator", 1);
        }
        else break;
    }
    return res;
}

Value ch_expr(char **ptr, char **code, char *file, Value self) {
    Value res = ch_value_3(ptr, code, file, self);
    for (;;) {
        if (**ptr == '=' && (*ptr)[1] == '=') {
            (*ptr) += 2, *(*code)++ = EQUALITY, slide(ptr, file, 0), ch_value_3(ptr, code, file, self);
            res.obj = &boolean;
        }
        else if (**ptr == '!' && (*ptr)[1] == '=') {
            (*ptr) += 2, *(*code)++ = INEQUALITY, slide(ptr, file, 0), ch_value_3(ptr, code, file, self);
            res.obj = &boolean;
        }
        else break;
    }
    return res;
}


void readfile(FILE *fp, char **code, char *file, Value *func, int md, Value self) {
    fseek(fp, 0, SEEK_END);
    long length = ftell(fp);
    char *fileptr = malloc(sizeof(char) * (length + 1)), *tmpptr = fileptr, **ptr = &tmpptr;
    fseek(fp, 0, SEEK_SET), fread(fileptr, 1, length, fp), fclose(fp);

    slide(ptr, file, 1);
    while (**ptr != 0) ch_code(ptr, code, file, func, md, self);
    free(fileptr);
}

/**
 * md
 * 0 normal
 * 1 loop
 * 2 declare
 * 3 static
 */
int ch_code(char **ptr, char **code, char *file, Value *func, int md, Value self) {

    int res = 0;
    if (**ptr == '{') {
        if (md >= 2) error(file, *ptr, ERROR1, 1);
        char *tmp_ptr = *ptr;
        Log log[256], *logp = log;
        (*ptr) ++, *(*code)++ = BLOCK, slide(ptr, file, 1), ch_start_scope(&vars, &logp);
        while (**ptr != '}') {
            if (**ptr == 0) error(file, tmp_ptr, "SyntaxError: Unclosed '{'", 1);
            else if (ch_code(ptr, code, file, func, md, self)) res = 1;
        }
        (*ptr) ++, *(*code)++ = BLOCK_E, ch_end_scope(&vars, log);
    }
    else if (is_var(*ptr)) {
        (*ptr) += 3, *(*code)++ = VAR, slide(ptr, file, 1);
        Object *obj;
        if (md == 2) obj = self.data, self = (Value){obj};
        else if (md == 3) obj = self.obj;
        else obj = &vars;

        for (;;) {
            char *nm_ptr = *ptr, *id = *code;
            Value val = traversal(ptr, code, file, obj);
            if (val.obj != undefined) error(file, nm_ptr, "SyntaxError: Declared identifier", (*ptr) - nm_ptr);     
            slide(ptr, file, 0);

            if (**ptr == ':') (*ptr) ++, slide(ptr, file, 0), val = ch_type(ptr, code, file, self);
            if (**ptr == '=') {
                (*ptr) ++, *(*code)++ = ASSIGNMENT, slide(ptr, file, 0);
                if (val.obj == undefined) val = ch_expr(ptr, code, file, self);
                else {
                    char *tmp_ptr = *ptr;
                    Value val2 =  ch_expr(ptr, code, file, self);
                    if (val.obj != any && !typecmp(val, val2)) error(file, tmp_ptr, "SyntaxError: Type mismatch", (*ptr) - tmp_ptr);
                }
            }
            if (**ptr == '{') {
                char *st_ptr = *ptr, get = 0, set = 0;
                (*ptr) ++, *(*code)++ = BLOCK, slide(ptr, file, 1);
                while (**ptr != '}') {
                    if (is_get(*ptr)) {
                        if (get) error(file, *ptr, "SyntaxError: Getterは定義済み", 3);
                        get = 1, (*ptr) += 3, *(*code)++ = GET, slide(ptr, file, 1);
                        char *tmp_ptr = *ptr;
                        if (!ch_code(ptr, code, file, &val, 0, self)) {
                            if (val.obj == undefined) val.obj = null;
                            else if (val.obj != null) error(file, tmp_ptr, "SyntaxError: 戻り値がない", 1);
                        }
                    }
                    else if (is_set(*ptr)) {
                        if (set) error(file, *ptr, "SyntaxError: Setterは定義済み", 3);
                        set = 1, (*ptr) += 3, *(*code)++ = SET, slide(ptr, file, 1), ch_code(ptr, code, file, NULL, 0, self);
                    }
                    else if (**ptr == 0) error(file, st_ptr, "SyntaxError: Unclosed '{'", 1);
                    else error(file, *ptr, ERROR2, 1);
                }
                if (get == 0) error(file, st_ptr, "SyntaxError: Getterがない", 1);
                (*ptr) ++, *(*code)++ = BLOCK_E;
            }
            if (val.obj == undefined) val.obj = null;
            obj->list ++, obj->list->id = id, obj->list->val = val;
            
            if (**ptr == ',') (*ptr) ++, slide(ptr, file, 1);
            else break;
        }

        if (**ptr == ';' || **ptr == '\n' || **ptr == 0) (*ptr) ++, *(*code)++ = EOL;
        else error(file, *ptr, ERROR2, 1);
    }
    else if (is_func(*ptr)) {
        (*ptr) += 4, *(*code)++ = FUNC, slide(ptr, file, 1);
        Object *obj;
        if (md == 2) obj = self.data, self = (Value){obj};
        else if (md == 3) obj = self.obj;
        else obj = &vars;

        char *nm_ptr = *ptr, *id = *code;
        Value val = traversal(ptr, code, file, obj), *ret, *arg;
        if (val.obj != undefined) error(file, nm_ptr, "SyntaxError: Declared identifier", (*ptr) - nm_ptr);
        
        Key *func = ++ obj->list, *list = vars.list;
        Log log[256], *logp = log;
        func->id = "", slide(ptr, file, 0), ch_start_scope(&vars, &logp), in_args(ptr, code, file, self), func->val.obj = &function, ret = func->val.data = malloc(sizeof(Value) * (vars.list - list + 3)), *ret ++ = self, arg = ret + 1;
        while (list != vars.list) list ++, *arg ++ = list->val;
        arg->obj = undefined;

        if (**ptr == ':') (*ptr) ++, slide(ptr, file, 0), *ret = ch_type(ptr, code, file, self), func->id = id, in_func(ptr, code, file, self, ret);
        else ret->obj = undefined, in_func(ptr, code, file, self, ret), func->id = id;
        ch_end_scope(&vars, log);

        if (**ptr == ';' || **ptr == '\n' || **ptr == 0) (*ptr) ++, *(*code)++ = EOL;
        else error(file, *ptr, ERROR2, 1);
    }
    else if (is_class(*ptr)) {
        (*ptr) += 5, *(*code)++ = CLASS, slide(ptr, file, 1);
        Object *obj;
        if (md == 2) obj = self.data, self = (Value){obj};
        else if (md == 3) obj = self.obj;
        else obj = &vars;

        char *nm_ptr = *ptr, *id = *code;
        Value val = traversal(ptr, code, file, obj);
        if (val.obj != undefined) error(file, nm_ptr, "SyntaxError: Declared identifier", (*ptr) - nm_ptr);

        Object *stc = malloc(sizeof(Object)), *newobj = malloc(sizeof(Object));
        stc->list = stc->set, stc->list->id = (char *)1, stc->list->val.data = (Value [1]){{undefined}}, newobj->list = newobj->set;
        slide(ptr, file, 1), obj->list ++, obj->list->id = id, obj->list->val = (Value){stc, newobj};

        if (**ptr != '{') error(file, *ptr, ERROR2, 1);
        char *tmp_ptr = *ptr;
        (*ptr) ++, *(*code)++ = BLOCK, slide(ptr, file, 1);
        while (**ptr != '}') {
            if (**ptr == 0) error(file, tmp_ptr, "SyntaxError: Unclosed '{'", 1);
            else ch_code(ptr, code, file, NULL, 2, obj->list->val);
        }
        (*ptr) ++, *(*code)++ = BLOCK_E;
    }
    else if (is_static((*ptr))) {
        if (md != 2) error(file, *ptr, ERROR1, 6);
        (*ptr) += 6, *(*code)++ = STATIC, slide(ptr, file, 1), ch_code(ptr, code, file, func, 3, self);
    }
    else if (is_init(*ptr)) {
        if (md != 2) error(file, *ptr, ERROR1, 4);
        Key *list = vars.list;
        Value *arg;
        Log log[256], *logp = log;
        (*ptr) += 4, *(*code)++ = INIT, slide(ptr, file, 0), ch_start_scope(&vars, &logp), in_args(ptr, code, file, (Value){self.data});
        arg = self.obj->set->val.data = malloc(sizeof(Value) * (vars.list - list + 1));
        while (list != vars.list) list ++, *arg ++ = list->val;
        arg->obj = undefined, in_func(ptr, code, file, (Value){self.data}, NULL), ch_end_scope(&vars, log);

        if (**ptr == ';' || **ptr == '\n' || **ptr == 0) (*ptr) ++, *(*code)++ = EOL;
        else error(file, *ptr, ERROR2, 1);
    }
    else if (is_extend(*ptr)) {
        (*ptr) += 6, *(*code)++ = EXTEND, slide(ptr, file, 1);
        Object *obj;
        if (md == 2) obj = self.data, self = (Value){obj};
        else if (md == 3) obj = self.obj;
        else obj = &vars;

        char *nm_ptr = *ptr, *id = *code;
        Value val = traversal(ptr, code, file, obj);
        if (val.obj == undefined || val.obj == null || val.obj == any ? 1 : val.obj->set->id != (char *)1) error(file, nm_ptr, "SyntaxError: Undeclared class", (*ptr) - nm_ptr);
        slide(ptr, file, 1);

        if (**ptr != '{') error(file, *ptr, ERROR2, 1);
        char *tmp_ptr = *ptr;
        (*ptr) ++, *(*code)++ = BLOCK, slide(ptr, file, 1);
        while (**ptr != '}') {
            if (**ptr == 0) error(file, tmp_ptr, "SyntaxError: Unclosed '{'", 1);
            else ch_code(ptr, code, file, NULL, 2, val);
        }
        (*ptr) ++, *(*code)++ = BLOCK_E;
    }
    
    // else if (is_if(*ptr)) {
    //     if (md > 1) error(file, *ptr, ERROR1, 2);
    //     (*ptr) += 2, *(*code)++ = IF, slide(ptr, file, 0), inexpr(ptr, code, file, self), slide(ptr, file, 1);
    //     ch_start_scope(&vars);
    //     int ch = ch_code(ptr, code, file, func, md, self);
    //     ch_end_scope(&vars);
    //     if (is_else(*ptr)) {
    //         (*ptr) += 4, *(*code)++ = ELSE, slide(ptr, file, 1);
    //         if (ch && ch_code(ptr, code, file, func, md, self)) res = 1;
    //     }
    // }
    // else if (is_while(*ptr)) {
    //     if (clsmd) error(file, *ptr, ERROR1, 5);
    //     (*ptr) += 5, *(*code)++ = WHILE, slide(ptr, file, 0), inexpr(ptr, code, file, self), slide(ptr, file, 1), ch_code(ptr, code, file, 0, fnmd, 1, self);
    // }
    // else if (is_do(*ptr)) {
    //     if (clsmd) error(file, *ptr, ERROR1, 2);
    //     (*ptr) += 2, *(*code)++ = DO, slide(ptr, file, 1), ch_code(ptr, code, file, 0, fnmd, 1, self);
    //     if (is_while(*ptr)) {
    //         (*ptr) += 5, *(*code)++ = WHILE, slide(ptr, file, 0), inexpr(ptr, code, file, self);
    //         if (**ptr == ';' || **ptr == '\n' || **ptr == 0) (*ptr) ++, *(*code)++ = ';';
    //         else error(file, *ptr, "SyntaxError: Invalid syntax", 1);
    //     } else error(file, (*ptr) + 1, "SyntaxError: need 'while'", 1);
    // }
    // else if (is_for(*ptr)) {
    //     if (clsmd) error(file, *ptr, ERROR1, 3);
    //     (*ptr) += 3, *(*code)++ = FOR;
    //     SLIDE(ptr)
    //     EXPR(ptr)
    //     TERMINATION(ptr)
    //     SLIDE(ptr)
    //     EXPR(ptr)
    //     TERMINATION(ptr)
    //     SLIDE(ptr)
    //     EXPR(ptr)
    //     SLIDE_LINE(ptr)
    //     ch_code(ptr, code, file, clsmd, fnmd, lpmd);
    // }
    // else if (is_switch(*ptr)) {
    //     if (clsmd) error(file, *ptr, ERROR1, 6);
    //     (*ptr) += 6, *(*code)++ = SWITCH;
    //     SLIDE(ptr)
    //     EXPR(ptr)
    //     SLIDE_LINE(ptr)
    //     if (**ptr != '{') error(file, *ptr, ERROR2, 1);
    //     (*ptr) ++;
    //     SLIDE_LINE(ptr)
    //     while (is_case(*ptr)) {
    //         (*ptr) += 4, *(*code)++ = CASE;
    //         SLIDE(ptr)
    //         EXPR(ptr)
    //         CODE(ptr)
    //     }
    //     if (is_default(*ptr)) {
    //         (*ptr) += 7, *(*code)++ = DEFAULT, slide(ptr, file, 1), ch_code(ptr, code, file, clsmd, fnmd, lpmd, self);
    //     }
    //     if (**ptr != '}') error(file, *ptr, ERROR2, 1);
    //     (*ptr) ++, *(*code)++ = BLOCK_E;
    // }
    else if (is_break(*ptr)) {
        if (md != 1) error(file, *ptr, ERROR1, 5);
        (*ptr) += 5, *(*code)++ = BREAK;
        if (**ptr == ';' || **ptr == '\n' || **ptr == 0) (*ptr) ++, *(*code)++ = EOL;
        else error(file, *ptr, ERROR2, 1);
    }
    else if (is_continue(*ptr)) {
        if (md != 1) error(file, *ptr, ERROR1, 8);
        (*ptr) += 8, *(*code)++ = CONTINUE;
        if (**ptr == ';' || **ptr == '\n' || **ptr == 0) (*ptr) ++, *(*code)++ = ';';
        else error(file, *ptr, ERROR2, 1);
    }
    else if (is_return(*ptr)) {
        if (func == NULL) error(file, *ptr, ERROR1, 6);
        (*ptr) += 6, *(*code)++ = RETURN, slide(ptr, file, 0), res = 1;
        if (**ptr == ';' || **ptr == '\n' || **ptr == 0) {
            if (func->obj == undefined) func->obj = null;
            else if (func->obj != any && func->obj != null) error(file, *ptr, "TypeError: 戻り値の型の不一致", 1);    
        }
        else {
            char *tmp_ptr = *ptr;
            Value val = inexpr(ptr, code, file, self);
            if (func->obj == undefined) *func = val;
            else if (func->obj != any && !typecmp(*func, val)) error(file, tmp_ptr, "TypeError: 戻り値の型の不一致", (*ptr) - tmp_ptr);
            if (**ptr != ';' && **ptr != '\n' && **ptr != 0) error(file, *ptr, ERROR2, 1);
        }
        (*ptr) ++, *(*code)++ = EOL;
    }
    else if (is_else(*ptr)) error(file, *ptr, ERROR1, 4);
    else if (is_case(*ptr)) error(file, *ptr, ERROR1, 4);
    else if (is_default(*ptr)) error(file, *ptr, ERROR1, 7);
    else if (is_import(*ptr)) {
        (*ptr) += 6, slide(ptr, file, 0);
        if (**ptr == '"' || **ptr == '\'') {
            char *endptr = (*ptr) ++, path[256], *p = path;
            while (**ptr != *endptr) *p ++ = *(*ptr) ++;
            *p = 0, (*ptr) ++;
            FILE *ifp = fopen(path, "r");
            if (ifp == NULL) error(file, endptr, "file not found", (*ptr) - endptr);
            readfile(ifp, code, path, func, md, self), slide(ptr, file, 1);
        }
        else error(file, *ptr, ERROR2, 1);
    }
    else {
        if (md >= 2) error(file, *ptr, ERROR1, 1);
        inexpr(ptr, code, file, self);
        if (**ptr == ';' || **ptr == '\n' || **ptr == 0) (*ptr) ++, *(*code)++ = EOL;
        else error(file, *ptr, ERROR2, 1);
    }
    
    slide(ptr, file, 1);

    return res;
}

int main(int argc, char **argv) {

    FILE *fp = fopen(argv[1], "r");
    if (fp == NULL) {
        puts("error: file not found");
        return EXIT_FAILURE;
    }

    /**
     * function
     * (Value [5]){{null}, {&string}, {&number}, {&boolean}, {undefined}}};
     *              self    return      arg1        arg2       end
     * initializer
     * (Value [3]){{&number}, {&boolean}, {undefined}}};
     *                arg1        arg2       end
    */

    Object string_stc;
    string_stc.list = string_stc.set, string_stc.list->id = (char *)1, string_stc.list->val.data = (Value [2]){{any}, {undefined}};
    string.list = string.set;
    // string.list ++, string.list->id = "replace", string.list->val = (Value){&function, (Value [5]){{&string}, {&string}, {&string}, {&string}, {undefined}}};

    Object number_stc;
    number_stc.list = number_stc.set, number_stc.list->id = (char *)1, number_stc.list->val.data = (Value [2]){{any}, {undefined}};
    number.list = number.set;

    Object integrate_stc;
    integrate_stc.list = integrate_stc.set, integrate_stc.list->id = (char *)1, integrate_stc.list->val.data = (Value [2]){{any}, {undefined}};
    integrate_stc.list ++, integrate_stc.list->id = "random", integrate_stc.list->val = (Value){&function, (Value [5]){{&integrate_stc}, {&integrate}, {&integrate}, {&integrate}, {undefined}}};
    integrate.list = integrate.set;

    Object boolean_stc;
    boolean_stc.list = boolean_stc.set, boolean_stc.list->id = (char *)1, boolean_stc.list->val.data = (Value [2]){{any}, {undefined}};
    boolean.list = boolean.set;

    // Value val_array = {&array, "[]", NULL};
    // list ++, list->id = "Array", list->val.obj = null, list->val.data = &val_array, list->val.type = (char[1]){CLASS};

    Object function_stc; 
    function_stc.list = function_stc.set, function_stc.list->id = (char *)1, function_stc.list->val.data = (Value [1]){{undefined}};
    function.list = function.set;



    /**
     * vars
     */
    vars.list = vars.set;
    vars.list ++, vars.list->id = "String", vars.list->val.obj = &string_stc, vars.list->val.data = &string;
    vars.list ++, vars.list->id = "Number", vars.list->val.obj = &number_stc, vars.list->val.data = &number;
    vars.list ++, vars.list->id = "Int", vars.list->val.obj = &integrate_stc, vars.list->val.data = &integrate;
    vars.list ++, vars.list->id = "Boolean", vars.list->val.obj = &boolean_stc, vars.list->val.data = &boolean;
    vars.list ++, vars.list->id = "Function", vars.list->val.obj = &function_stc, vars.list->val.data = &function;
    vars.list ++, vars.list->id = "puts", vars.list->val = (Value){&function, (Value [4]){{any}, {null}, {&string}, {undefined}}};




    char *source = malloc(sizeof(char) * BUFSIZ * 4), *code = source;

    Log log[256], *logp = log;
    ch_start_scope(&vars, &logp), readfile(fp, &code, argv[1], NULL, 0, (Value){null}), ch_end_scope(&vars, log);

    // log
    // FILE *log_ = fopen("log", "wb");
    // // fprintf(log, "length: %ld\n", code - source);
    // fwrite(source, 1, code - source, log_);



    string_stc.set[0].val.data = (char [3]){'@', 1, 0};
    integrate_stc.set[0].val.data = (char [3]){'@', 2, 0};
    vars.set[6].val.data = (char [3]){'@', 0, 1};

    char md = 0;
    while (*source != 0) ex_code(&source, (Value){null}, &md);



    return EXIT_SUCCESS;
}