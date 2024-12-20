//
// Created by rinpostk on 24-12-18.
//

#ifndef _ORANGES_LOG_H
#define _ORANGES_LOG_H

#include "type.h"

PUBLIC int syslog(const char *fmt, ...);
PUBLIC int log_command(char * cmd);
PUBLIC void custom_log(char * log);

#endif //_ORANGES_LOG_H
