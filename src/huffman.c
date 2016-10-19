#include "huffman.h"
#include "bit_manager.h"
#include "fila.h"
#include "testes.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

// Ordena o vetor de 'huffPair' com base na frequencias dos simbolos
huffPair* ordena_hvetor(huffPair *hvet, int size, int nsym){

	int i, j, k;
	double aux_f;
	short aux_s;
	huffPair *ordered = calloc(size,sizeof(huffPair));	

	k = 0;
	
	for(i=0; i<size; i++){
		if(hvet[i].frequency != 0){
			ordered[k].symbol = hvet[i].symbol;
			ordered[k].frequency = hvet[i].frequency;
			k++;
		}
	}	

	for(i=k-1; i>=1; i--){
		for(j=0; j<i; j++){
			if(ordered[j].frequency >= ordered[j+1].frequency){
				aux_f = ordered[j].frequency;
				aux_s = ordered[j].symbol;
				ordered[j].frequency = ordered[j+1].frequency;
				ordered[j].symbol = ordered[j+1].symbol;
				ordered[j+1].frequency = aux_f;
				ordered[j+1].symbol = aux_s;
			}
		}
	}

    return ordered;
}

// Ordena o vetor de 'node' com base na frequencia de cada no
node* ordena_nvetor(node* nos, node no, int size, int num_nos){

	int i, j;
	int k;
	node aux;
	node *ordered = calloc(num_nos,sizeof(node));	

	for(i=0; i<size; i++){
		if(no.frequency < nos[i].frequency){
			k = i;
			ordered[i] = no;
		}
		else{
			k = size;
			ordered[k] = no;
		}
	}	

	if(k == size){
		for(i=0; i<k+1; i++){
			ordered[i] = nos[i];
		}
	}
	else{
		for(i=0; i<k; i++){
			ordered[i] = nos[i];
		}

		for(i=k+1; i<size; i++){
			ordered[i] = nos[i-1];
		}	
	}

	return ordered;
} 

// Monta arvore de Huffman
node* cria_arvore(huffPair* ordered_hvet, int size){

	int i, j, k;
	int id = 1;
	int num_nos = (2*size)-1;	// o numero de nos da arvore eh dado por: (2*numero_de_simbolos)+1
	
	node *nos = calloc(sizeof(node),num_nos);
printf("num nos = %d\n", num_nos);
printf("size = %d\n", size);
	for(i=0; i<size; i++){																												
		nos[i].frequency = ordered_hvet[i].frequency;
		nos[i].symbol = ordered_hvet[i].symbol;
		nos[i].left = NULL;
		nos[i].right = NULL;
		nos[i].id = 0;			// quando o no eh o proprio simbolo (no folha), eh atribuido 0 para 'id'
	}

	for(i=size; i<num_nos; i++){
		nos[i].frequency = 100;
		nos[i].symbol = -100;
		nos[i].id = 1;			
	}

	for(i=0; i<num_nos; i+=2){
		if(i+1 != num_nos){	

			nos[size].frequency = nos[i].frequency + nos[i+1].frequency;
			nos[size].edge = -1;
			nos[size].parent = -1;
			nos[size].left = &nos[i];
			nos[size].right = &nos[i+1];
			nos[size].id = id;

			nos[i].edge = 0;
			nos[i].parent = id;

			nos[i+1].edge = 1;
			nos[i+1].parent = id;	

			nos = ordena_nvetor(nos, nos[size], size, num_nos);		

			size++;
			id++;		
		}
		else return nos;		
	}		
}

// Constroi o codigo do simbolo no formato 'short'
void forma_codigo(huffCode* codigo, int i, int aux, int tam_arvore){

	int j, k, aux1;
	
	for(j=0; j<codigo[i].num_bit; j++){
		if(codigo[i].parent[j] == 0){
			codigo[i].code += 0; 
			aux--;		
		}
		else{
			codigo[i].code += (int)pow(2,aux);
			aux--;
		}			
	}
}

// Busca pelo pai de cada no, obtendo o codigo de cada simbolo (armazenado em '*parent')
int* buscar_pai(node* arvore, int* parent, int i, int j, int last){

	if(i != j && arvore[i].parent == arvore[j].id){
		if(arvore[j].parent == -1){
			return parent;
		}
		else{

			parent[last] = arvore[j].edge;	
			last--;
			buscar_pai(arvore, parent, j, j+1, last);
		}
	}
	else{
			buscar_pai(arvore, parent, i, j+1, last);
	}	
}

