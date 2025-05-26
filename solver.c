#include "solver.h"

extern hashset set_init; // Assuming this is your dictionary hashset

void check_word(const char* word) {
    if (search(set_init, word)) {
        // Process the found word (e.g., print or store it)
        fprintf(stderr, "Found word: %s\n", word);
    }
}

void search_direction(char** puzzle, int rows, int cols, int startRow, int startCol, int dRow, int dCol) {
    int length = (dRow != 0 && dCol != 0) ? min(rows, cols) : (dRow != 0 ? rows : cols);
    char word[length + 1];

    for (int len = 3; len <= length; len++) {
        for (int i = 0; i < len; i++) {
            int r = startRow + i * dRow;
            int c = startCol + i * dCol;
            word[i] = puzzle[r][c];
        }
        word[len] = '\0';
        check_word(word);
    }
}

void* solve(void* arg) {
    sub_puzzle_info* info = (sub_puzzle_info*) arg;

    for (int row = 0; row < info->rows; row++) {
        for (int col = 0; col < info->cols; col++) {
            // Search in all directions from each cell
            search_direction(info->puzzle, info->rows, info->cols, row, col, 0, 1); // Horizontal L-R
            search_direction(info->puzzle, info->rows, info->cols, row, col, 0, -1); // Horizontal R-L
            search_direction(info->puzzle, info->rows, info->cols, row, col, 1, 0); // Vertical T-B
            search_direction(info->puzzle, info->rows, info->cols, row, col, -1, 0); // Vertical B-T
            search_direction(info->puzzle, info->rows, info->cols, row, col, 1, 1); // Diagonal slope +1
            search_direction(info->puzzle, info->rows, info->cols, row, col, -1, -1); // Diagonal slope -1
        }
    }

    free(info);
    fprintf(stderr, "Solver thread: Finished processing a sub-puzzle.\n");
    return NULL;
}

