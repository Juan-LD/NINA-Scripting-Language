#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <linux/limits.h>

long int current_line = 0, pet_nina = 0, function_arg_num = 0, errors = 0;
char ReadLine [2000], ReadLineUF [2000];

void FormatWhiteSpace(void);
void ShowError(const char * line, bool is_fatal, bool is_normal);
char * ReturnArgument(const char * line, const char key_word, int starting_arg);
void RemoveWhiteSpace(char * line);
bool IsValidBool(const char * line);
bool IsValidChar(const char * line);
bool IsValidInt(const char * line);

//"Compiler" to convert .txt files into .NINA files
int main(int agrv, char *argc[]){

    setlocale(LC_ALL, "");
    clock_t begin = clock();
    bool debug = false;

    if(agrv != 2){
        ShowError("Usage: ./NINAFileMaker [File Name]", true, false);
        return 1;
    }

    bool handle_dir = false;
    int argcstrlen = strlen(argc[1]);

    if(argc[1][argcstrlen - 1] != 't' && argc[1][argcstrlen - 2] != 'x' && argc[1][argcstrlen - 3] != 't' && argc[1][argcstrlen - 4] != '.'){
        ShowError("File extention MUST be .txt", true, false);
        return 2;
    }

    for(int i = 0; i < argcstrlen; i ++){
        if(argc[1][i] == '/'){
            handle_dir = true;
            break;
        }
    }

    if(handle_dir){
		char next_dir[PATH_MAX];
		memset(next_dir, '\0', sizeof(next_dir));
		int cur_char = -1, last_slash_pos = 0;

		//Switch to dir if necessary
		while(true){
			int temp_int = 0;

			if(argc[1][cur_char] != '.'){
				cur_char ++;
				memset(next_dir, '\0', sizeof(next_dir));
			}

			while(argc[1][cur_char] != '/' && argc[1][cur_char] != '.'){
				next_dir[temp_int] = argc[1][cur_char];
				cur_char ++;
				temp_int ++;
			}

			if(argc[1][cur_char] != '.'){
				last_slash_pos = cur_char;
				if(chdir(next_dir) != 0){
                    ShowError("Issues with changing bettewn file directory", true, false);
					return 3;
				}
			}

			if(argc[1][cur_char] == '.'){
				last_slash_pos ++;
				int name_strlen = strlen(argc[1]);
				temp_int = 0;

				while(last_slash_pos != name_strlen){
					next_dir[temp_int] = argc[1][last_slash_pos];
					last_slash_pos ++;
					temp_int ++;
				}

				break;
			}
		}

		argc[1] = next_dir;
	}

    argcstrlen = strlen(argc[1]);

    char NINAFileName [argcstrlen + 1];
    memset(NINAFileName, '\0', argcstrlen + 1);
    strcpy(NINAFileName, argc[1]);

    NINAFileName[argcstrlen] = 'a';
    NINAFileName[argcstrlen - 1] = 'n';
    NINAFileName[argcstrlen - 2] = 'i';
    NINAFileName[argcstrlen - 3] = 'n';
    NINAFileName[argcstrlen + 1] = '\0';

    if(access(argc[1], F_OK)!= 0){
        ShowError("File does not exist", true, false);
        return 4;
    }

    char temp [PATH_MAX];
    memset(temp, '\0', sizeof(temp));
    getcwd(temp, sizeof(temp));

    FILE * finterpert = fopen(argc[1], "r");
    chdir(temp);

    if(access("__temp.nina", F_OK)== 0){
        ShowError("Auto removing TEMP FILE called \"__temp.nina\", please re-run the compiler.", true, false);
        return 5;
    }

    FILE * fnina = fopen("__temp.nina", "w");

    memset(ReadLine, '\0', 2000); memset(ReadLineUF, '\0', 2000);

    current_line ++;
    //Type checking
    fgets(ReadLine, 2000, finterpert);
    FormatWhiteSpace();
    for(int i = 0; i < strlen(ReadLine); i ++)
    ReadLine[i] = tolower(ReadLine[i]);

    if(strncmp(ReadLine, "script", 6)== 0)
    fprintf(fnina, "1\n");

    else if(strncmp(ReadLine, "debug", 5)== 0){
        fprintf(fnina, "2\n");
        debug = true;
    }

    else {
        ShowError("Unknown file type, did you mean \"Script\" or \"Debug\"?", false, true);
        errors ++;
    }

    //Attribute checking
    current_line ++;
    memset(ReadLine, '\0', 2000); memset(ReadLineUF, '\0', 2000);
    fgets(ReadLine, 2000, finterpert);
    FormatWhiteSpace();
    for(int i = 0; i < strlen(ReadLine); i ++)
    ReadLine[i] = tolower(ReadLine[i]);

    /*
    Here is the attribute list:

    0 -> Put (*ALL) text to the middle of the line                               .centertext
    1 -> Put (*ALL) text to the right of the line                                .righttext
    5 -> Ignore displacement and set to this one (New Displacement value rq)     .ignoredisplacement{value}

    * -> Functions that specify a certain text format type will NOT be affected by the attribute (CenterText / RightText)
    ** -> Functions that specify a change in color / font style will NOT be affected by the attribute, instead they will stack together (ChangeTerminalColor / ChangeTerminalFont)
    */

	char * rest, temp_char = '\0';
	char delim [2] = ".";
    int displacement;

	rest = strtok(ReadLine, delim);

    int attrib_id = -1;

    bool is_attrib_active[4] = { false };
    while(rest != NULL){

        bool has_no_args = true;

        if(strcmp(rest, "centertext")== 0)
        attrib_id = 0;

        else if(strcmp(rest, "righttext")== 0)
        attrib_id = 1;

        else if(strncmp(rest, "ignoredisplacement(", 19)== 0)
        attrib_id = 2;

        if(attrib_id == 3){
            displacement = 19;
            has_no_args = false;
        }

        is_attrib_active[attrib_id] = true;

        for(int i = 0; i < 3; i ++){
            if(is_attrib_active[i]){
                ShowError("Sorry, you can't have more than one attribute active at the same time", false, true);
                errors ++;
            }
        }

        fprintf(fnina, "%d\n", attrib_id);

        if(attrib_id == 2) {
            int cur_char = displacement, repeat_times = 1;
            char key_word = ')';

            int temp = 0;
            char next_val[2000];
            memset(next_val, '\0', sizeof(next_val));

            while(rest[cur_char] != key_word){
                next_val[temp] = rest[cur_char];
                cur_char ++;
                temp ++;
            }

            cur_char ++;

            if(next_val[0] == '\0'){
                ShowError("Expected argument value for attribute.", false, true);
                errors ++;
            }

            fprintf(fnina, "%s\n", next_val);
        }

        rest = strtok(NULL, delim);
    }

    memset(ReadLine, '\0', 2000); memset(ReadLineUF, '\0', 2000);
    int next_ptr_disp = ftell(finterpert);

    fprintf(fnina, "START\n");
    while(fgets(ReadLine, 2000, finterpert)!= NULL){

        bool is_function = false;
        int displacement = 0, argument_number = 0;
        char current_keyword [50] = { '\0' };

        if(ReadLineUF[0] != '\0'){
            strcpy(ReadLine, ReadLineUF);
            fseek(finterpert, next_ptr_disp - ftell(finterpert), SEEK_CUR);
        }

        FormatWhiteSpace();
        function_arg_num = 0;

        if(ReadLineUF[0] == '\0'){
            strcpy(ReadLineUF, ReadLine);
            next_ptr_disp = ftell(finterpert);
            current_line ++;
        }

        for(int i = 0; i < strlen(ReadLine); i ++)
        ReadLine[i] = tolower(ReadLine[i]);

        FILE * fkeywords = fopen("ArtKeyWords.txt", "r");

        //KEYWORD CHECKING GOES HERE
        while(fscanf(fkeywords, "%s\n", current_keyword)!= EOF){
            argument_number ++;

            int cur_lenght = strlen(current_keyword);
            if(strncmp(ReadLine, current_keyword, cur_lenght)== 0)
            break;

            memset(current_keyword, '\0', sizeof(current_keyword));
        }
        displacement = strlen(current_keyword);

        fclose(fkeywords);

        //Handler (programmed with the art moduel only for now)
        if(argument_number > 0 && argument_number < 31){

            //Get what args we want
            FILE * fargs = fopen("ArgsReq.txt", "r");
            for(int i = 0; i < argument_number; i ++){
                memset(current_keyword, '\0', sizeof(current_keyword));
                fscanf(fargs, "%s\n", current_keyword);
            }
            fclose(fargs);

            int char_count = 0, int_count = 0, bool_count = 0, max_limit_expected = 0, handler_count = 0;

            /*
                Here is the full list of argument types
                    B - Boolean argument here
                    C - String argument here
                    H - Needs custom handler
                    I - Integer argument here
                    L - Set up next delimiters for the argument (only works with String arguments)
                    O - No arguments
                    / - Comment (aka no arguments)
            */

            //Counting argument types (and other things) wanted (needed for later)
            for(int i = 0; i < strlen(current_keyword); i ++){
                switch(current_keyword[i]){
                    case 'B':
                        bool_count ++;
                        break;

                    case 'C':
                        char_count ++;
                        break;

                    case 'H':
                        if(argument_number ==  7)
                        handler_count = 4;

                        break;

                    case 'I':
                        int_count ++;
                        break;

                    case 'L': {
                        int cur_delim = 1;

                        while(current_keyword[i - 1] != ')'){
                            if(current_keyword[i] == ',')
                            cur_delim ++;

                            i ++;
                        }

                        if(cur_delim > max_limit_expected)
                        max_limit_expected = cur_delim;

                        break;
                    }

                    case 'O':
                        break;

                    //Sometimes comments get in the way :>
                    case '/':
                        break;

                    default:
                        printf("\x1b[31mCouldnt handle argument type %c ( %s )\n\x1b[0m", current_keyword[i], current_keyword);
                        break;
                }
            }

            bool delim_next = false, break_out_of_this_loop = false;
            char * catch_chars [char_count];
            char * catch_ints [int_count];
            char * catch_booleans [bool_count];
            char * limit_arguments [max_limit_expected];
            char * custom_handler [handler_count];

            char custom_handler1_placeholder [3];
            memset(custom_handler1_placeholder, '\0', sizeof(custom_handler1_placeholder));
            //^Variable needs higher access becase "custom_handler[1]" will eventually point to it and it tends to create corrupt memory
            //If the variable does not have access to the "printf to file and free everything" zone

            for(int i = 0; i < max_limit_expected; i ++)
            limit_arguments[i] = '\0';

            int cur_char = -1, argument_type_count [3] = { 0 }, skip_delim = 0;
            while(cur_char != strlen(current_keyword) - 1){

                char char_to_check = ',';
                cur_char ++;

                if(cur_char == strlen(current_keyword) - 1 - skip_delim)
                char_to_check = ')';

                //Setting up delims for next argument call
                if(current_keyword[cur_char + 1] == 'L'){
                    skip_delim = cur_char + 3;
                    int current_lim = 0;

                    //Repeat argument getting untill done
                    while(current_keyword[skip_delim - 1] != ')'){
                        int temp = skip_delim;

                        //See the next keyword for argument
                        while(current_keyword[temp] != ',' && current_keyword[temp] != ')'){
                            char_to_check = current_keyword[temp + 1];
                            temp ++;
                        }

                        limit_arguments[current_lim] = ReturnArgument(current_keyword, char_to_check, skip_delim);
                        skip_delim += strlen(limit_arguments[current_lim]) + 1;

                        current_lim ++;
                    }

                    delim_next = true;
                }

                switch(current_keyword[cur_char]){

                    //Handle booleans
                    case 'B':{
                        function_arg_num ++;
                        catch_booleans[argument_type_count[0]] = ReturnArgument(ReadLineUF, char_to_check, displacement);
                        displacement += strlen(catch_booleans[argument_type_count[0]]) + 1;

                        if(catch_booleans[argument_type_count[0]] == NULL)
                        break;

                        if(!IsValidBool(catch_booleans[argument_type_count[0]])){
                            ShowError("Expected a BOOLEAN value for this argument.", false, true);
                            errors ++;
                        }

                        RemoveWhiteSpace(catch_booleans[argument_type_count[0]]);

                        if(strcmp(catch_booleans[argument_type_count[0]], "true")== 0)
                        strcpy(catch_booleans[argument_type_count[0]], "1");

                        else if(strcmp(catch_booleans[argument_type_count[0]], "false")== 0)
                        strcpy(catch_booleans[argument_type_count[0]], "0");

                        argument_type_count[0] ++;
                        break;
                    }

                    //Handle chars
                    case 'C':{
                        function_arg_num ++;
                        catch_chars[argument_type_count[1]] = ReturnArgument(ReadLineUF, char_to_check, displacement);

                        if(catch_chars[argument_type_count[1]] == NULL)
                        break;

                        if(!IsValidChar(catch_chars[argument_type_count[1]])){
                            ShowError("Invalid CHAR argument value for this argument. (needs to be \"[ARG]\")", false, true);
                            errors ++;
                        }

                        //Check this weird error
                        if(catch_chars[argument_type_count[1]] != NULL)
                        displacement += strlen(catch_chars[argument_type_count[1]]) + 1;

                        RemoveWhiteSpace(catch_chars[argument_type_count[1]]);

                        //Delimter (will check if the argument passed or not)
                        if(delim_next){
                            bool passed = false;
                            for(int i = 0; i < max_limit_expected; i ++){
                                passed = false;

                                if(limit_arguments[i] != NULL && strcmp(catch_chars[argument_type_count[1]], limit_arguments[i])== 0){
                                    passed = true;
                                    break;
                                }
                            }

                            if(!passed){
                                ShowError("This argument does not respect the delimiters set.", false, true);
                                errors ++;
                            }

                        }

                        //This is just to remove the anoying "" inside the argument
                        int strlen_wanted = strlen(catch_chars[argument_type_count[1]]);
                        for(int i = 0; i <  strlen_wanted - 2; i ++)
                        catch_chars[argument_type_count[1]][i] = catch_chars[argument_type_count[1]][i + 1];

                        catch_chars[argument_type_count[1]][strlen_wanted - 2] = '\0';

                        argument_type_count[1] ++;
                        break;
                    }

                    case 'H':{
                        if(argument_number == 7){
                            custom_handler[0] = ReturnArgument(ReadLineUF, ')', displacement);
                            if(custom_handler[0] == NULL)
                            break;

                            RemoveWhiteSpace(custom_handler[0]);

                            if(custom_handler[0][0] != 'F' && custom_handler[0][0] != 'B' && custom_handler[0][0] != 'S'){
                                ShowError("Unknown format type (is not F nor B nor S)", false, true);
                                errors ++;
                            }

                            int strlen_wanted = strlen(custom_handler[0]) - 1;
                            if(strlen_wanted > 3)
                            strlen_wanted = 3;

                            //Get custom handler 1 (format id)
                            for(int i = 0; i < strlen_wanted; i ++)
                            custom_handler1_placeholder[i] = custom_handler[0][i + 1];

                            //Removing garbo values and setting the handler 1 to be the placeholder
                            custom_handler1_placeholder[strlen_wanted] = '\0';
                            custom_handler[1] = custom_handler1_placeholder;

                            displacement += strlen(custom_handler[0]) + 2;

                            if(!IsValidInt(custom_handler[1])){
                                ShowError("Expected a INTEGER value for this argument. (format type mode)", false, true);
                                errors ++;
                            }

                            RemoveWhiteSpace(custom_handler[1]);

                            custom_handler[3] = NULL;
                            custom_handler[2] = ReturnArgument(ReadLineUF, ')', displacement);

                            //"END" keyword failed
                            if(strcasecmp(custom_handler[2], "end")!= 0){
                                custom_handler[2] = ReturnArgument(ReadLineUF, ',', displacement);
                                if(custom_handler[2] == NULL)
                                break;

                                if(!IsValidInt(custom_handler[2])){
                                    ShowError("Expected INTEGER value for this argument OR END keyword. (Until argument 1)", false, true);
                                    errors ++;
                                }
                                RemoveWhiteSpace(custom_handler[2]);

                                displacement += strlen(custom_handler[2]) + 1;

                                custom_handler[3] = ReturnArgument(ReadLineUF, ')', displacement);
                                if(custom_handler[3] == NULL)
                                break;

                                if(!IsValidInt(custom_handler[3])){
                                    ShowError("Expected INTEGER value for this argument (Until argument 2)", false, true);
                                    errors ++;
                                }
                                RemoveWhiteSpace(custom_handler[3]);
                            }
                        }

                        else
                        pet_nina ++;

                        break_out_of_this_loop = true;
                        break;
                    }

                    //Handle ints
                    case 'I':{
                        function_arg_num ++;
                        catch_ints[argument_type_count[2]] = ReturnArgument(ReadLineUF, char_to_check, displacement);
                        if(catch_ints[argument_type_count[2]] == NULL)
                        break;

                        displacement += strlen(catch_ints[argument_type_count[2]]) + 1;

                        if(!IsValidInt(catch_ints[argument_type_count[2]])){
                            ShowError("Expected a INTEGER value for this argument.", false, true);
                            errors ++;
                        }

                        RemoveWhiteSpace(catch_ints[argument_type_count[2]]);
                        argument_type_count[2] ++;
                        break;
                    }

                    //Just skip it (already executed)
                    case 'L':
                        cur_char += skip_delim - 3; //This constant is kinda dangerous, keep an eye on it if things go wrong :>
                        break;

                    //Empty argument
                    case 'O':
                        break;

                    default:
                        break;
                }

                if(break_out_of_this_loop)
                break;
            }

            fprintf(fnina, "%d\n", argument_number);

            for(int i = 0; i < 3; i ++)
            argument_type_count[i] = 0;

            for(int i = 0; i < strlen(current_keyword); i ++){
                switch(current_keyword[i]){
                    case 'B': {
                        if(catch_booleans[argument_type_count[0]] != NULL){
                            fprintf(fnina, "%s\n", catch_booleans[argument_type_count[0]]);

                            free(catch_booleans[argument_type_count[0]]);
                            argument_type_count[0] ++;
                        }
                        break;
                    }

                    case 'C': {
                        if(catch_chars[argument_type_count[1]] != NULL){
                            fprintf(fnina, "%s\n", catch_chars[argument_type_count[1]]);

                            free(catch_chars[argument_type_count[1]]);
                            argument_type_count[1] ++;
                        }
                        break;
                    }

                    case 'H':{
                        for(int i = 0; i < handler_count; i ++){
                            if(custom_handler[i] != NULL){
                                if(i == 0)
                                fprintf(fnina, "%c\n", custom_handler[i][0]);

                                else
                                fprintf(fnina, "%s\n", custom_handler[i]);

                                //Cannot free because memory is on a char (*)[3] variable
                                if(i != 1)
                                free(custom_handler[i]);
                            }
                        }

                        break;
                    }

                    case 'I': {
                        if(catch_ints[argument_type_count[2]] != NULL){
                            fprintf(fnina, "%s\n", catch_ints[argument_type_count[2]]);

                            free(catch_ints[argument_type_count[2]]);
                            argument_type_count[2] ++;
                        }
                        break;
                    }

                    //This may or may not give seg. fault if there are two delims in the same line
                    case 'L': {
                        for(int i = 0; i < max_limit_expected; i ++){
                            if(limit_arguments[i] != NULL)
                            free(limit_arguments[i]);
                        }
                        break;
                    }

                    default:
                        break;
                }
            }

        } else {
            ShowError("Unknown Keyword", false, true);
            errors ++;
        }

        //Second command in one line checker
        int next_command_starter = 0;
        for(int i = displacement; i < strlen(ReadLineUF); i ++){
            if(ReadLineUF[i] == '.'){
                next_command_starter = i + 1;
                break;
            }
        }

        memset(ReadLine, '\0', 2000);

        if(next_command_starter != 0){
            for(int i = 0; i < strlen(ReadLineUF) - next_command_starter; i ++)
            ReadLine[i] = ReadLineUF[i + next_command_starter];

            strcpy(ReadLineUF, ReadLine);
            RemoveWhiteSpace(ReadLineUF);
        } else
        memset(ReadLineUF, '\0', 2000);

    }

    fclose(finterpert);
    fclose(fnina);

    printf("\n");

    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;

    if(debug)
    printf("\x1b[38;5;226m[DEBUG] Compilation took %.6lf seconds\x1b[0m\n", time_spent);

    if(errors == 0){
        rename("__temp.nina", NINAFileName);
        printf("\x1b[38;5;70mNo errors during compilation!\x1b[0m\n");
    }

    else {
        remove("__temp.nina");
        printf("\x1b[31mNumber of errors during program compilation: %ld\x1b[0m\n", errors);
    }

    if(pet_nina > 0)
    printf("\x1b[38;5;33mNina was pet %ld times during compilation!\x1b[0m\n", pet_nina);

    printf("\n");
    return 0;
}

