#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
/**
 * Brain fuck consists of + - < > . , [ and ] operators
 *
 *  + increments the value at the pointer
 *  - decrements the value of the pointer
 *  < decrements the pointer
 *  > increments the pointer
 *  . outputs the values of the pointer as an ascii character
 *  , takes keyboard input and stores the ascii code at the data pointer
 *  [ checks if the data pointer is 0 and if so jumps forward to the closing ] else it loops
 *  ] ends the loop
 *
 *  the goal of this is to translate .bf files to c files and then compiles them to an executable
 */

/**
 * This array holds every recognized operand
 */
const char operands[] = {'+', '-', '<', '>', '.', ',', '[', ']'};

/**
 * @details loads a file to memory
 * @param filePath the path to the file
 * @return the contents of the file passed - null if file is not found
 */
char *readFile(const char *filePath) {
  FILE *fp;
  int sz;
  char *data;

  /* Open file */
  fp = fopen(filePath, "rb");
  if(!fp) {
    printf("Unable to open file\n");
    return NULL;
  }

  /* Get file size */
  fseek(fp, 0, SEEK_END); /* <--- causes seg fault rn */
  sz = (int)ftell(fp);
  rewind(fp);

  /* Load file content into memory, null terminate */
  data = malloc(sz + 1);
  fread(data, 1, sz, fp);

  fclose(fp);
  return data;
}

/**
 * @details modifies the data string passed to remove non brain fuck recognized characters
 * @param data the brain fuck source code
 */
void refine(char *data) {
  bool found;
  int i = 0;

  while(i < (int)strlen(data)) {
    found = false;

    for (int j = 0; j < 8; ++j) {
      if(data[i] == operands[j]) {
        found = true;
        i++;
        break;
      }
    }

    if(!found)
      memmove(&data[i], &data[i + 1], strlen(data) - i); /* delete the character and shift the rest down. */
  }
}

/**
 * @details emits c source code to the file pointer passed
 * @param fp pointer to the file to be written to
 * @param data actual c code to be written to the passed file
 */
void emit(FILE *fp, const char *data) {
  if(!fp)
    return;

  fwrite(data, 1, strlen(data), fp);
}

/**
 * @details adds the essential program boilerplate to the generated c code
 * @param fp file pointer to file to write to
 * @return returns true if successful
 */
void emitProgramStart(FILE *fp) {
  const char *programStart = "#include <stdio.h>\n"
                             "char data[1024] = {0};\n"
                             "char *ptr = data;\n"
                             "int main() {\n";

  if(!fp)
    return;

  emit(fp, programStart);
}

/**
 * @details adds the last part of boilerplate at the end of the file
 * @param fp file pointer to a file to write to
 * @return true if successful
 */
void emitProgramEnd(FILE *fp) {
  const char *programEnd = "return 0;\n"
                           "}\n";
  if(!fp)
    return;

  emit(fp, programEnd);
}

/**
 * @details goes through the brainfuck source and generates c source code
 * @param data the brainfuck source code
 * @param fp file pointer to the c file to be written to
 */
void generateSourceCode(char *data, FILE *fp) {
  for (int i = 0; i < (int)strlen(data); ++i) {
    switch (data[i]) {
      case (int)'+': /* increment pointer value */
        emit(fp, "++*ptr;\n");
        break;
      case (int)'-': /* decrement pointer data */
        emit(fp, "--*ptr;\n");
        break;
      case (int)'<': /* decrement pointer index */
        emit(fp, "--ptr;\n");
        break;
      case (int)'>': /* increment pointer index */
        emit(fp, "++ptr;\n");
        break;
      case (int)'.': /* output character at data pointer */
        emit(fp, "putchar(*ptr);\n");
        break;
      case (int)',': /* store keyboard input */
        emit(fp, "*ptr=getchar();;\n");
        break;
      case (int)'[': /* start loop if data pointer isn't 0 */
        emit(fp, "while(*ptr) {\n");
        break;
      case (int)']': /* end of loop */
        emit(fp, "}\n");
        break;
    }
  }
}

/**
 * @details generates the full c source file
 * @param fp pointer to the file to be written
 * @param data minified brainfuck source to be used
 */
void convertSource(FILE *fp, char *data) {
  /* add boilerplate to temp file */
  emitProgramStart(fp);

  /* read data and create source code */
  printf("Converting to C code\n");
  generateSourceCode(data, fp);

  /*end the source code*/
  emitProgramEnd(fp);
}

/**
 * @details parses the command line arguments for the input file
 * @param argv the argv parameter passed to the main function
 * @return NULL if no argument, the filename if it's passed
 */
char *parseArguments(char **argv) {
  if(argv[1] == NULL) {
    printf("Usage:\n\t\tbf <file to be compiled>\n");

    return NULL;
  }

  return argv[1];
}

int main(int argc, char **argv) {
  FILE *fp;
  char *data;
  char *sourceName;

  /* check arguments */
  if(!(sourceName = parseArguments(argv))) {
    goto exit;
  }

  /* init variables */
  data = readFile(sourceName);
  fp = fopen(strcat(sourceName, ".c"), "w+");

  /* make sure there is data to be read */
  if(!data) {
    printf("Error - Exiting.");
    goto exit;
  }

  /* setup the data */
  printf("Refining source code\n");
  refine(data);

  /* create c source file */
  convertSource(fp, data);

  printf("source compiled to: %s\n", sourceName);

  exit:
  /* clean up */
  fclose(fp);
  return 0;
}