

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"
#include <stdio.h>

void generate_assembly(Program* program, FILE* output_file);

#endif // CODEGEN_H