int* numero_bits(node* arvore, int* nbits, int i, int j, int tam_arvore){
	
	int k;	

	if(arvore[j].parent == -1){
		return nbits;
	}
	else{
		for(k=0; k<tam_arvore; k++){		
			if(arvore[j].parent == arvore[k].id){
				nbits[i]++;
				numero_bits(arvore, nbits, i, k, tam_arvore);
			}
		}
	}	
}

// Obtem o codigo de cada simbolo
huffCode* busca_codigo(node* arvore, int tam_arvore){

	int i, j, aux;	
	int k = 0;
	int *nbits;

	huffCode *codigo = calloc(sizeof(huffCode),tam_arvore); 
	nbits = (int*)calloc(sizeof(int),tam_arvore);	
	
	for(i=0; i<tam_arvore; i++){
		numero_bits(arvore, nbits, i, i, tam_arvore);
		codigo[i].num_bit = nbits[i];		
		codigo[i].parent = (int*)calloc(codigo[i].num_bit,sizeof(int));			
	}

	for(i=0; i<tam_arvore; i++){
		if(arvore[i].parent == -1){
			codigo[i].parent[(codigo[i].num_bit)-1] = -1;						// no raiz principal
		}
		else{
			codigo[i].parent[(codigo[i].num_bit)-1] = arvore[i].edge;			// demais nos 
			buscar_pai(arvore, codigo[i].parent, i, 0, (codigo[i].num_bit)-2);
		}
	}		
	
	for(i=0; i<tam_arvore; i++){
		aux = (codigo[i].num_bit)-1;
		forma_codigo(codigo, i, aux, tam_arvore);

		if(arvore[i].id != 0){
			codigo[i].symbol = -1;
		}
	}	

	return codigo;
}

int num_simbolos(short *buffer, int size){

	int i, j, nsym;
	int check[size];

	for(i=0; i<size; i++){
		check[i] = 0;
	}

	for(i=0; i<size; i++){
		if(check[i] == 0){			// o simbolo ainda nao pode ter sido analisado para que sua frequencia seja atualizada
			nsym++;
			printf("n: %d\n", nsym);
			for(j=0; j<size; j++){
				if(i != j && buffer[i] == buffer[j]){
					check[j] = 1;
				}
			}
			check[i] = 1;			
		} 		
	}

	return nsym;
}

void create_huffheader(node * arvore, int num_elementos, short ** vetor, short ** pai){

	int i,child_aux;
	node current_node;

	*vetor = (short*) calloc (num_elementos, sizeof(short));
	*pai = 	(short*) calloc (num_elementos, sizeof(short));

	current_node = arvore[num_elementos];

	fila *q = (fila*) calloc (1, sizeof(fila));

	inicia(q);
	insere(q, &arvore[num_elementos-1]); /* Inserindo o no raiz */

	child_aux = 1;

	for(i = 0; vazia(q) == 0; i++){

		node *current = remove_item(q);


		if(current->left != NULL)
			insere(q, current->left);
		else child_aux--;

		if(current->right != NULL)
			insere(q, current->right);
		else child_aux--;

		if(current->id == 0)
			*(*vetor+i) = current->symbol;
		else{
			*(*pai+i) = 1;
			*(*vetor+i) = i*2+child_aux;
		}
	}

    free_fila(q);
    free(q);
}

int merge_datas(short* file, short ** result, int size){

	short *aux;
	int result_size, i, j, k, current_bit, ptr, shift, bit, aux_count;

	aux = (short*) calloc (size, sizeof(short));

	current_bit = file[0];
	ptr = 1;
	aux_count = 16;
	i = 0;
	result_size = 1;

	for(j = 1; j < size; j+=2){

		/* file[j] = codigo
		   file[j+1] = quantidade de bits do proximo codigo 
		   current_bit = quantidade de bits do codigo atual */

		for(k = current_bit-1; k >= 0; k--){

			if(aux_count == 0){
				i++;
				aux_count = 16;
				ptr = 1;

				result_size++;
			}

			bit = (file[j] >> k) & 1;
			shift = 16 - ptr;
			ptr++;
			aux[i] = aux[i] | (bit << shift);
			aux_count--;

		}

		current_bit = file[j+1];

	}

	*result = (short*) calloc (result_size, sizeof(short));

	for(i = 0; i<result_size; i++){

		*(*result+i) = aux[i];
	}

	free(aux);

	return result_size;

}


short * merge_data_w_header(short* data, short *vetor, short *pai, int num_elementos, int data_size){

	short *result;
	int size_result, i, j;

	size_result = 1 + 2*(num_elementos) + data_size;

	result = (short*) calloc (size_result, sizeof(short));

	result[0] = num_elementos;

	for(i = 0; i < num_elementos; i++)
		result[i+1] = vetor[i];
	for(i = num_elementos + 1, j = 0; j < num_elementos; i++, j++)
		result[i] = pai[j];
	for(i = 0, j = 1 + 2*(num_elementos); i < data_size; i++, j++)
		result[j] = data[i];

	return result;
}

