#define IDENTIFIER              '$'     // abc
#define STRING                  '"'    // "foo" 'bar'
#define INT                     '#'     // 123
#define NUMBER                  '%'     // 123.456
#define ANY                     'a'     // any
#define TRUE                    't'     // true
#define FALSE                   'f'     // false
#define NAN_                    'n'     // nan
#define INFINITY_               'i'     // infinity
#define VOID                    'v'     // void
#define SELF                    's'     // self
#define FUNCTION                '<'     // func(
#define FUNCTION_E              '>'     // ) ->

#define COMMA                   ','     // ,
#define GROUP                   '('     // (
#define ARRAY                   '['     // [
#define GROUP_E                 ')'     // )
#define ARRAY_E                 ']'     // ]

#define EOL                     '\n'     // ;
#define BLOCK                   '{'     // {
#define BLOCK_E                 '}'     // }

#define VAR                     'V'     // var
#define FUNC                    'F'     // func
#define CLASS                   'C'     // class
#define STATIC                  'T'     // static
#define INIT                    'N'     // init
#define EXTEND                  'X'     // extend
#define IF                      'I'     // if
#define ELSE                    'L'     // else
#define WHILE                   'W'     // while
#define DO                      'D'     // do
#define FOR                     'O'     // for
#define SWITCH                  'S'     // switch
#define CASE                    'A'     // case
#define DEFAULT                 'U'     // default
#define RETURN                  'R'     // return
#define BREAK                   'B'     // break
#define CONTINUE                'N'     // continue
#define GET                     'G'     // get
#define SET                     'E'     // set

#define INCREMENT_A             -64     // a ++
#define DECREMENT_A             -65     // a --
#define INCREMENT_B             -66     // ++a
#define DECREMENT_B             -67     // --a
#define NOT                     '!'     // !a
#define BITWISE_NOT             '~'     // ~a
#define PERIOD                  '.'     // .a
#define PLUS                    '+'     // +a
#define MINUS                   '-'     // -a

#define ADDITION                -72     // "+"
#define SUBTRACTION             -73     // "-"
#define EXPONENTIATION          -74     // "**"
#define MULTIPLICATION          -75     // "*"
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
#define AND                     -87     // &&
#define OR                      -88     // ||
#define BITWISE_AND             -90     // &
#define BITWISE_OR              -91     // |
#define BITWISE_XOR             -92     // ^
#define CONDITIONAL             -93     // ?
#define CONDITIONAL_ELSE        -94     // :

#define ASSIGNMENT              '='     // =
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




#define is_id(ch) ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || ch == '_')
#define isnot_id(ch) ((ch <= 'A' || ch >= 'Z') && (ch <= 'a' || ch >= 'z') && (ch <= '0' || ch >= '9') && ch != '_')
#define is_number(ch) (ch >= '0' && ch <= '9')
#define is_octadecimal(ch) (ch >= '0' && ch <= '7')
#define is_hexadecimal(ch) ((ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') || (ch >= 'a' && ch <= 'f'))
#define atox(ch) (ch - (ch <= '9' ? 48 : ch <= 'A' ? 55 : 87))
#define is_operator(ch) (ch <= -60)

#define is_any(str) (str)[0] == 'a' && (str)[1] == 'n' && (str)[2] == 'y' && isnot_id((str)[3])
#define is_true(str) ((str)[0] == 't' && (str)[1] == 'r' && (str)[2] == 'u' && (str)[3] == 'e' && isnot_id((str)[4]))
#define is_false(str) ((str)[0] == 'f' && (str)[1] == 'a' && (str)[2] == 'l' && (str)[3] == 's' && (str)[4] == 'e' && isnot_id((str)[5]))
#define is_void(str) ((str)[0] == 'v' && (str)[1] == 'o' && (str)[2] == 'i' && (str)[3] == 'd' && isnot_id((str)[4]))
#define is_self(str) ((str)[0] == 's' && (str)[1] == 'e' && (str)[2] == 'l' && (str)[3] == 'f' && isnot_id((str)[4]))
#define is_nan(str) (str)[0] == 'n' && (str)[1] == 'a' && (str)[2] == 'n' && isnot_id((str)[3])
#define is_infinity(str) ((str)[0] == 'i' && (str)[1] == 'n' && (str)[2] == 'f' && (str)[3] == 'i' && (str)[4] == 'n' && (str)[5] == 'i' && (str)[6] == 't' && (str)[7] == 'y' && isnot_id((str)[8]))

