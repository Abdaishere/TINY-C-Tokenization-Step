#include <cstdio>
#include <cstring>
#include <iostream>

using namespace std;


/*{ Sample program
  in TINY language
  compute factorial
}

read x; {input an integer}
if 0<x then {compute only if x>=1}
  fact:=1;
  repeat
    fact := fact * x;
    x:=x-1
  until x=0;
  write fact {output factorial}
end
*/

// sequence of statements separated by ;
// no procedures - no declarations
// all variables are integers
// variables are declared simply by assigning values to them :=
// variable names can include only alphabetic characters (a:z or A:Z) and underscores
// if-statement: if (boolean) then [else] end
// repeat-statement: repeat until (boolean)
// boolean only in if and repeat conditions < = and two mathematical expressions.
// math expressions integers only, + - * / ^
// I/O read write
// Comments {}

////////////////////////////////////////////////////////////////////////////////////
// Strings /////////////////////////////////////////////////////////////////////////

bool Equals(const char *a, const char *b) {
    return strcmp(a, b) == 0;
}

bool StartsWith(const char *a, const char *b) {
    int nb = strlen(b);
    return strncmp(a, b, nb) == 0;
}

void Copy(char *a, const char *b, int n = 0) {
    if (n > 0) {
        strncpy(a, b, n);
        a[n] = 0;
    } else strcpy(a, b);
}

void AllocateAndCopy(char **a, const char *b) {
    if (b == 0) {
        *a = 0;
        return;
    }
    int n = strlen(b);
    *a = new char[n + 1];
    strcpy(*a, b);
}

////////////////////////////////////////////////////////////////////////////////////
// Input and Output ////////////////////////////////////////////////////////////////

#define MAX_LINE_LENGTH 10000

struct InFile {
    FILE *file;
    int cur_line_num;

    char line_buf[MAX_LINE_LENGTH];
    int cur_ind, cur_line_size;

    InFile(const char *str) {
        file = 0;
        if (str) file = fopen(str, "r");
        cur_line_size = 0;
        cur_ind = 0;
        cur_line_num = 0;
    }

    ~InFile() { if (file) fclose(file); }

    void SkipSpaces() {
        while (cur_ind < cur_line_size) {
            char ch = line_buf[cur_ind];
            if (ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n') break;
            cur_ind++;
        }
    }

    bool SkipUpto(const char *str) {
        while (true) {
            SkipSpaces();
            while (cur_ind >= cur_line_size) {
                if (!GetNewLine()) return false;
                SkipSpaces();
            }

            if (StartsWith(&line_buf[cur_ind], str)) {
                cur_ind += strlen(str);
                return true;
            }
            cur_ind++;
        }
        return false;
    }

    bool GetNewLine() {
        cur_ind = 0;
        line_buf[0] = 0;
        if (!fgets(line_buf, MAX_LINE_LENGTH, file)) return false;
        cur_line_size = strlen(line_buf);
        if (cur_line_size == 0) return false; // End of file
        cur_line_num++;
        return true;
    }

    char *GetNextTokenStr() {
        SkipSpaces();
        while (cur_ind >= cur_line_size) {
            if (!GetNewLine()) return 0;
            SkipSpaces();
        }
        return &line_buf[cur_ind];
    }

    void Advance(int num) {
        cur_ind += num;
    }
};

struct OutFile {
    FILE *file;

    OutFile(const char *str) {
        file = 0;
        if (str) file = fopen(str, "w");
    }

    ~OutFile() { if (file) fclose(file); }

