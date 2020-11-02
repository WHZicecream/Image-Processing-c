/********************************************************
***IMPORTANT NOTE***
If you have problems with the provided sample code,
part of the reason might be due to the function "fopen".
Please try changing "r/w" to "rb/wb" or the other way
when you use this function.
*********************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <complex.h>
#define PI 3.14159265
#define max(x, y) ((x>y) ? (x):(y))
#define min(x, y) ((x<y) ? (x):(y))


int xdim;
int ydim;
int maxraw;
int padx;
int pady;
unsigned char *image;
unsigned char *newImage;
unsigned char *Preal;
unsigned char *Pimg;
unsigned char *out;

void ReadPGM(FILE*);
void WritePGM(FILE*);
void Padding();
void DFT();
void idealLPF(double);
void inverseDFT();


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

  padx = xdim* 2 - 1;
  pady = ydim* 2 - 1;
  Padding();
  DFT();
  //idealLPF(15.0);
  //inverseDFT();


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


  fprintf(fp, "P5\n%d %d\n%d\n", padx, pady, 255);
  for (j=0; j<pady; j++)
    for (i=0; i<padx; i++) {
      fputc(newImage[j*padx+i], fp);
    }

  fclose(fp);

}

void Padding()
{
    int i, j;
    newImage = (unsigned char*)malloc(sizeof(unsigned char)*padx*pady);
    for (i=0;i<pady;i++){
        for (j=0;j<padx;j++){
            if (j>xdim||i>ydim){
                newImage[i*padx+j] = 0.0;
            } else{
                newImage[i*padx+j] = image[i*xdim+j];
            }
        }
    }
    for (i=0;i<pady;i++){
        for (j=0;j<padx;j++){
            newImage[i*padx+j] = newImage[i*padx+j] * pow(-1,(j+i));
        }
    }
}

void DFT()
{
    Preal = (unsigned char*)malloc(sizeof(unsigned char)*padx*pady);
    Pimg = (unsigned char*)malloc(sizeof(unsigned char)*padx*pady);
    out = (unsigned char*)malloc(sizeof(unsigned char)*padx*pady);

    for (int y=0;y<pady;y++){
        for(int u=0;u<padx;u++){
            Preal[padx*y+u] = 0.0;
            Pimg[padx*y+u] = 0.0;
            //out[padx*y+u] = 0.0;
            for (int x=0;x<padx;x++){
                //out[padx*y+u] += newImage[padx*y+u]*cexp(-((2*PI*u*x)/padx)*_Complex_I);
                Preal[padx*y+u] += newImage[padx*y+x]*cos((2*PI*u*x)/padx);
                Pimg[padx*y+u] += (-newImage[padx*y+x])*sin((2*PI*u*x)/padx)*_Complex_I;
            }
            newImage[padx*y+u] = Preal[padx*y+u]+Pimg[padx*y+u];
        }
    }

    for (int x=0;x<padx;x++){
        for(int v=0;v<pady;v++){
            Preal[padx*v+x] = 0.0;
            Pimg[padx*v+x] = 0.0;
            for (int y=0;y<pady;y++){
                //newImage[padx*v+x] += out[padx*v+x]*cexp(-((2*PI*v*y)/pady)*_Complex_I);
                Preal[padx*v+x] += newImage[padx*y+x]*cos((2*PI*v*y)/pady);
                Pimg[padx*v+x] += (-newImage[padx*y+x])*sin((2*PI*v*y)/pady)*_Complex_I;
            }
            newImage[padx*v+x] = Preal[padx*v+x]+Pimg[padx*v+x];
        }
    }

    /*for (int y=0;y<pady;y++){
        for(int x=0;x<padx;x++){
            newImage[padx*y+x] = sqrt(pow(Preal[padx*v+x],2)+pow(Pimg[padx*v+x],2));
        }
    }*/
}

void idealLPF(double cutOff)
{
    double D;
    double Do = 5;
    int i, j;
    for (j=0; j<pady; j++)
        for (i=0; i<padx; i++) {
            D = sqrt(pow((i-padx/2),2)+pow((j-pady/2),2));
            if (D <= Do){
                newImage[padx*j+i] = newImage[padx*j+i];
            } else {
                newImage[padx*j+i] = 0;
            }
        }
}

void inverseDFT()
{
        Preal = (unsigned char*)malloc(sizeof(unsigned char)*padx*pady);
    Pimg = (unsigned char*)malloc(sizeof(unsigned char)*padx*pady);

    for (int y=0;y<pady;y++){
        for(int u=0;u<padx;u++){
            Preal[padx*y+u] = 0.0;
            Pimg[padx*y+u] = 0.0;
            for (int x=0;x<padx;x++){
                Preal[padx*y+u] += newImage[padx*y+x]*cos((2*PI*u*x)/padx);
                Pimg[padx*y+u] += newImage[padx*y+x]*sin((2*PI*u*x)/padx)*_Complex_I;
                newImage[padx*y+u] = Preal[padx*y+u]+Pimg[padx*y+u];
            }
        }
    }

    for (int x=0;x<padx;x++){
        for(int v=0;v<pady;v++){
            Preal[padx*v+x] = 0.0;
            Pimg[padx*v+x] = 0.0;
            for (int y=0;y<pady;y++){
                Preal[padx*v+x] += newImage[padx*y+x]*cos((2*PI*v*y)/padx);
                Pimg[padx*v+x] += newImage[padx*y+x]*sin((2*PI*v*y)/padx)*_Complex_I;
                newImage[padx*v+x] = Preal[padx*v+x]+Pimg[padx*v+x];
            }
        }
    }

    for (int y=0;y<pady;y++){
        for(int x=0;x<padx;x++){
            newImage[padx*y+x] = newImage[padx*y+x]/(padx*pady);
        }
    }
}
