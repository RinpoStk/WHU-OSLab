//
// Created by rinpostk on 24-12-17.
//

#ifndef _ORANGES_RANDOM_H
#define _ORANGES_RANDOM_H

#define LCG_A   1103515245
#define LCG_C   12345
#define LCG_M   (1 << 31)

PUBLIC void srand(int seed);
PUBLIC int rand(void);


#endif //_ORANGES_RANDOM_H
