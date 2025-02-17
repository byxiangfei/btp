/*
Written By: Jitendra Prasad Keer
BTech, CSE, IIT Mandi
 */

#include "../include/preprocessing.h"

#include "initialization.cpp"

int main(int argc, char *argv[]){
  if(argc < 3){
    fprintf(stderr, "Usage: <binary> <path to feature file(HTK format)>  <path to .scp file>\n");
    exit(0);
  }
  // Variable declaration
  char                                 buffer[512];
  FILE                                 *feat_file, *scp_file;
  int                                  i = 0, j= 0, count = 0, TotalFeatures = 0,ret=0;
  char                                 line[512]; //to store the whole line    
  int                                  *flag;

  VECTOR_OF_F_VECTORS                  *features; //check memory access (need to allocate more space)
  F_VECTOR                             *feat;
  
  int                                  States = INITIAL_STATES;
  int                                  *numStates = &States;
  //============================================================================
  //**************Allocating Memory**************
  flag = (int *)calloc(MAX_NUM_FEATURES, sizeof(int ));
  
  //  features = (VECTOR_OF_F_VECTORS *)calloc(MAX_NUM_FEATURES, sizeof());
  features = new F_VECTOR *[MAX_NUM_FEATURES];
  feat = new F_VECTOR(DIM);  
  for(i = 0; i < MAX_NUM_FEATURES; i++){
    //features[i] = (F_VECTOR *)AllocFVector(DIM);
    features[i] = new F_VECTOR(DIM);
  }
  
  featureSpace = (int **)calloc(MAX_NUM_FEATURES, sizeof(int *));
  for(i = 0; i < MAX_NUM_FEATURES; i++)
    featureSpace[i] = (int *)calloc(3, sizeof(int ));
  //============================================================================
  //open feature file(HTK format) and convert to ASCII and write it  
  snprintf(buffer, sizeof(buffer), "/usr/local/bin/HList -r %s > feature_file_all.txt", argv[1]);
  if(system(buffer) < 0){
    fprintf(stderr, "unable to open HTK binary file: HList or paths to files are wrong\nExiting\n");
    exit(0);
  }
  // open feature file
  feat_file = fopen("feature_file_all.txt", "r");
  if(feat_file == NULL){
    fprintf(stderr, "unable to open feature_file_all.txt\n");
    exit(0);
  }
  // open scp file
  scp_file = fopen(argv[2], "r");
  if(scp_file == NULL){
    fprintf(stderr, "unable to open scp file\n");
    exit(0);
  }
  
  while((ret = fscanf(scp_file, "%s", line)) == 1){
    //printf("%s\n", line);
    if(ret == EOF)
      break;  
    char start[20], end[20];
    i = 0;
    while(line[i] != '_')
      i++;
    for(j = 0,i++; line[i] != '_'; j++,i++)
      start[j] = line[i];
    start[j] = '\0';
    for(j=0, i++; line[i] != '='; i++, j++)
      end[j] = line[i];
    end[j] = '\0';
    int s , e;
    s = atoi(start);
    e = atoi(end);
    //printf("%d  %d \n", s, e);
    for(i = s; i <= e; i++)
      flag[i] = 1;        
  }
  
  //process feature file and drop all unvoiced frames         
  if(!features || !feat){
    fprintf(stderr, "unable to allocate memory\n");
    exit(0);
  }
  
  count = 0; //count actual feature vectors
  i = -1; // count file feature vectors
  while(!feof(feat_file) && !ferror(feat_file)){
    i++;
    float number = 0;
    for(j = 0; j < DIM; j++){
      fscanf(feat_file, "%f", &number);
      feat->array[j] = number;
    }
    //check if i is voiced feature vector or not
    if(flag[i] == 0){
      //unvoiced frame
      featureSpace[i][0] = 0;//counter
      featureSpace[i][1] = -1;//counter in voiced features
      featureSpace[i][2] = -1;//speaker number
      continue;
    }
    else{
      //this feature vector is voiced 
      featureSpace[i][0] = 1;
      featureSpace[i][1] = count;
      for(j = 0; j < DIM; j++)
	features[count]->array[j] = feat->array[j];
      features[count]->numElements = DIM;
      count++;
      //      printf("%d  ", i);
    }
  }
  TotalFeatures = count;
  allFeaturesCount = i;
  printf("TotalFeatures: %d  allFeaturesCount: %d\n", TotalFeatures, allFeaturesCount);
  /* for(i = 0; i < count; i++)
     printf("%d  ", flag[i]);
  
     for(i = 0; i < TotalFeatures; i++){
     for(j = 0;j < DIM; j++)
     printf("%f ", features[i]->array[j]);
     printf("\n");
     }
  */
  fclose(feat_file);
  fclose(scp_file);  
  // EXTRACT FILE NAME
  char path[512];
  for(i = 0; argv[1][i] != '\0'; i++){
    path[i] = argv[1][i];
  }
  path[i] = '\0';  
  int len = strlen(path);
  printf("path: %s  l: %d\n", path, len);
  int start = 0, end = 0;
  for(i = len-1; i >= 0; i--){
    if(path[i] == '.'){
      end = i;
    }
    if(path[i] == '/'){
      start = i;
      break;
    }
  }
  // printf("start: %d  end: %d\n", start, end);
  for(i = start+1; i < end; i++){
    fileName[i-start-1] = path[i];
  }
  fileName[i] = '\0';
  printf("file name: %s \n", fileName);
  //Uniform initialization all Gaussian mixtures 
  InitializeGMMs(features, DIM, TotalFeatures, numStates);
  printf("exiting from main....\n");
  return 0;
}

/***************************************************************************************/
void printHMM(ESHMM *mdHMM){
  //print means
  int states = mdHMM->hmmStates;
  printf("no of states: %d \n", states);
  int i = 0, j = 0, k = 0;
  //print means
  for(i = 0; i < states; i++){
    Mean = mdHMM->HMMstates[i].Mean(); // Obtain Mean Vector by calling object method
    Mean.print("printing Mean:\n");
    printf("\n");
  }
  //print Covariace Matrix
  for(i = 0; i < states; i++){
    Cov = mdHMM->HMMstates[i].Covariance(); // Obtain Covariance Matrix by calling object method
    Cov.print("Covariance Matrix:\n");
    printf("\n");
  }
  //print variance (using diagonal covariance matrix
  // print transition matrix
  // printf("\ntransition matrix:\n");
  // for(i = 0; i < states * MIN_DUR; i++){
  //   for(j = 0; j < states * MIN_DUR; j++)
  //     if(mdHMM->trans[i]->array[j] > 0.001)
  // 	printf("%f ", mdHMM->trans[i]->array[j]);
  //   printf("\n");
  // }
  //print prior prob
  // printf("\nprior prob:\n");
  // for(i = 0; i < states * MIN_DUR; i++){
  //   printf("%f ", mdHMM->prior->array[i]);
  // }
  printf("\n");
}
/******************************************************************************/

