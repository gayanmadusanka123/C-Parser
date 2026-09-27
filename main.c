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

void print_token(Token token){
	printf("TOKEN VALUE : ");
	if(token.value != NULL){
		for (int i = 0; token.value[i] != '\0' ; i++){
			printf("%c", token.value[i]);
		}
	}
	
	
	switch(token.type){
		case INT:
			printf("  TOKEN TYPE : INT\n");break;
		case KEYWORD:
			printf("  TOKEN TYPE : KEYWORD\n");break;
		case SEPARATOR:
			printf("  TOKEN TYPE : SEPARATOR\n");break;
		case OPERATOR:
			printf("  TOKEN TYPE : OPERATOR\n");break;
		case IDENTIFIER:
			printf("  TOKEN TYPE : IDENTIFIER\n");break;
		case END_OF_TOKENS:
			printf("  TOKEN TYPE : SEPARATOR\n");break;
	}
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
	
	token->value = keyword;  // Set default value first
	
	if(strcmp(keyword, "exit") == 0 || strcmp(keyword, "int") == 0 || strcmp(keyword, "print") == 0){
	}else{
		token->type = IDENTIFIER;
	}
	
	token->type = IDENTIFIER;
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
			
		// Handle operators
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
typedef enum{
	INT_LIT,
	STATEMENT,
	EXTRAS,
	BEGINING,
}NodeTypes;

typedef struct Node{
	char *value;
	NodeTypes type;
	struct Node *right;
	struct Node *left;
}Node;

void print_tree(Node *node){
	if(node == NULL){
		return;
	}

	for(size_t i = 0; node->value[i] != '\0'; i++){
		printf("%c", node->value[i]);
	}
	printf("\n");
	
	print_tree(node->left);
	print_tree(node->right);
	
}

Node *init_node(char *value, NodeTypes type){
	Node *node= malloc(sizeof(Node));
	node->type = type;
	node->value = value;
	node->left = NULL;
	node->right = NULL;
	return node;
}

Token *parser(Token *tokens){
	Token *current_token = &tokens[0];
	
	Node *root = init_node("PROGRAM", BEGINING);
			
	Node *current = root;
		
	while(current_token->type != END_OF_TOKENS){
		if(root == NULL){
			break;
		}
		
		if(current_token->type == KEYWORD && strcmp(current_token->value,"exit") == 0){
			Node *exit_node = init_node(current_token->value, STATEMENT);
			root->right = exit_node;
			current = exit_node;
			current_token++;
			if(current_token->type != SEPARATOR){
				printf("ERROR\n");
				exit(1);
			}
			Node *open_paran_node = init_node(current_token->value, EXTRAS);
			current->left = open_paran_node;
			
			current_token++;
			
			Node *expr_node = init_node(current_token->value, INT_LIT);
			current->left->left = expr_node;
			
			current_token++;
			
			Node *close_paran_node = init_node(current_token->value, EXTRAS);
			current->left->right = close_paran_node;
			
			current_token++;
			
			Node *semi_node = init_node(current_token->value, EXTRAS);
			current->right = semi_node;
			
			print_tree(root);
			break;
		}
		
		print_tree(root);
		current_token++;
	}
	return current_token;
}

int main(){
	
	FILE *file;
	file = fopen("test.unn", "r");
	
	Token *tokens = lexer(file);
	
	for(size_t i = 0; tokens[i].type != END_OF_TOKENS; i++){
		print_token(tokens[i]);
	}
	
	parser(tokens);
	
	return 0;
}

