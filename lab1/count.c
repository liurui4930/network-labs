#include <stdio.h>
#include <stdlib.h>
#include <string.h> 

int main(int argc, char **args) {
    //Missing input
    if(argc!=4){
	printf("Usage: ./count <input-filename> <search-string> <output-filename>\n");
	return -1;
    }
    FILE* fi; 
    fi = fopen(args[1],"r");

    //Fail to open input file
    if(fi==NULL){
	printf("No such file found: please type in exsiting files\n");
	return -1;
    }

    //Get file size
    fseek(fi, 0, SEEK_END);	//Seek to end of file
    long int sz = ftell(fi);    //Get current pointer
    fseek(fi, 0, SEEK_SET);     //Reset the file pointer to the beginning

    //Find matches	
    int len=strlen(args[2]);
    char c;
    int cnt=0;
    int i=0;  			//Initialize index to 0	
    while((c=fgetc(fi))!=EOF){  //Read input file char by char
	if(c==args[2][i]){	
	    i++;		    	       	
	    if(i==len){         //index == Length of Search String : A match found
		cnt++;	
		fseek(fi,-i+1,SEEK_CUR);//Set search pointer to the second char of the match
		i=0;    
	    }
	}
	else{		        //Not a match,reset the index to 0
	    fseek(fi,-i,SEEK_CUR);   //Move back the file pointer
	    i=0;
        }		
    }
    fclose(fi);
	
    //Output to outputfile
    FILE* fo;
    fo=fopen(args[3],"w"); 
    if(fo==NULL){
	printf("Fail to creat output file\n");
	return -1;
    }	
    fprintf(fo,"Size of file = %d\n",sz);
    fprintf(fo,"Number of matches = %d\n", cnt);
    fclose(fo);
    return 0;
}

