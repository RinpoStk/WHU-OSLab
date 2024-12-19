//
// Created by Rinpo on 24-12-18.
//

#ifndef _ORANGES_LOG_H
#define _ORANGES_LOG_H

PUBLIC int log_command	(char * cmd);
PUBLIC int syslog_file(int log_buf_flag, const char*fmt, ...);
PUBLIC int filelog(int log_buf_flag, char * file_name);

#endif //_ORANGES_LOG_H
