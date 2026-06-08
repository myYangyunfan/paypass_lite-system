#include "stack.h"
#include <string.h>

void stack_init(UndoStack *s) {
    s->top = -1;
}

int stack_push(UndoStack *s, UndoRecord *rec) {
    if (stack_full(s)) return 0;
    s->top++;
    s->data[s->top] = *rec;
    return 1;
}

int stack_pop(UndoStack *s, UndoRecord *rec) {
    if (stack_empty(s)) return 0;
    *rec = s->data[s->top];
    s->top--;
    return 1;
}

int stack_empty(UndoStack *s) {
    return s->top < 0;
}

int stack_full(UndoStack *s) {
    return s->top >= MAX_UNDO - 1;
}

int stack_size(UndoStack *s) {
    return s->top + 1;
}
