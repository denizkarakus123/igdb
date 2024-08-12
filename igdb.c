#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>
#include "database.h"



void print_record(const Record *r){
	char formatted_time[20];

    	time_t last_modified_time = (time_t)r->date_last_modified;

    	struct tm *local_time = localtime(&last_modified_time);

    	strftime(formatted_time, sizeof(formatted_time), "%Y-%m-%d %H:%M", local_time);

	printf("%-20.20s | %-10lu | %-20s | %-30.30s\n", 
	r->handle, r->follower_count, formatted_time, r->comment);
}

void list_records(Database *db){
	printf("| %-18s | %-10s | %-20s | %s\n",
           "HANDLE", "FOLLOWERS", "LAST MODIFIED", "COMMENT");
    	printf("|--------------------|------------|----------------------|------------------------|\n");
	for (int i = 0; i<db->size; i++){
		print_record(&db->records[i]);
	}

}

void add_record(Database *db, const char *handle, unsigned long followers){
	if (db_lookup(db, handle) != NULL){
		printf("Error: handle %s already exists.\n", handle);
		return;
	}

	if (strlen(handle) > HANDLE_MAX_LENGTH){
		fprintf(stderr, "Error: Handle too long\n");
		return;
	}

	char comment[COMMENT_MAX_LENGTH];
	printf("Comment ");
	scanf(" %[^\n]%*c", comment); //this reads until a new line is encountered
	

	if (strlen(comment) > COMMENT_MAX_LENGTH){
		fprintf(stderr,"Error: comment too long\n");
		return;
	}
	if(strchr(comment, ',') != NULL){
		printf("Error: comment cannot contain commas.\n");
		return;
	}

	


	Record newRecord;
	strncpy(newRecord.handle, handle, HANDLE_MAX_LENGTH-1);
	newRecord.handle[HANDLE_MAX_LENGTH-1] = '\0';
	newRecord.follower_count = followers;
	newRecord.date_last_modified = time(NULL);
	strncpy(newRecord.comment, comment, COMMENT_MAX_LENGTH-1);
	newRecord.comment[COMMENT_MAX_LENGTH-1] = '\0';

	db_append(db, &newRecord);
}

void update_record(Database *db, const char *handle, unsigned long followers){
	if (db_lookup(db,handle) == NULL){
		printf("Error: no entry with handle %s.\n", handle);
		return;
	}

	if (strlen(handle) > HANDLE_MAX_LENGTH){
                fprintf(stderr, "Error: Handle too long\n");
        }

	char comment[COMMENT_MAX_LENGTH];
        printf("Comment ");
        scanf(" %[^\n]%*c", comment);

	if (strlen(comment) > COMMENT_MAX_LENGTH){
                fprintf(stderr,"Error: comment too long\n");
        }

	if(strchr(comment, ',') != NULL){
		printf("Error: comment cannot contain commas.\n");
		return;
	}

	Record *record_update = db_lookup(db,handle);
	record_update->follower_count = followers;
	record_update->date_last_modified = time(NULL);
	strncpy(record_update->comment, comment, COMMENT_MAX_LENGTH-1);
	record_update->comment[COMMENT_MAX_LENGTH-1] = '\0';
}

void save(Database *db, const char *path){
	db_write_csv(db, path);
}



int main_loop(Database * db)
{
	char *line = NULL;
    	size_t len = 0;
    	ssize_t read;
    	char *command, *handle, *followers_str;
	bool is_running = true;
	bool is_saved = true;	

	printf("loaded %d records\n", db->size);

	while(is_running){
		fflush(stdout);  // Make sure the prompt is shown immediately

        	read = getline(&line, &len, stdin);  // Read the entire line
        	if (read == -1) {
            		fprintf(stderr, "Error reading input.\n");
            		break;  // Exit loop on read error or EOF
        	}

        	if (line[read - 1] == '\n') {
            		line[read - 1] = '\0';  // Remove the newline character
        	}	

        	command = strtok(line, " ");	

		if (command != NULL && strcmp(command, "list") == 0){
			if (strtok(NULL, " ") != NULL){
				fprintf(stderr, "Error: wrong number of arguments\n");
			}
			else{
				list_records(db);
			}
		}
		
		else if(command != NULL && strcmp(command, "add") == 0){
			handle = strtok(NULL, " ");
			followers_str = strtok(NULL, " ");
			if (handle && followers_str && strtok(NULL, " ") == NULL) {
				if(handle[0] == '@' && strlen(handle) > 1) {
					char *endptr;
            				unsigned long followers = strtoul(followers_str, &endptr, 10);
					if (*endptr == '\0'){
						add_record(db, handle, followers);
						is_saved = false;
					}
					else{
						printf("Error: Followers must be an integer.\n");
					}
				}
				else{
					 if (handle[0] != '@'){
					 	 printf("Error: Handle must start with '@'.\n");
					 }
					 if (strlen(handle) <= 1){
					 	 printf("Error: Handle must be longer than one character.\n");
					 }
				}
			}
			else{
				printf("Error: Wrong number of arguments for 'add' command.\n");
			}	
		}

		else if(command != NULL && strcmp(command, "update") == 0){
			handle = strtok(NULL, " ");
                        followers_str = strtok(NULL, " ");
                        if (handle && followers_str && strtok(NULL, " ") == NULL) {
				char *endptr;
                                unsigned long followers = strtoul(followers_str, &endptr, 10);
                                if (*endptr == '\0'){
                                        update_record(db, handle, followers);
                                        is_saved = false;
                                }
			}
			else{
				printf("Error: Wrong number of arguments for update command\n");
			}
		}
		else if(command != NULL && strcmp(command, "save") == 0){
			
			if (strtok(NULL, " ") != NULL){
                                fprintf(stderr, "Error: wrong number of arguments\n");
                        }
			else{
				save(db, "database.csv");
				is_saved = true;
			}
		}
		else if(command != NULL && strcmp(command, "exit") == 0){
			char *next_token = strtok(NULL, " ");
			if (next_token == NULL){
				 if (!is_saved) {
            				printf("Error: you did not save your changes. Use 'exit fr' to force exiting anyway.\n");
        		}
				 else{
					 db_free(db);
					 is_running = false;
				}
			}	 
			else if (strcmp(next_token, "fr") == 0 && strtok(NULL, " ") == NULL){
				db_free(db);
				is_running = false;
			}
			else{
				printf("Error: unknown command\n");
			}
		}
		
		
		else{
			fprintf(stderr, "Error: wrong command\n");
		}
		free(line);
		line = NULL;	
	}
	free(line);
	return 0;
}

int main()
{
    Database db = db_create();
    db_load_csv(&db, "database.csv");
    return main_loop(&db);
}
