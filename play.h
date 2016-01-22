#ifndef PLAY_H_INCLUDED
#define PLAY_H_INCLUDED

int endcam();
int pausecam();
int playcam();
void stopgravar();
void gravar();
int cam ();

void setprogresso(guint pos, guint len);

int mp3 (char *musica);
int pausepipe();
int playpipe();
int endpipe();

#endif // PLAY_H_INCLUDED
