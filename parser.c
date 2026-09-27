#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

typedef enum{
	INT,
	KEYWORD,
	SEPARATOR,
	OPERATOR,
	IDENTIFIER,
	END_OF_TOKENS,
}TokenType;

typedef struct{
	TokenType type;
	char *value;
}Token;

typedef struct{
	char *name;
	int value;
	int initialized;
}Variable;

Variable variables[100];
int var_count = 0;

int variable_exists(char *name){
	for(int i = 0; i < var_count; i++){
		if(strcmp(variables[i].name, name) == 0){
			return 1;
		}
	}
	return 0;
}

void set_variable(char *name, int value){
	// CHANGE: Check for redeclaration and exit with error
	for(int i = 0; i < var_count; i++){
		if(strcmp(variables[i].name, name) == 0){
			printf("SEMANTIC ERROR: Variable '%s' is already declared\n", name);
			exit(1);  
		}
	}
	
	variables[var_count].name = malloc(strlen(name) + 1);
	strcpy(variables[var_count].name, name);
	variables[var_count].value = value;
	variables[var_count].initialized = 1;  
	var_count++;
}

int get_variable(char *name){
	for(int i = 0; i < var_count; i++){
		if(strcmp(variables[i].name, name) == 0){
			if(!variables[i].initialized){  // ADD THIS CHECK
				printf("SEMANTIC ERROR: Variable '%s' is used before initialization\n", name);
				exit(1);
			}
			return variables[i].value;
		}
	}
	printf("SEMANTIC ERROR: Variable '%s' is not declared\n", name); 
	exit(1);
}

void print_token(Token token){
	//printf("TOKEN VALUE : ");
	if(token.value != NULL){
		for (int i = 0; token.value[i] != '\0' ; i++){
			//printf("%c", token.value[i]);
		}
	}
	
	/*switch(token.type){
		case INT:
			//printf("  TOKEN TYPE : INT\n");break;
		case KEYWORD:
			//printf("  TOKEN TYPE : KEYWORD\n");break;
		case SEPARATOR:
			//printf("  TOKEN TYPE : SEPARATOR\n");break;
		case OPERATOR:
			//printf("  TOKEN TYPE : OPERATOR\n");break;
		case IDENTIFIER:
			//printf("  TOKEN TYPE : IDENTIFIER\n");break;
		case END_OF_TOKENS:
			//printf("  TOKEN TYPE : END_OF_TOKENS\n");break;
	}*/
}

Token *generate_operator(char current, FILE *file){
	Token *token = malloc(sizeof(Token));
	token->value = malloc(sizeof(char) * 2);
	token->value[0] = current;
	token->value[1] = '\0';
	token->type = OPERATOR;
	return token;
}

Token *generate_number(char current, FILE *file){
	
	Token *token = malloc(sizeof(Token));
	token->type = INT;
	char *value = malloc(sizeof(char) * 8);
	int value_index = 0; 
	
	while(isdigit(current) && current != EOF){
		value[value_index] = current;
		value_index++;
		
		current = fgetc(file);
	}
	value[value_index] = '\0';
	ungetc(current, file);
	token->value = value;
	return token;
}

Token *generate_keyword(char current, FILE *file){
	Token *token = malloc(sizeof(Token));
	char *keyword = malloc(sizeof(char) * 32);
	int keyword_index = 0;
	
	while((isalpha(current) || isdigit(current) || current == '_') && current != EOF){
		keyword[keyword_index] = current;
		current = fgetc(file);
		keyword_index++;
	}
	
	keyword[keyword_index] = '\0';
	
	token->value = keyword;
	
	if(strcmp(keyword, "exit") == 0 || strcmp(keyword, "int") == 0 || strcmp(keyword, "print") == 0){
		token->type = KEYWORD;
	}else{
		token->type = IDENTIFIER;
	}
	
	ungetc(current, file);
	
	return token;
}

Token *generate_separator(char current, FILE *file){
	Token *token = malloc(sizeof(Token));
	token->value = malloc(sizeof(char) * 2);
	token->value[0] = current;
	token->value[1] = '\0';
	token->type = SEPARATOR;
	return token;
}

size_t tokens_index;

Token *lexer(FILE *file){
	char current = fgetc(file);
	
	Token *tokens = malloc(sizeof(Token) * 100);
	
	tokens_index = 0;
	
	while(current != EOF){
		
		if(isspace(current)){
			current = fgetc(file);
			continue;
		}
		
		if(strchr("();", current) != NULL){
			Token *token = generate_separator(current, file);
			tokens[tokens_index] = *token;
			tokens_index++;
			free(token);
			
		}else if(strchr("=+-*/%", current) != NULL){
			Token *token = generate_operator(current, file);
			tokens[tokens_index] = *token;
			tokens_index++;
			free(token);	
		}else if(isdigit(current)){
			Token *token = generate_number(current, file);
			tokens[tokens_index] = *token;
			tokens_index++;			 
			free(token);
			
		}else if(isalpha(current)){
			Token *token = generate_keyword(current, file);
			tokens[tokens_index] = *token;
			tokens_index++;
			free(token);
		}
		
		current = fgetc(file);
	}
	
	tokens[tokens_index].value = NULL;
	tokens[tokens_index].type = END_OF_TOKENS;
	return tokens;
}

////////////////////////////parser////////////////////////////////

