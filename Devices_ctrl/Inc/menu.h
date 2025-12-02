#ifndef INC_MENU_H_
#define INC_MENU_H_

#include "main.h"

// Stany aplikacji
typedef enum {
    STATE_INIT,
    STATE_MENU,
    STATE_STREAMING,
    STATE_ERROR
} AppState_t;

void Menu_InitWrapper(void);
void Menu_Loop(void);

#endif /* INC_MENU_H_ */