#define is_if(str) (str)[0] == 'i' && (str)[1] == 'f' && isnot_id((str)[2])
#define is_else(str) ((str)[0] == 'e' && (str)[1] == 'l' && (str)[2] == 's' && (str)[3] == 'e' && isnot_id((str)[4]))
#define is_while(str) ((str)[0] == 'w' && (str)[1] == 'h' && (str)[2] == 'i' && (str)[3] == 'l' && (str)[4] == 'e' && isnot_id((str)[5]))
#define is_do(str) (str)[0] == 'd' && (str)[1] == 'o' && isnot_id((str)[2])
#define is_for(str) (str)[0] == 'f' && (str)[1] == 'o' && (str)[2] == 'r' && isnot_id((str)[3])
#define is_switch(str) ((str)[0] == 's' && (str)[1] == 'w' && (str)[2] == 'i' && (str)[3] == 't' && (str)[4] == 'c' && (str)[5] == 'h' && isnot_id((str)[6]))
#define is_case(str) ((str)[0] == 'c' && (str)[1] == 'a' && (str)[2] == 's' && (str)[3] == 'e' && isnot_id((str)[4]))
#define is_default(str) ((str)[0] == 'd' && (str)[1] == 'e' && (str)[2] == 'f' && (str)[3] == 'a' && (str)[4] == 'u' && (str)[5] == 'l' && (str)[6] == 't' && isnot_id((str)[7]))
#define is_return(str) ((str)[0] == 'r' && (str)[1] == 'e' && (str)[2] == 't' && (str)[3] == 'u' && (str)[4] == 'r' && (str)[5] == 'n' && isnot_id((str)[6]))
#define is_break(str) ((str)[0] == 'b' && (str)[1] == 'r' && (str)[2] == 'e' && (str)[3] == 'a' && (str)[4] == 'k' && isnot_id((str)[5]))
#define is_continue(str) ((str)[0] == 'c' && (str)[1] == 'o' && (str)[2] == 'n' && (str)[3] == 't' && (str)[4] == 'i' && (str)[5] == 'n' && (str)[6] == 'u' && (str)[7] == 'e' && isnot_id((str)[8]))
#define is_get(str) (str)[0] == 'g' && (str)[1] == 'e' && (str)[2] == 't' && isnot_id((str)[3])
#define is_set(str) (str)[0] == 's' && (str)[1] == 'e' && (str)[2] == 't' && isnot_id((str)[3])

#define is_import(str) ((str)[0] == 'i' && (str)[1] == 'm' && (str)[2] == 'p' && (str)[3] == 'o' && (str)[4] == 'r' && (str)[5] == 't' && isnot_id((str)[6]))
#define is_var(str) (str)[0] == 'v' && (str)[1] == 'a' && (str)[2] == 'r' && isnot_id((str)[3])
#define is_class(str) ((str)[0] == 'c' && (str)[1] == 'l' && (str)[2] == 'a' && (str)[3] == 's' && (str)[4] == 's' && isnot_id((str)[5]))
#define is_func(str) ((str)[0] == 'f' && (str)[1] == 'u' && (str)[2] == 'n' && (str)[3] == 'c' && isnot_id((str)[4]))
#define is_init(str) ((str)[0] == 'i' && (str)[1] == 'n' && (str)[2] == 'i' && (str)[3] == 't' && isnot_id((str)[4]))
#define is_static(str) ((str)[0] == 's' && (str)[1] == 't' && (str)[2] == 'a' && (str)[3] == 't' && (str)[4] == 'i' && (str)[5] == 'c' && isnot_id((str)[6]))
#define is_extend(str) ((str)[0] == 'e' && (str)[1] == 'x' && (str)[2] == 't' && (str)[3] == 'e' && (str)[4] == 'n' && (str)[5] == 'd' && isnot_id((str)[6]))

#define ERROR1 "SyntaxError: Invalid syntax"
#define ERROR2 "SyntaxError: Unexpected token"

#define ERROR3 "SyntaxError: Invalid left-hand side expression in assignment"
#define ERROR4 "SyntaxError: Missing operator"
#define ERROR5 "SyntaxError: Invalid left-hand side expression in prefix operation"
#define ERROR6 "SyntaxError: Invalid identifier"
#define ERROR7 "SyntaxError: Invalid left-hand side expression in prefix operation"
