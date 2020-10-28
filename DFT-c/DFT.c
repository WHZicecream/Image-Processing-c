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

#define max(x, y) ((x>y) ? (x):(y))
#define min(x, y) ((x<y) ? (x):(y))


int xdim;
int ydim;
int maxraw;
int padx;
int pady;
unsigned char *image;
unsigned char *newImage;

void ReadPGM(FILE*);
void WritePGM(FILE*);
void Padding();
void DFT();
void idealLPF();


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

  // your application here
  Padding();
  DFT();

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
    padx = xdim * 2 - 1;
    pady = ydim * 2 - 1;
    int i, j;
    newImage = (unsigned char*)malloc(sizeof(unsigned char)*padx*pady);
    for (i=0;i<pady;i++){
        for (j=0;j<padx;j++){
            if (j>xdim||i>ydim){
                newImage[i*padx+j] = 0;
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
    double Preal[padx*pady];
    double Pimg[padx*pady];

    for (int y=0;y<pady;y++){
        for(int u=0;u<padx;u++){
            Preal[padx*y+u] = 0;
            Pimg[padx*y+u] = 0;
            for (int x=0;x<padx;x++){
                Preal[padx*y+u] += newImage[padx*y+x]*cos((2*M_PI*u*x)/padx);
                Pimg[padx*y+u] += -newImage[padx*y+x]*sin((2*M_PI*u*x)/padx);
            }
        }
    }
    for (int y=0;y<pady;y++){
        for(int u=0;u<padx;u++){
            for (int x=0;x<padx;x++){
                newImage[padx*y+u] += Preal[padx*y+x]*cos((2*M_PI*u*x)/pady);
                newImage[padx*y+u] += -Pimg[padx*y+x]*sin((2*M_PI*u*x)/pady);
            }
        }
    }
}

void idealLPF()
{
    ;
}
