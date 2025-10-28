#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Allocates a new empty list
list_t* list_alloc() {
    list_t* list = (list_t*)malloc(sizeof(list_t));
    list->head = NULL;
    list->length = 0;
    return list;
}

// Frees the memory used by the list
void list_free(list_t* list) {
    node_t* current = list->head;
    while (current) {
        node_t* temp = current;
        current = current->next;
        free(temp);
    }
    free(list);
}

// Returns the length of the list
int list_length(list_t* list) {
    return list->length;
}

// Adds a value to the front of the list
void list_add_to_front(list_t* list, int value) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->value = value;
    new_node->next = list->head;
    list->head = new_node;
    list->length++;
}

// Adds a value to the back of the list
void list_add_to_back(list_t* list, int value) {
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->value = value;
    new_node->next = NULL;
    if (!list->head) {
        list->head = new_node;
    } else {
        node_t* current = list->head;
        while (current->next) {
            current = current->next;
        }
        current->next = new_node;
    }
    list->length++;
}

// Adds a value at a specific index
void list_add_at_index(list_t* list, int index, int value) {
    if (index < 0 || index > list->length) return;
    if (index == 0) {
        list_add_to_front(list, value);
        return;
    }
    node_t* new_node = (node_t*)malloc(sizeof(node_t));
    new_node->value = value;
    node_t* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }
    new_node->next = current->next;
    current->next = new_node;
    list->length++;
}

// Removes a value from the back of the list
int list_remove_from_back(list_t* list) {
    if (!list->head) return -1; // Empty list
    if (!list->head->next) { // Single element
        int value = list->head->value;
        free(list->head);
        list->head = NULL;
        list->length--;
        return value;
    }
    node_t* current = list->head;
    while (current->next->next) {
        current = current->next;
    }
    int value = current->next->value;
    free(current->next);
    current->next = NULL;
    list->length--;
    return value;
}

// Removes a value from the front of the list
int list_remove_from_front(list_t* list) {
    if (!list->head) return -1; // Empty list
    node_t* temp = list->head;
    int value = temp->value;
    list->head = list->head->next;
    free(temp);
    list->length--;
    return value;
}

// Removes a value at a specific index
int list_remove_at_index(list_t* list, int index) {
    if (index < 0 || index >= list->length) return -1; // Out of bounds
    if (index == 0) return list_remove_from_front(list);
    node_t* current = list->head;
    for (int i = 0; i < index - 1; i++) {
        current = current->next;
    }
    node_t* temp = current->next;
    int value = temp->value;
    current->next = temp->next;
    free(temp);
    list->length--;
    return value;
}

// Gets the value at a specific index
int list_get_elem_at(list_t* list, int index) {
    if (index < 0 || index >= list->length) return -1; // Out of bounds
    node_t* current = list->head;
    for (int i = 0; i < index; i++) {
        current = current->next;
    }
    return current->value;
}

// Converts the list to a string
char* listToString(list_t* list) {
    if (!list->head) return strdup("[]");
    char* result = (char*)malloc(1024);
    strcpy(result, "[");
    node_t* current = list->head;
    while (current) {
        char buffer[16];
        sprintf(buffer, "%d", current->value);
        strcat(result, buffer);
        if (current->next) strcat(result, ", ");
        current = current->next;
    }
    strcat(result, "]");
    return result;
}
