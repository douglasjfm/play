#ifndef LIB_INCLUDED
#define LIB_INCLUDED

#define PORTA_RTCP_ENVR_SRC 10000
#define PORTA_RTCP_REC_SINK 10001

int cam ();
int playcam();
int pausecam();
void gravar();
void send(char *ip);
int endcam();
void stopgravar();


gulong video_area_xid;

#endif // LIB_INCLUDED