void print_buffer(char** sub_puzzle, int subpuzzle_rows, int subpuzzle_cols){
	//this function is used for printing the content of each buffer cell.
	//do NOT call this function anywhere in your final submission.
	printf("%d by %d\n", subpuzzle_rows, subpuzzle_cols);
	for(int i = 0; i < subpuzzle_rows;i++)
		for(int j = 0; j < subpuzzle_cols;j++)
			printf("%c%s", sub_puzzle[i][j], j == subpuzzle_cols - 1?"\n":"");
}
int main(int argc, char** argv){
	tnode* root = NULL;
	
	if (argc < 11)
		error("Fatal Error. Usage: solve -dict dict.txt -input puzzle1mb.txt -size 1000 -nbuffer 64 -len 4:7 [-s]", 1);
	int puzzle_size = 0, buf_cells = 0, i = 0,j, fd, min_len = 0, max_len = 0, sorted = 0, buf_dimension;
	char* filename = NULL;
	FILE* dictionary_file;
	char*** buffer;
	while(++i < argc){
		if(argv[i][0] != '-')
			error("Fatal Error. Invalid CLA", 2);
		if(!strcmp(argv[i], "-size")){
			puzzle_size = atoi(argv[++i]);
			if(puzzle_size < 15 || puzzle_size > 46340)
				error("Fatal Error. Illegal value passed after -size",3);
		}
		else if(!strcmp(argv[i], "-nbuffer")){
			buf_cells = atoi(argv[++i]);
			if(buf_cells != 1 && buf_cells != 4 && buf_cells != 16
				&& buf_cells != 64)
				error("Fatal Error. Illegal value passed after -nbuffer",4);
			buf_dimension = (int)sqrt(MEM_LIMIT / buf_cells);
			//buf_cells = 1 -> buf_dimension = 8000
			//buf_cells = 4 -> buf_dimension = 4000
			//buf_cells = 16 -> buf_dimension = 2000
			//buf_cells = 64 -> buf_dimension = 1000
		}
		else if(!strcmp(argv[i], "-input")){
			filename = strdup(argv[++i]);
			fd = open(filename, O_RDONLY, 0);
			if(fd < 0)
				error("Fatal Error. Illegal value passed after -input",5);
		}else if(!strcmp(argv[i], "-dict")){
			dictionary_file = fopen(argv[++i], "r");
			if(!dictionary_file)
				error("Fatal Error. Illegal value passed after -dict",6);
		}else if(!strcmp(argv[i], "-len")){
			char* min_max = strdup(argv[++i]);
			char* max_str;
			if(strrchr(min_max, ':') != (max_str = strchr(min_max, ':'))
				|| !strchr(min_max, ':'))
				error("Fatal Error. Illegal value passed after -len",7);
			max_len = atoi(max_str+1);
			min_max[max_str - min_max] = '\0';
			min_len = atoi(min_max);
			if(min_len < 3 || max_len > 14 || min_len > max_len)
				error("Fatal Error. Illegal value passed after -len",7);
		}else if(!strcmp(argv[i], "-s"))
			sorted = 1;
		else{
			error("Fatal Error. Usage: solve -dict dict.txt -input puzzle1mb.txt -size 1000 -nbuffer 64 -len 4:7 [-s]", 1);
			break;
		}
	}
	//Read and move all words from dictionary_file to a new hash table (hashset)
	//Your code here...
	
	// Open the dictionary file
	FILE *dict_file = fopen("dict.txt", "r");
	if (dict_file == NULL) {
		// Handle the error if the file cannot be opened
		fprintf(stderr, "Error opening dictionary file.\n");
		exit(1);
	}

	// Initialize the hashset
	hashset my_hashset = set_init();

	// Read words from the file and insert them into the hashset
	char word[256]; // Assuming each word will not exceed 255 characters
	while (fscanf(dict_file, "%255s", word) == 1) {
		insert(&my_hashset, word);
	}

// Close the dictionary file
fclose(dict_file);

	
	//allocate 64MB of buffer in the heap
	//buffer is a 3D array
	//on the outermost dimension, we have buf_cells elements
	//each element is given to one consumer thread
	//each element is going to store a (square-shape) sub-puzzle
	//each element is a square-shape 2D array of characters
	//dimension of this square-shape array is buf_dimension by buf_dimension
	buffer = (char***)malloc(buf_cells * sizeof(char**));
	for(i = 0;i < buf_cells;i++){
		buffer[i] = (char**)malloc(buf_dimension * sizeof(char*));
		for(j = 0; j < buf_dimension;j++)
			buffer[i][j] = (char*) malloc(buf_dimension);
	}
	int buf_index = 0;
	pthread_t t_id[buf_cells];
	for(i = 0; i < buf_cells;i++)
		t_id[i] = NULL;
	for(int row = 0; row + max_len - 1 < puzzle_size; row += (buf_dimension - max_len + 1)){
		int subpuzzle_rows = (row + buf_dimension <= puzzle_size)?
				 buf_dimension:	puzzle_size - row;
		for(int column = 0; column + max_len - 1 < puzzle_size;column += (buf_dimension - max_len + 1)){
			long start = (long)row * (puzzle_size+1) + column;
			lseek(fd,start,SEEK_SET);
			int subpuzzle_cols = (column + buf_dimension <= puzzle_size)?
				 buf_dimension:	puzzle_size - column;
			if(t_id[buf_index])//if there is a busy consumer/solver, 
				pthread_join(t_id[buf_index], NULL);//wait for it to finish the job before manipulating the buffer[buffer_index]
			for(i = 0; i < subpuzzle_rows;i++){
				int n_read = read(fd, buffer[buf_index][i], subpuzzle_cols);
				if(n_read < subpuzzle_cols)
					error("Fatal Error. Bad read from input file", 10);
				if(subpuzzle_cols < buf_dimension)
					buffer[buf_index][i][subpuzzle_cols] = '\0';
				lseek(fd, puzzle_size-subpuzzle_cols+1, SEEK_CUR);
			}
			if(subpuzzle_rows < buf_dimension)
				buffer[buf_index][subpuzzle_rows] = NULL;
			
			//modify these lines so that you can create and start a solver thread 
			
			// Allocate memory for sub-puzzle information
			sub_puzzle_info* info = (sub_puzzle_info*) malloc(sizeof(sub_puzzle_info));
			if (!info) {
				fprintf(stderr, "Failed to allocate memory for sub_puzzle_info\n");
				exit(1); // Or handle the error appropriately
			}

			// Assign sub-puzzle data to the structure
			info->puzzle = buffer[buf_index];
			info->rows = subpuzzle_rows;
			info->cols = subpuzzle_cols;
			info->start_row = row;
			info->start_col = column;

			// Create and start the solver thread
			fprintf(stderr, "Consuming buffer #%d\n", buf_index);
			pthread_create(&t_id[buf_index], NULL, solve, info);

			// Optionally, uncomment to print the buffer for debugging
			// print_buffer(buffer[buf_index], subpuzzle_rows, subpuzzle_cols);

			
			//end of modification
			buf_index = (buf_index == buf_cells - 1)?0: buf_index + 1;
		}
	}
	for(i = 0; i < buf_cells;i++)
		if(t_id[i])
			pthread_join(t_id[i], NULL);
	
	if(sorted){
		inorder_print(root);
	}
}