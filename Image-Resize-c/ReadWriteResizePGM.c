/********************************************************
*Coding project for CS 712 Image Processing
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>

#define max(x, y) ((x>y) ? (x):(y))
#define min(x, y) ((x<y) ? (x):(y))


int xdim;
int ydim;
int newX;
int newY;
int ratio;
int maxraw;
unsigned char *image;
unsigned char *newImage;


void ReadPGM(FILE*);
void WritePGM(FILE*);

int main(int argc, char **argv)
{
  int i, j;
  FILE *fp;

  if (argc != 3){
    printf("Usage: MyProgram <input_ppm> <output_ppm> \n");
    printf("       <input_ppm>: PGM file \n");
    printf("       <output_ppm>: PGM file \n");
    exit(0);
  }


  /* begin reading PGM.... */
  printf("begin reading PGM.... \n");
  if ((fp=fopen(argv[1], "rb"))==NULL){
    printf("read error...\n");
    exit(0);
  }
  ReadPGM(fp);
  
  /*begin resize PGM..*/
  ratio = 2;
  if (ratio>0){
      newY = ydim * ratio;
      newX = xdim * ratio;
  }else{
      newY = ydim/(-ratio);
      newX = xdim/(-ratio);
  }
  newImage = (unsigned char*)malloc(sizeof(unsigned char)*newX*newY);
  //make size smaller, if ratio < 0 then the picture will be resized to 1/(-ratio) times original size
  if (ratio<0){
    for (int j=0;j<newY;j++){
        for (int i=0;i<newX;i++){
            newImage[j*newX+i]=image[j*(-ratio)*xdim+i*(-ratio)];
        }
    }
  }
  //using bi linear interpolation to enlarge picture. the picture will be resized to ratio times original size
  else{
  for (int j=0;j<newY;j++){  //deal with rows that can be floored by ratio first
        if (j%ratio==0){
            for (int i=0;i<newX;i++){
                    int temp = i%ratio;
                    if (temp==0){  //for columns that can be floored by ratio, plug in the original value from Image array
                        newImage[j*newX+i]=image[j/ratio*xdim+i/ratio];
                    }else{         //for columns that are "new", calculate it
                        newImage[j*newX+i]=(double)image[j/ratio*xdim+(i-temp)/ratio]*(temp/ratio)+(double)image[j/ratio*xdim+(i-temp)/ratio+1]*((ratio-temp)/ratio);
                    }
            }
        }
  }
  for (int j=0;j<newY;j++){  //deal with the "new" rows added
    if (j%ratio!=0){
        int temp = j%ratio;
        for (int i=0;i<newX;i++){  //calculate the value from the entered value of above and below, and its position relative to those numbers
            newImage[j*newX+i]=(double)image[(j-temp)/ratio*xdim+i]*(temp/ratio)+(double)image[((j-temp)/ratio+1)*xdim+i]*((ratio-temp)/ratio);
        }
    }
  }
  }
  /* Begin writing PGM.... */
  printf("Begin writing PGM.... \n");
  if ((fp=fopen(argv[2], "wb")) == NULL){
     printf("write pgm error....\n");
     exit(0);
   }
  WritePGM(fp);

  free(image);
  free(newImage);

  return (1);
}



void ReadPGM(FILE* fp)
{
    int c;
    int i,j;
    int val;
    unsigned char *line;
    char buf[1024];


    while ((c=fgetc(fp)) == '#')
        fgets(buf, 1024, fp);
     ungetc(c, fp);
     if (fscanf(fp, "P%d\n", &c) != 1) {
       printf ("read error ....");
       exit(0);
     }
     if (c != 5 && c != 2) {
       printf ("read error ....");
       exit(0);
     }

     if (c==5) {
       while ((c=fgetc(fp)) == '#')
         fgets(buf, 1024, fp);
       ungetc(c, fp);
       if (fscanf(fp, "%d%d%d",&xdim, &ydim, &maxraw) != 3) {
         printf("failed to read width/height/max\n");
         exit(0);
       }
       printf("Width=%d, Height=%d \nMaximum=%d\n",xdim,ydim,maxraw);

       image = (unsigned char*)malloc(sizeof(unsigned char)*xdim*ydim);
       getc(fp);

       line = (unsigned char *)malloc(sizeof(unsigned char)*xdim);
       for (j=0; j<ydim; j++) {
          fread(line, 1, xdim, fp);
          for (i=0; i<xdim; i++) {
            image[j*xdim+i] = line[i];
         }
       }
       free(line);

     }

     else if (c==2) {
       while ((c=fgetc(fp)) == '#')
         fgets(buf, 1024, fp);
       ungetc(c, fp);
       if (fscanf(fp, "%d%d%d", &xdim, &ydim, &maxraw) != 3) {
         printf("failed to read width/height/max\n");
         exit(0);
       }
       printf("Width=%d, Height=%d \nMaximum=%d,\n",xdim,ydim,maxraw);

       image = (unsigned char*)malloc(sizeof(unsigned char)*xdim*ydim);
       getc(fp);

       for (j=0; j<ydim; j++)
         for (i=0; i<xdim; i++) {
            fscanf(fp, "%d", &val);
            image[j*xdim+i] = val;
         }

     }

     fclose(fp);
}


void WritePGM(FILE* fp)
{
  int i,j;

  fprintf(fp, "P5\n%d %d\n%d\n", newX, newY, 255);
  for (j=0; j<newY; j++)
    for (i=0; i<newX; i++) {
      fputc(newImage[j*newX+i], fp);
    }

  fclose(fp);

}
