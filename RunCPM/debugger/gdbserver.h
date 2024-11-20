
#ifndef GDBSERVER_H
#define GDBSERVER_H

void gdbserver_init();
int gdbserver_start( int port );
void gdbserver_stop();
int gdbserver_activate();

//void gdbserver_refresh_status();

#endif				/* #ifndef GDBSERVER_H */