    void Out(const char *s) {
        fprintf(file, "%s\n", s);
        fflush(file);
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Compiler Parameters /////////////////////////////////////////////////////////////

struct CompilerInfo {
    InFile in_file;
    OutFile out_file;
    OutFile debug_file;

    CompilerInfo(const char *in_str, const char *out_str, const char *debug_str)
            : in_file(in_str), out_file(out_str), debug_file(debug_str) {
    }
};

////////////////////////////////////////////////////////////////////////////////////
// Scanner /////////////////////////////////////////////////////////////////////////

#define MAX_TOKEN_LEN 40

enum TokenType {
    IF, THEN, ELSE, END, REPEAT, UNTIL, READ, WRITE,
    ASSIGN, EQUAL, LESS_THAN,
    PLUS, MINUS, TIMES, DIVIDE, POWER,
    SEMI_COLON,
    LEFT_PAREN, RIGHT_PAREN,
    LEFT_BRACE, RIGHT_BRACE,
    ID, NUM,
    ENDFILE, ERROR
};

// Used for debugging only /////////////////////////////////////////////////////////
const char *TokenTypeStr[] =
        {
                "If", "Then", "Else", "End", "Repeat", "Until", "Read", "Write",
                "Assign", "Equal", "LessThan",
                "Plus", "Minus", "Times", "Divide", "Power",
                "SemiColon",
                "LeftParen", "RightParen",
                "LeftBrace", "RightBrace",
                "ID", "Num",
                "EndFile", "Error"
        };

struct Token {
    TokenType type;
    char str[MAX_TOKEN_LEN + 1];

    Token() {
        str[0] = 0;
        type = ERROR;
    }

    Token(TokenType _type, const char *_str) {
        type = _type;
        Copy(str, _str);
    }
};

const Token reserved_words[] =
        {
                Token(IF, "if"),
                Token(THEN, "then"),
                Token(ELSE, "else"),
                Token(END, "end"),
                Token(REPEAT, "repeat"),
                Token(UNTIL, "until"),
                Token(READ, "read"),
                Token(WRITE, "write")
        };
const int num_reserved_words = sizeof(reserved_words) / sizeof(reserved_words[0]);

// if there is tokens like < <=, sort them such that sub-tokens come last: <= <
// the closing comment should come immediately after opening comment
const Token symbolic_tokens[] =
        {
                Token(ASSIGN, ":="),
                Token(EQUAL, "="),
                Token(LESS_THAN, "<"),
                Token(PLUS, "+"),
                Token(MINUS, "-"),
                Token(TIMES, "*"),
                Token(DIVIDE, "/"),
                Token(POWER, "^"),
                Token(SEMI_COLON, ";"),
                Token(LEFT_PAREN, "("),
                Token(RIGHT_PAREN, ")"),
                Token(LEFT_BRACE, "{"),
                Token(RIGHT_BRACE, "}")
        };
const int num_symbolic_tokens = sizeof(symbolic_tokens) / sizeof(symbolic_tokens[0]);

inline bool IsDigit(char ch) { return (ch >= '0' && ch <= '9'); }

inline bool IsLetter(char ch) { return ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z')); }

inline bool IsLetterOrUnderscore(char ch) { return (IsLetter(ch) || ch == '_'); }

inline char *getStr(int &s, int &e, char array[]) {
    char *temp = new char[e - s - 1];
    for (int i = s; i < e; i++) temp[i - s] = array[i];
    temp[e - s] = 0;
//    printf("%d : %d in line %s the word %s size %d\n", s, e - 1, array, temp, strlen(temp));
    return temp;
}

inline int isSymbolicToken(const char *str) {
    for (int i = 0; i < num_symbolic_tokens; i++) {
        if (Equals(str, symbolic_tokens[i].str)) {
            return i;
        }
    }
    return -1;
}

inline int isReservedWord(const char *str) {
    for (int i = 0; i < num_reserved_words; i++) {
        if (Equals(str, reserved_words[i].str)) {
            return i;
        }
    }
    return -1;
}

inline void printToken(int line, char *str, int tokenType, OutFile &out_file) {
    char res[1000] = "";
    sprintf(res, "[%d] %s (%s)", line, str, TokenTypeStr[tokenType]);
    out_file.Out(res);
}

enum status {
    null, letter, LetterOrUnderscore, digit, symbol, comment
};

int main() {
    CompilerInfo compiler("input.txt", "output.txt", "debug.txt");

    // index of cur line
    int *end = &compiler.in_file.cur_ind;

    status was = null;

    while (true) {
        compiler.in_file.GetNextTokenStr();
        int start = *end;

        while (*end < compiler.in_file.cur_line_size) {
            char ch = compiler.in_file.line_buf[*end];

//            printf("%c\n", compiler.in_file.line_buf[*end]);

            // is letter
            if (IsLetter(ch) && (was == null || was == letter)) {
                (*end)++;
                was = letter;
                continue;
            }

            // letter token
            if ((ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == 0 ||
                 *end >= compiler.in_file.cur_line_size) && was == letter) {
                was = null;
                short match = isReservedWord(getStr(start, *end, compiler.in_file.line_buf));

                if (match >= 0) {

                    printToken(compiler.in_file.cur_line_num, getStr(start, *end, compiler.in_file.line_buf),
                               reserved_words[match].type, compiler.out_file);
                } else {
                    printToken(compiler.in_file.cur_line_num, getStr(start, *end, compiler.in_file.line_buf), 21,
                               compiler.out_file);
                }

                compiler.in_file.SkipSpaces();
                start = *end;
                continue;
            }

            if (IsLetterOrUnderscore(ch) &&
                (was == letter || was == LetterOrUnderscore || was == null)) {
                was = LetterOrUnderscore;
                (*end)++;
                continue;
            }

            // all IDs (letter or underscore but not reserved) tokens
            if (*end > start && (was == LetterOrUnderscore || was == letter)) {
                was = null;

                printToken(compiler.in_file.cur_line_num, getStr(start, *end, compiler.in_file.line_buf), 21,
                           compiler.out_file);

                compiler.in_file.SkipSpaces();
                start = *end;
                continue;
            }

            // is digit
            if (IsDigit(ch) && (was == digit || was == null)) {
                was = digit;
                (*end)++;
                continue;
            }

            // num token
            if (*end > start && was == digit) {
                was = null;

                printToken(compiler.in_file.cur_line_num, getStr(start, *end, compiler.in_file.line_buf), 22,
                           compiler.out_file);

                compiler.in_file.SkipSpaces();
                start = *end;
                continue;
            }


            // is a symbol
            if (!IsDigit(ch) && !IsLetterOrUnderscore(ch) && ch != ' ' && ch != '\t' && ch != '\r' && ch != '\n' &&
                *end < compiler.in_file.cur_line_size &&
                (was == null || was == symbol)) {
                was = symbol;
                (*end)++;
                continue;
            }

            // Symbol token
            if (*end > start && was == symbol) {
                was = null;

                short match = isSymbolicToken(getStr(start, *end, compiler.in_file.line_buf));

                if (match >= 0) {
                    if (match == 11)
                        was = comment;

                    printToken(compiler.in_file.cur_line_num, getStr(start, *end, compiler.in_file.line_buf),
                               symbolic_tokens[match].type, compiler.out_file);

                    compiler.in_file.SkipSpaces();
                    start = *end;
                    continue;
                }
            }

            // close comment
            if (was == comment && ch == '}') {
                was = null;
                printToken(compiler.in_file.cur_line_num, "}", symbolic_tokens[12].type, compiler.out_file);

            }

            compiler.in_file.SkipSpaces();
            compiler.in_file.Advance(1);
            start = *end;
        }
        if (compiler.in_file.GetNewLine() == 0) break;
    }
}
