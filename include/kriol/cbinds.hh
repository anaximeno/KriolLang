#ifndef _KRIOL_CBINDS_HEADER
#define _KRIOL_CBINDS_HEADER

#include <string>

#define _(cstr) std::string(cstr)

#define C_EQ _("=")

#define C_SEMICOLON _(";")

#define C_COMMA _(",")

#define C_OPEN_CURLY _("{")

#define C_CLOSE_CURLY _("}")

#define C_OPEN_PAR _("(")

#define C_CLOSE_PAR _(")")

#define C_IF _("if")

#define C_ELSE _("else")

#define C_WHILE _("while")

#define C_FOR _("for")

#define C_RETURN _("return")

#define C_IMPORT _("#include")

#define C_PRINTF _("printf")

#endif // _KRIOL_CBINDS_HEADER