//Used to not get any weird "unknown keyword" errors
void FormatWhiteSpace(void){
    bool lmk_if_words = false;
    int next_index = 0;

    for(int i = 0; i < strlen(ReadLine); i ++){
        if(ReadLine[i] != 32 && ReadLine[i] != 10){
            next_index = i;
            lmk_if_words = true;
            break;
        }
    }

    if(lmk_if_words){
        int bckp_next_index = next_index;
        for(int i = 0; i < strlen(ReadLine); i ++){
            ReadLine[i] = ReadLine[bckp_next_index];
            bckp_next_index ++;
        }

        next_index += strlen(ReadLine);
        ReadLine[next_index] = '\0';

    } else
    strcpy(ReadLine, "//");

    return;
}

//Show err. :>
void ShowError(const char * line, bool is_fatal, bool is_normal){
    printf("\n");

    if(is_normal)
    printf("On line %ld: %s", current_line, ReadLineUF);

    printf("\x1b[31m%s", line);

    if(is_normal && function_arg_num > 0)
    printf(" (on argument %ld)", function_arg_num);

    printf("\x1b[0m\n");

    if(is_fatal){
        printf("\n");
        remove("__temp.nina");
    }
    return;
}

//Making it eazier to get the seperate arguments
char * ReturnArgument(const char * line, const char key_word, int starting_arg){
    if(line == NULL)
    return NULL;

    int temp = 0;
    char * rest_wanted = (char *)malloc(2000);
    memset(rest_wanted, '\0', 2000);

    while(line[starting_arg] != key_word){
        if(line[starting_arg] == 10 || starting_arg > strlen(line)){
            free(rest_wanted);
            ShowError("Missing finishing keyword for argument", false, true);
            errors ++;
            return rest_wanted;
        }

        rest_wanted[temp] = line[starting_arg];
        starting_arg ++;
        temp ++;
    }

    if(rest_wanted[0] == '\0'){
        free(rest_wanted);
        ShowError("Expected argument value for function.", false, true);
        errors ++;
        return NULL;
    }

    //Is empty?
    int temptemp = 0;
    for(int i = 0; i < strlen(rest_wanted); i ++){
        if(rest_wanted[i] == 0 || rest_wanted[i] == 10 || rest_wanted[i] == 32)
        temptemp ++;
    }

    if(temptemp == strlen(rest_wanted)){
        ShowError("Empty argument :/", false, true);
        errors ++;
    }

    return rest_wanted;
}