void expand_data_n_header(short * file, int **vetor, int **pai, int **data, int file_size, int *data_size, int *num_elementos){

	int i, j, aux;

	aux = 1 + *num_elementos + *num_elementos;

	*num_elementos = file[0];
	*data_size = file_size - 1 - 2*(file[0]);

	for(i = 0; i < *num_elementos; i++)
		*(*vetor+i) = file[i+1];
	for(i = *num_elementos + 1, j = 0; j < *num_elementos; i++, j++)
		*(*pai+j) = file[i];
	for(i = 0, j = aux; i < *data_size; i++, j++)
		*(*data+i) = file[j];

}

// Realiza a codificao Huffman
int huffman_encoder(short ** result, short *buffer, int size){

	// Cada posicao do vetor 'buffer'(16 bits) eh um simbolo a ser analisado na codificacao Huffman

	int i, j, nsym, data_size, result_size;	
	int k = 0;
	int check[size];
	short *huff, *result1, *result2, *vetor, *pai; 

	huff = (short*)calloc(sizeof(short),(2*size));
	
	huffPair *hvet = calloc(sizeof(huffPair),size);
	nsym = 0;

	for(i=0; i<size; i++){
		hvet[i].frequency = 1; 		// existe pelo menos uma ocorrencia de cada simbolo
		check[i] = 0;				// nenhum simbolo  ainda foi analisado
	}

	for(i=0; i<size; i++){		

		if(check[i] == 0){			// o simbolo ainda nao pode ter sido analisado para que sua frequencia seja atualizada
			nsym++;
			for(j=0; j<size; j++){
				if(i != j && buffer[i] == buffer[j]){
					hvet[i].frequency++;
					check[j] = 1;
				}
			}
			check[i] = 1;
			hvet[i].symbol = buffer[i];
		} 
		else{
			hvet[i].frequency = 0;		
		}
	}

	for(i=0; i<size; i++){
		hvet[i].frequency = (hvet[i].frequency)/size;
	}

	// Manipulando frequencias
	huffPair *ordered_hvet = calloc(sizeof(huffPair),nsym);
	ordered_hvet = ordena_hvetor(hvet, size, nsym);		

	// Manipulando arvore
    int tam_arvore = (2*nsym)-1;
    node *arvore = calloc(sizeof(node),tam_arvore);		
    arvore = cria_arvore(ordered_hvet, nsym);    

    // Manipulando codigos
    huffCode *codigo = calloc(sizeof(huffPair),tam_arvore);
    codedPair *coded = calloc(sizeof(codedPair),nsym);

    for(i=0; i<nsym; i++){
    	coded[i].symbol = arvore[i].symbol;
    }
print_arvore(arvore, tam_arvore);
	codigo = busca_codigo(arvore, tam_arvore);	

	int r = 0;

	for(i=0; i<tam_arvore; i++){
		if(codigo[i].symbol != -1){
			coded[r].code = codigo[i].code;
			coded[r].num_bit = codigo[i].num_bit;
			r++;
		}
	}		

	for(i=0; i<size; i++){
		for(j=0; j<nsym; j++){
			if(buffer[i] == coded[j].symbol){
				huff[k] = coded[j].num_bit;
				k++;
				huff[k] = coded[j].code;
				k++;
			}
		}
	}
print_vetor(huff, k);
	data_size = merge_datas(huff, &result1, 2*size);

	create_huffheader(arvore, tam_arvore, &vetor, &pai);

	*result = merge_data_w_header(result1, vetor, pai, tam_arvore, data_size);
	result_size = 1 + 2*(tam_arvore) + data_size;

	return result_size;
}

void print_arvore(node *arvore, int size){

	int i;

	for(i=0; i<size; i++){

		printf("simbolo: %d id: %d pai: %d freq: %f edge: %d\n", arvore[i].symbol, arvore[i].id, arvore[i].parent, arvore[i].frequency, arvore[i].edge);
	}
	printf("\n");

}

int next_bit(short *entrada, int i, int j){
	return ((entrada[i] >> j) & 1);
}

short busca_simbolo(short *entrada, short *vetor, int pos, int nsym, int i, int j, int bit, int *pai){

	if(pai[pos] == 0){
		return vetor[pos];
	}

	j--;

	if(j<0){
		i++;
		j = 15;
	}

	if(bit == 0)
		pos = vetor[pos];
	else
		pos = vetor[pos]+1;

	busca_simbolo(entrada, vetor, pos, nsym, i, j, next_bit(entrada, i, j),pai);

}
