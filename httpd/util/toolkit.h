struct arraylist* split(char* string, char* delimiter);
int string_to_int(char* value, int should_be_positive);
int contains_double_dot(char* string);
int get_file_size(char* file_name);
void print_file_contents(FILE* file);
int safe_fork();
int is_parent(int process_id);
int is_child(int process_id);