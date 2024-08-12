#include "database.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Database db_create(){
	Database db;
	db.size = 0;
	db.capacity = 4;
	db.records = malloc(db.capacity*sizeof(Record));
	if (db.records == NULL){
		fprintf(stderr, "Memory allocation failed\n");
		exit(1);
	}
	return db;
}

void db_append(Database *db, const Record *item) {
    if (db->size >= db->capacity) {
        // Time to resize our array as it's full
        db->capacity *= 2;
        Record *new_records = realloc(db->records, db->capacity * sizeof(Record));
        if (new_records == NULL) {
	    //handle memory reallocation feature, and prevent memory leak
            free(db->records);
	    fprintf(stderr, "Memory reallocation failed\n");
            return;
        }
        db->records = new_records;
    }
    db->records[db->size] = *item; // Copy the item into the array
    db->size++;
}

Record *db_index(Database *db, int index) {
    if (index < 0 || index >= db->size) {
        // Index is out of bounds
        return NULL;
    }
    return &(db->records[index]);
}


Record *db_lookup(Database *db, const char *handle) {
    for (int i = 0; i < db->size; i++) {
        if (strcmp(db->records[i].handle, handle) == 0) {
            // Found the record with the given handle
            return &(db->records[i]);
        }
    }
    // No record found with the given handle
    return NULL;
}

void db_free(Database *db) {
    free(db->records);
    db->records = NULL;
    db->size = 0;
    db->capacity = 0;
}

Record parse_record(const char *line) {
    Record r;
    char buffer[128]; // Ensure this is large enough to hold the longest possible line
    strncpy(buffer, line, sizeof(buffer));
    buffer[sizeof(buffer) - 1] = '\0';

    // Use strtok to split the string by commas
    char *token = strtok(buffer, ",");
    if (token != NULL) {
        strncpy(r.handle, token, sizeof(r.handle) - 1);
        r.handle[sizeof(r.handle) - 1] = '\0';
    }

    token = strtok(NULL, ",");
    if (token != NULL) {
        r.follower_count = strtoul(token, NULL, 10);
    }

    token = strtok(NULL, ",");
    if (token != NULL) {
        strncpy(r.comment, token, sizeof(r.comment) - 1);
        r.comment[sizeof(r.comment) - 1] = '\0';
    }

    token = strtok(NULL, ",");
    if (token != NULL) {
        r.date_last_modified = strtoul(token, NULL, 10);
    }

    return r;
}

void db_load_csv(Database *db, const char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening file");
        exit(2);
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, file)) != -1) {
        // Remove newline character
        if (line[read - 1] == '\n') line[read - 1] = '\0';

        Record newRecord = parse_record(line);
        db_append(db, &newRecord);
    }

    free(line);
    fclose(file);
}

void db_write_csv(Database *db, const char *path) {
    FILE *file = fopen(path, "w");
    if (file == NULL) {
        perror("Error opening file");
        return;
    }

    for (int i = 0; i < db->size; i++) {
        Record *r = &db->records[i];
        fprintf(file, "%s,%lu,%s,%lu\n",
                r->handle,
                r->follower_count,
                r->comment,
                r->date_last_modified);
    }

    fclose(file);
}


