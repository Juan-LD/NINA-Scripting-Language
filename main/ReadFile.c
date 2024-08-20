/*
 * PLEASE BE AWARE THIS FILE DOES NOT WORK DUE TO IT BEING UNLINKED FROM THE LIBRARY ITS SUPOST TO BE LINKED TO.
*/

//Nothing to see here... (i warned you)
int ReadNINAFile(char *FileName){

	bool debug_mode = false, contains_dir = false;
	int argument_number = 0;
	char ReadLine [2000];

	//This linked list was from cs50 rubber duck debuger
	typedef struct node
	{
		long int PTR_pos, repeat_times;
		struct node* next;
	} node;

	node* head = NULL;

	for(int i = 0; i < strlen(FileName); i ++){
		if(FileName[i] == '/'){
			contains_dir = true;
			break;
		}
	}

	if(contains_dir)
	FileName = ComputeThisDir(true, FileName);

	if(strcmp(FileName, "1")== 0)
	return 1;

	if(access(FileName, F_OK)!= 0)
	return 2;

	FILE * fnina = fopen(FileName, "r");

	//File type
	fscanf(fnina, "%d", &argument_number);

	if(argument_number == 1)
	debug_mode = false;

	if(argument_number == 2)
	debug_mode = true;

	//Attribute handler
	char wait_for_start [50];

	fscanf(fnina, "%s\n", wait_for_start);
	while(strcmp(wait_for_start, "START") != 0){
		bool end = false;

		int attrib_num = atoi(wait_for_start), temp = 0;

		if(attrib_num > -2 && attrib_num < 3)
		active_styles[attrib_num] = true;

		//Attrib handler
		switch(attrib_num){
			case -1:
				end = true;
				break;

			case 2:
				fscanf(fnina, "%d", &temp);
				displacement_value = temp;
				break;

			default:
				break;

			if(debug_mode)
			printf("[DEBUG] Got attrib_num %d\n", attrib_num);
		}

		if(end){
			fscanf(fnina, "%s", wait_for_start);
			break;
		}

		memset(wait_for_start, '\0', sizeof(wait_for_start));
		fscanf(fnina, "%s", wait_for_start);
	}

	while(fscanf(fnina, "%d\n", &argument_number)!= EOF){
		switch(argument_number){

			//handling cases without arguments 1st
			case 1:
			case 9:
			case 31:
				if(debug_mode)
				printf("[DEBUG] Keynumber without arguments reached...\n");

				break;

			//wait custom keyword
			case 2: {
				long int long_time = 0;
				char type [4];

				fscanf(fnina, "%ld", &long_time);
				fscanf(fnina, "%s", type);

				if(strcmp(type, "s")== 0)
				long_time *= 1000000;

				else if(strcmp(type, "ms")== 0)
				long_time *= 1000;

				if(debug_mode)
				printf("[DEBUG] Sleeping for %ld microseconds\n", long_time);

				usleep(long_time);

				break;
			}

			//stop custom keyword
			case 3:
				if(debug_mode)
				printf("[DEBUG] Stopping file execution\n");

				fclose(fnina);
				return 0;
				break;

			//repeat custom keyword
			case 4: {
				int repeating_times = 0;
				fscanf(fnina, "%d", &repeating_times);

				node* new_node = malloc(sizeof(node));
				if (new_node == NULL)
				printf("ERR. on repeat, couldnt malloc memory for it\n");

				new_node->PTR_pos = ftell(fnina);
				new_node->repeat_times = repeating_times;
				new_node->next = NULL;

				if (head == NULL)
				head = new_node;

				else {
					node* temp = head;
					while (temp->next != NULL)
					temp = temp->next;

					temp->next = new_node;
				}

				if(debug_mode)
				printf("[DEBUG] Repeating %d times with pointer position on %ld\n", repeating_times, ftell(fnina));

				break;
			}

			//endrepeat custom keyword
			case 5: {
				node* temp = head;
				while (temp != NULL && temp->next != NULL)
				{
					temp = temp->next;
				}

				long int repeater_pos = temp->PTR_pos;
				int repeating_times = temp->repeat_times;

				repeating_times --;

				if(repeating_times > 0)
				fseek(fnina, repeater_pos - ftell(fnina), SEEK_CUR);

				else {
					if(head == NULL) {

					} else if (head->next == NULL){
						free(head);
						head = NULL;

					} else {
						node* temp = head;
						while (temp->next->next != NULL)
						temp = temp->next;

						free(temp->next);
						temp->next = NULL;

					}
				}

				temp->repeat_times = repeating_times;

				if(debug_mode)
				printf("[DEBUG] endrepeat keyword reached, repeat_times is at %d\n", repeating_times);

				break;
			}

			//displacepointer custom keyword
			case 6: {
				fscanf(fnina, "%ld", &displace_ptr_ART);

				if(debug_mode)
				printf("[DEBUG] Set displace_ptr_ART to %ld\n", displace_ptr_ART);

				break;
			}

			//set custom keyword
			case 7: {
				//handler part goes here
				bool formating_font = false, formating_bg = false, formating_color = false;
				int id_gotten = 0;
				long int columns_val = -1, lines_val = -1;
				char type, keyword [4];

				fscanf(fnina, "%c\n", &type);
				fscanf(fnina, "%d\n", &id_gotten);

				switch(type){
					case 'B':
						formating_bg = true;
						break;

					case 'F':
						formating_color = true;
						break;

					case 'S':
						formating_font = true;
						break;
				}

				fscanf(fnina, "%s\n", keyword);

				if(strcmp(keyword, "end")!= 0){
					columns_val = atoi(keyword);
					fscanf(fnina, "%ld\n", &lines_val);
				}

				set_node* new_node = malloc(sizeof(set_node));
				if (new_node == NULL)
				{
					printf("ERR. on set, couldnt malloc memory for it\n");
				}

				new_node->is_font = formating_font;
				new_node->is_color = formating_color;
				new_node->is_bg = formating_bg;
				new_node->what_id = id_gotten;
				new_node->columns = columns_val;
				new_node->lines = lines_val;

				new_node->next = NULL;

				if (global_head == NULL)
				global_head = new_node;

				else {
					set_node* temp = global_head;
					while (temp->next != NULL)
					temp = temp->next;

					temp->next = new_node;
				}

				break;
			}

			//write custom keyword
			case 8:
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				printf("%s", ReadLine);

				if(debug_mode)
				printf("\n[DEBUG] Writing the unformated line on screen...\n");

				break;

			//case 9 is handled up in the begining of the switch statement

			//(Start of TerminalStyle.h function handlers)
			//SlowText handler
			case 10: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				long long int wait_time = 0;
				fscanf(fnina, "%lld\n", &wait_time);

				int catch_return_value = SlowText(ReadLine, wait_time);

				if(debug_mode)
				printf("[DEBUG] SlowText function returned value %d\n", catch_return_value);

				break;
			}

			//ManualDisplacement handler
			case 11: {
				int columns = 0, rows = 0;

				fscanf(fnina, "%d\n", &columns);
				fscanf(fnina, "%d\n", &rows);

				//This one debug call is going up because we dont wanna mess up the format in any way lol
				if(debug_mode)
				printf("[DEBUG] Executing function ManualDisplacement (no return value) with values columns - %d and rows - %d\n", columns, rows);

				ManualDisplacement(columns, rows);

				break;
			}

			//CenterText handler
			case 12: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				int catch_return_value = CenterText(ReadLine);

				if(debug_mode)
				printf("[DEBUG] CenterText function returned value %d\n", catch_return_value);

				break;
			}

			//RightText handler
			case 13: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				int catch_return_value = RightText(ReadLine);

				if(debug_mode)
				printf("[DEBUG] RightText function returned value %d\n", catch_return_value);

				break;
			}

			//RandText handler
			case 14: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				int catch_return_value = RandText(ReadLine);

				if(debug_mode)
				printf("[DEBUG] RandText function returned value %d\n", catch_return_value);

				break;
			}

			//MidScreenText handler
			case 15: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				int catch_return_value = MidScreenText(ReadLine);

				if(debug_mode)
				printf("[DEBUG] MidScreenText function returned value %d\n", catch_return_value);

				break;
			}

			//FormatTextEXT handler
			case 16: {
				char arg1 [2000], arg2 [2000], arg5 [2000];
				bool arg3, arg6;
				int arg4, arg7, arg8, temp;

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';
				strcpy(arg1, ReadLine);

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';
				strcpy(arg2, ReadLine);

				fscanf(fnina, "%d\n", &temp);
				arg3 = temp;
				fscanf(fnina, "%d\n", &arg4);

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';
				strcpy(arg5, ReadLine);

				fscanf(fnina, "%d\n", &temp);
				arg6 = temp;
				fscanf(fnina, "%d\n", &arg7);
				fscanf(fnina, "%d\n", &arg8);

				int catch_return_value = FormatTextEXT(arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);

				if(debug_mode)
				printf("[DEBUG] FormatTextEXT returned value %d\n", catch_return_value);

				break;
			}

			//DisplayTable handler
			case 17: {
				char arg1 [2000], arg2 [2000];

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';
				strcpy(arg1, ReadLine);

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';
				strcpy(arg2, ReadLine);

				int catch_return_value = DisplayTable(arg1, arg2);

				if(debug_mode)
				printf("[DEBUG] DisplayTable returned value %d\n", catch_return_value);

				break;
			}

			//ChangeTerminalColor handler
			case 18: {
				int temp, code;
				bool is_bg;

				fscanf(fnina, "%d\n", &temp);
				is_bg = temp;
				fscanf(fnina, "%d\n", &code);

				int catch_return_value = ChangeTerminalColor(is_bg, code);

				if(debug_mode)
				printf("[DEBUG] ChangeTerminalColor returned value %d\n", catch_return_value);

				break;
			}

			//ChangeTerminalColorEXT handler
			case 19: {
				int temp, code;
				bool is_bg;

				fscanf(fnina, "%d\n", &temp);
				is_bg = temp;
				fscanf(fnina, "%d\n", &code);

				int catch_return_value = ChangeTerminalColorEXT(is_bg, code);

				if(debug_mode)
				printf("[DEBUG] ChangeTerminalColorEXT returned value %d\n", catch_return_value);

				break;
			}

			//ChangeTerminalColorRGB handler
			case 20: {
				int temp, r, g, b;
				bool is_bg;

				fscanf(fnina, "%d\n", &temp);
				is_bg = temp;
				fscanf(fnina, "%d\n", &r);
				fscanf(fnina, "%d\n", &g);
				fscanf(fnina, "%d\n", &b);

				int catch_return_value = ChangeTerminalColorRGB(is_bg, r, g, b);

				if(debug_mode)
				printf("[DEBUG] ChangeTerminalColorRGB returned value %d\n", catch_return_value);

				break;
			}

			//ChangeTerminalFont handler
			case 21: {
				int code;

				fscanf(fnina, "%d\n", &code);

				int catch_return_value = ChangeTerminalFont(code);

				if(debug_mode)
				printf("[DEBUG] ChangeTerminalFont returned value %d\n", catch_return_value);

				break;
			}

			//ChangeTerminalDefenitions handler
			case 22: {
				int code;

				fscanf(fnina, "%d\n", &code);

				int catch_return_value = ChangeTerminalDefenitions(code);

				if(debug_mode)
				printf("[DEBUG] ChangeTerminalDefenitions returned value %d\n", catch_return_value);

				break;
			}

			//ChangeCursorDefenitions handler
			case 23: {
				int code;

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				fscanf(fnina, "%d\n", &code);

				int catch_return_value = ChangeCursorDefenitions(ReadLine, code);

				if(debug_mode)
				printf("[DEBUG] ChangeCursorDefenitions returned value %d\n", catch_return_value);

				break;
			}

			//MoveCursorTo handler
			case 24: {
				int line, column;

				fscanf(fnina, "%d\n", &line);
				fscanf(fnina, "%d\n", &column);

				int catch_return_value = MoveCursorTo(line, column);

				if(debug_mode)
				printf("[DEBUG] MoveCursorTo returned value %d\n", catch_return_value);

				break;
			}

			//EraseTerminalArea handler
			case 25: {
				int code;

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				fscanf(fnina, "%d\n", &code);

				int catch_return_value = EraseTerminalArea(ReadLine, code);

				if(debug_mode)
				printf("[DEBUG] EraseTerminalArea returned value %d\n", catch_return_value);

				break;
			}

			//ChangePrivateMode handler
			case 26: {
				int temp;
				bool trigger;

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				fscanf(fnina, "%d\n", &temp);
				trigger = temp;

				int catch_return_value = ChangePrivateMode(ReadLine, trigger);

				if(debug_mode)
				printf("[DEBUG] ChangePrivateMode returned value %d\n", catch_return_value);

				break;
			}

			//EncryptedWords handler
			case 27: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				char * catch_encryptedword = EncryptedWords(ReadLine);

				if(debug_mode)
				printf("[DEBUG] Executed EncryptedWords (for some reason) (returned %s)\n", catch_encryptedword);

				free(catch_encryptedword);

				break;
			}

			//Art handler
			case 28: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				int catch_return_value = Art(ReadLine);

				if(debug_mode)
				printf("[DEBUG] Art returned value %d\n", catch_return_value);

				break;
			}

			//Animation handler
			case 29: {
				long int wait_time;

				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				fscanf(fnina, "%ld\n", &wait_time);

				int catch_return_value = Animation(ReadLine, wait_time);

				if(debug_mode)
				printf("[DEBUG] Animation returned value %d\n", catch_return_value);

				break;
			}

			//ReadNINAFile handler
			case 30: {
				memset(ReadLine, '\0', sizeof(ReadLine));
				fgets(ReadLine, 2000, fnina);
				ReadLine[strlen(ReadLine) - 1] = '\0';

				int catch_return_value = ReadNINAFile(ReadLine);

				if(debug_mode)
				printf("[DEBUG] ReadNINAFile returned value %d\n", catch_return_value);

				break;
			}

			//Err. reached.
			default:
				return 3;
				break;
		}
	}

	fclose(fnina);

	return 0;
}