int evaluate_expression(Token **current_token){
	int result = 0;
	char op = '\0';
	int expecting_operator = 0; 
	
	while((*current_token)->type != SEPARATOR || (*current_token)->value[0] != ';'){
		if((*current_token)->type == END_OF_TOKENS){
			printf("ERROR: Expected ';' at end of statement\n");
			exit(1);
		}
		
		if((*current_token)->type == INT){
			if(expecting_operator){
				printf("ERROR: Unexpected number '%s' \n", (*current_token)->value);
				exit(1);
			}
			int value = atoi((*current_token)->value);
			if(op == '\0') {
			result = value;
			}
			else if(op == '+') {
				result += value;
			}
			else if(op == '-') {
				result -= value;
			}
			else if(op == '*') {
				result *= value;
			}
			else if(op == '/') {
				if(value == 0){  // ADD THIS CHECK
					printf("SEMANTIC ERROR: Division by zero\n");
					exit(1);
				}
				result /= value;
			}
			else if(op == '%') {
				if(value == 0){  // ADD THIS CHECK
					printf("SEMANTIC ERROR: Modulo by zero\n");
					exit(1);
				}
				result %= value;
			}
			expecting_operator = 1;  // After a number, we expect an operator
			
		} else if((*current_token)->type == IDENTIFIER){
			if(expecting_operator){
				printf("ERROR: Unexpected identifier '%s' \n", (*current_token)->value);
				exit(1);
			}
			int value = get_variable((*current_token)->value);

			if(op == '\0') {
				result = value;
			}
			else if(op == '+') {
				result += value;
			}
			else if(op == '-') {
				result -= value;
			}
			else if(op == '*') {
				result *= value;
			}
			else if(op == '/') {
				if(value == 0){  // ADD THIS CHECK
					printf("SEMANTIC ERROR: Division by zero (variable value is 0)\n");
					exit(1);
				}
				result /= value;
			}
			else if(op == '%') {
				if(value == 0){  // ADD THIS CHECK
					printf("SEMANTIC ERROR: Modulo by zero (variable value is 0)\n");
					exit(1);
				}
				result %= value;
			}
			expecting_operator = 1;  // After an identifier, we expect an operator
			
		} else if((*current_token)->type == OPERATOR){
			if(!expecting_operator && op != '\0'){
				printf("SYNTAX ERROR: Unexpected operator '%s' missing operand?\n", (*current_token)->value);
				exit(1);
			}
			op = (*current_token)->value[0];
			expecting_operator = 0;  // After operator, we expect a number/identifier
			
		} else if((*current_token)->type == KEYWORD){
			printf("SYNTAX ERROR: Expected ';' before '%s'\n", (*current_token)->value);
			exit(1);
		} else {
			printf("SYNTAX ERROR: Unexpected token '%s' in expression\n", 
			       (*current_token)->value ? (*current_token)->value : "unknown");
			exit(1);
		}
		(*current_token)++;
	}
	
	if(!expecting_operator){
		printf("SYNTAX ERROR: Expression ends with operator missing operand\n");
		exit(1);
	}
	
	return result;
}

Token *parser(Token *tokens){
	Token *current_token = &tokens[0];
	
	while(current_token->type != END_OF_TOKENS){
		
		if(current_token->type == KEYWORD && strcmp(current_token->value, "int") == 0){
			current_token++;
			
			if(current_token->type != IDENTIFIER){
				printf("SYNTAX ERROR: Expected variable name after 'int'\n");
				exit(1);
			}
			char *var_name = current_token->value;
			current_token++;
			
			if(current_token->type != OPERATOR || current_token->value[0] != '='){
				printf("SYNTAX ERROR: Expected '=' after variable name\n");
				exit(1);
			}
			current_token++;
			
			Token *expr_start = current_token;
			
			while(current_token->type != SEPARATOR || current_token->value[0] != ';'){
				if(current_token->type == END_OF_TOKENS){
					printf("SYNTAX ERROR: Expected ';' at end of statement\n");
					exit(1);
				}
				current_token++;
			}
			
			current_token = expr_start;
			int result = evaluate_expression(&current_token);
			set_variable(var_name, result);
			
			if(current_token->type != SEPARATOR || current_token->value[0] != ';'){
				printf("ERROR: Expected ';' at end of statement\n");
				exit(1);
			}
			current_token++;
		}
		else if(current_token->type == KEYWORD && strcmp(current_token->value, "print") == 0){
			current_token++;
			
			if(current_token->type != SEPARATOR || current_token->value[0] != '('){
				printf("ERROR: Expected '(' after 'print'\n");
				exit(1);
			}
			current_token++;
			
			if(current_token->type != IDENTIFIER && current_token->type != INT){
				printf("ERROR: Expected variable or number in print statement\n");
				exit(1);
			}
			
			
			int value;
			if(current_token->type == IDENTIFIER){
				value = get_variable(current_token->value);
			} else {
				value = atoi(current_token->value);
			}

			current_token++;

			if(current_token->type != SEPARATOR || current_token->value[0] != ')'){
				printf("ERROR: Expected ')' after print argument\n");
				exit(1);
			}
			current_token++;

			if(current_token->type != SEPARATOR || current_token->value[0] != ';'){
				printf("ERROR: Expected ';' after print statement\n");
				exit(1);
			}
			current_token++;

			printf("%d\n", value);
		}
		else {
			printf("ERROR: Unexpected token '%s'\n", 
			       current_token->value ? current_token->value : "NULL");
			exit(1);
		}
	}
	
	return current_token;
}

int main(int argc, char *argv[]){
	
	if(argc != 2){
		printf("Usage: %s <filename>\n", argv[0]);
		printf("Example: %s test.unn\n", argv[0]);
		return 1;
	}
	
	FILE *file;
	file = fopen(argv[1], "r");
	
	if(file == NULL){
		printf("ERROR: Cannot open file '%s'\n", argv[1]);
		return 1;
	}
	
	Token *tokens = lexer(file);
	fclose(file);
	
	for(size_t i = 0; tokens[i].type != END_OF_TOKENS; i++){
		print_token(tokens[i]);
	}
	
	parser(tokens);
	
	
	return 0;
}