void RemoveWhiteSpace(char * line){
    if(line == NULL)
    return;

    int index = 0, line_strlen = strlen(line);
    while(line[index] == ' ')
    index ++;

    for(int i = 0; i < line_strlen; i ++)
    line[i] = line[index + i];

    line_strlen -= index;
    if(line[line_strlen - 1] == ' ')
    line[line_strlen - 1] = '\0';

    return;
}

bool IsValidBool(const char * line){
    if(line == NULL)
    return false;

    char copy_of_line[strlen(line)];
    memset(copy_of_line, '\0', sizeof(line));
    strcpy(copy_of_line, line);

    RemoveWhiteSpace(copy_of_line);

    if(strncmp(copy_of_line, "true", 4)== 0 || strncmp(copy_of_line, "false", 5)== 0 || strncmp(copy_of_line, "1", 1)== 0 || strncmp(copy_of_line, "0", 1)== 0)
    return true;

    else
    return false;
}

bool IsValidChar(const char * line){
    if(line == NULL)
    return false;

    char copy_of_line[strlen(line)];
    memset(copy_of_line, '\0', sizeof(line));
    strcpy(copy_of_line, line);

    RemoveWhiteSpace(copy_of_line);

    if(copy_of_line[0] != '"' || (copy_of_line[strlen(copy_of_line) - 1] != '"') || (copy_of_line[strlen(copy_of_line) - 2] == '\\' && copy_of_line[strlen(copy_of_line) - 1] == '"'))
    return false;

    for(int i = 1; i < strlen(copy_of_line) - 1; i ++){
        if(copy_of_line[i] == '"' && copy_of_line[i - 1] != '\\')
        return false;
    }

    return true;
}

bool IsValidInt(const char * line){
    if(line == NULL)
    return false;

    char copy_of_line[strlen(line)];
    memset(copy_of_line, '\0', sizeof(line));
    strcpy(copy_of_line, line);

    RemoveWhiteSpace(copy_of_line);

    for(int i = 0; i < strlen(copy_of_line); i ++){
        if(isdigit(copy_of_line[i])== 0)
        return false;
    }

    return true;
}
