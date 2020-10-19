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
unsigned char *image;
unsigned char *newImage;
double gaussianMask[25];
double bilateralMask[25];


void ReadPGM(FILE*);
void WritePGM(FILE*);
void GaussianFilter(int, double);
double GaussianLoop(int,int,int);
double test(int);
void BilateralFilter(int,double,double);
double BilateralLoop(int,int,int,double,double);


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
  //GaussianFilter(5,0.84);
  BilateralFilter(5,2,0.2);

  /* Begin writing PGM.... */
  printf("Begin writing PGM.... \n");
  if ((fp=fopen(argv[2], "wb")) == NULL){
     printf("write pgm error....\n");
     exit(0);
   }
  WritePGM(fp);

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


  fprintf(fp, "P5\n%d %d\n%d\n", xdim, ydim, 255);
  for (j=0; j<ydim; j++)
    for (i=0; i<xdim; i++) {
      fputc(newImage[j*xdim+i], fp);
    }

  fclose(fp);

}

void GaussianFilter(int gaussDim, double sigma)
{
    int i,j,curr,starting;
    double gsum=0.0;
    newImage = (unsigned char*)malloc(sizeof(unsigned char)*xdim*ydim);
    for(i=0;i<gaussDim;i++){
        for(j=0;j<gaussDim;j++){
            double x=i-(gaussDim-1)/2.0;
            double y=j-(gaussDim-1)/2.0;
            double result=(1/(2*M_PI*pow(sigma,2)))*exp(((pow((x),2)+pow((y),2))/((2*pow(sigma,2))))*(-1));
            gaussianMask[i*gaussDim+j]=result;
            gsum+=gaussianMask[i*gaussDim+j];
        }
    }
    for (i=0;i<gaussDim;i++){
        for (j=0;j<gaussDim;j++){
            gaussianMask[i*gaussDim+j]/=gsum;
        }
    }

    for (i=0;i<gaussDim;i++){
            for (j=0;j<gaussDim;j++){
                printf("%f ", gaussianMask[i*gaussDim+j]);
            }
            printf("\n");
    }
    for (i=0;i<ydim;i++){
        for(j=0;j<xdim;j++){
            newImage[i*xdim+j]=GaussianLoop(j,i,gaussDim);
        }
    }
}

double GaussianLoop(int currx,int curry,int gaussDim){
    int i,j;
    int k=0;
    double sum=0.0;
    double image2D[ydim][xdim];
    for (i=0;i<ydim;i++){
        for(j=0;j<xdim;j++) {
            image2D[i][j]=image[i*xdim+j];
        }
    }
    int startx=currx-gaussDim/2;
    int starty=curry-gaussDim/2;
    for (i=starty;i<starty+gaussDim;i++){
        for (j=startx;j<startx+gaussDim;j++){
            if (i>=0&&j>=0){
                sum+=image2D[i][j]*gaussianMask[k];
            }
            k++;
        }
    }
    return sum;
}

void BilateralFilter(int bilateralDim,double sigmaS, double sigmaR)
{
    int i,j;
    newImage = (unsigned char*)malloc(sizeof(unsigned char)*xdim*ydim);
    //use bilateralLoop to calculate all the pixels and store into newImage
    for (i=0;i<ydim;i++){
        for(j=0;j<xdim;j++){
            newImage[i*xdim+j]=BilateralLoop(j,i,bilateralDim,sigmaS,sigmaR);
        }
    }
}

double BilateralLoop(int currx, int curry, int bilateralDim, double sigmaS, double sigmaR)
{
    int i,j;
    int k=0;
    double bsum=0.0;
    double sum=0.0;
    double image2D[ydim][xdim];
    //forming the bilateral mask for this pixel
    for(i=0;i<bilateralDim;i++){
        for (j=0;j<bilateralDim;j++){
            double x=i-(bilateralDim-1)/2.0;
            double y=j-(bilateralDim-1)/2.0;
            //double result=(1/(2*M_PI*pow(sigmaS,2)))*exp(((pow((x),2)+pow((y),2))/((2*pow(sigmaS,2))))*(-1));
            double result = exp((-1)*pow((y*bilateralDim+x-curry*xdim-currx),2)/(2*pow(sigmaS,2)))*exp((-1)*pow((y*bilateralDim+x-curry*xdim-currx),2)/(2*pow(sigmaR,2)));
            bilateralMask[i*bilateralDim+j]=result;
            printf("%f ", result);
            bsum+=bilateralMask[i*bilateralDim+j];
        }
    }
    for (i=0;i<bilateralDim;i++){
        for (j=0;j<bilateralDim;j++){
            bilateralMask[i*bilateralDim+j]/=bsum;
        }
    }

    //reconstruct the array to 2d
    for (i=0;i<ydim;i++){
        for(j=0;j<xdim;j++) {
            image2D[i][j]=image[i*xdim+j];
        }
    }
    int startx=currx-bilateralDim/2;
    int starty=curry-bilateralDim/2;
    //use the 2D array and the mask to calculate the value on this pixel
    for (i=starty;i<starty+bilateralDim;i++){
        for (j=startx;j<startx+bilateralDim;j++){
            if (i>=0&&j>=0){
                sum+=image2D[i][j]*bilateralMask[k];
            }
            k++;
        }
    }
    return sum;
}
