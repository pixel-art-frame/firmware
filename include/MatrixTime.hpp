#ifndef _MATRIX_TIME_
#define _MATRIX_TIME_

unsigned long lastTimeShow = 0;

void setupNTPClient();
void updateTime();
void handleTime();

#endif