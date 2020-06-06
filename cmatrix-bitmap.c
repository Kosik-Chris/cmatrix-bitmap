/*
 * cmatrix-bitmap 
 * see readme.md or /docs for information.
 * 
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define PNG_HEADER_SIZE 8
void readFile(const char*);

/*
  We will hold char (byte) ptr to array of values.
  We will avoid the problem of int being 32bit or 64 bit depending on env.
  Dynamically allocate based on defined size of chunk
*/
//TODO: Fix the chunk_t structure to hold char arrays. oofta
typedef struct chunk_t{
  char* length; //chunk length 4 bytes
  char* type;   //chunk type 4 bytes
  char * data;  //chunk data 
  char * crc;   //chunk CRC
} Chunk;



/*
 * Entry point of test application
 * Receive number of arguments passed cli, ptr to array of arguments
 */  
int main(int argc, char* argv[]){

    const char * filename;
    if(argc >= 2){
      //let the second argument passed be the filename
      filename = argv[1]; //filename is second argument, first being command to run program
      printf("Filename passed: %s\n", filename);
      readFile(filename);
    }
    else if(argc < 2){
      printf("Error. Supply png file to overlay.\n");
      exit(1);
    }

    return 0;

}

void readFile(const char * filename){
    int numBytes = 0;
    int isPngFile = 0; 
    FILE *pngFile;

    /*
      we are processing PNG files so skip/ check the header for valid file info.
      First 8 bytes of PNG file : http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

      The first eight bytes of a PNG file always contain the following (decimal) values:

      137 80 78 71 13 10 26 10

      This signature indicates that the remainder of the file contains a single PNG image,
       consisting of a series of chunks beginning with an IHDR chunk and ending with an IEND chunk.
    */
    //process header information for filetype
    char PNGHEADER_BYTES[8] = {137,80,78,71,13,10,26,10}; //decimal form of defined PNG header 8 bytes
    char pngHeaderBuffer[PNG_HEADER_SIZE];
    char byteBuffer[1];
    char chunkFieldBuffer[4]; //buffer for reading field by field on chunks
    char * lengthPtr = chunkFieldBuffer;

    pngFile = fopen(filename,"rb"); //read binary value of png file.

    if(!pngFile){
      printf("Error opening file\n");
    }

    //seek to beginning of the file
    fseek(pngFile,0,SEEK_SET); //SEEK_SET == 0
    /* Read and display data
       place data into pngHeaderBuffer,Size of 1 byte , PNG_HEADER_SIZE = 8 elements to read, from pngFile ptr
    */ 
    fread(pngHeaderBuffer, 1, PNG_HEADER_SIZE, pngFile);

    //check if the read header data matches defined png format
    int cmpVal = strncmp(PNGHEADER_BYTES, pngHeaderBuffer, 8); //first 8 chars
    if(cmpVal == 0){
      isPngFile = 1;
    }
    else{
      isPngFile = 0;
      printf("Not PNG file: %d.\n", cmpVal);
    }

    if(isPngFile){
      //seek to desired post-png header
      fseek(pngFile,8, SEEK_SET);

      /*
          Source: http://www.libpng.org/pub/png/spec/1.2/PNG-Structure.html

          PNG file description
          Length
          A 4-byte unsigned integer giving the number of bytes in the chunk's data field. 
          The length counts only the data field, not itself, the chunk type code, or the CRC. 
          Zero is a valid length. Although encoders and decoders should treat the length as unsigned, 
          its value must not exceed 231 bytes


          Chunk Type
          A 4-byte chunk type code. For convenience in description and in examining PNG files, 
          type codes are restricted to consist of uppercase and lowercase ASCII letters 
          (A-Z and a-z, or 65-90 and 97-122 decimal). However, encoders and decoders must treat the codes as fixed 
          binary values, not character strings. For example, it would not be correct to represent the type code 
          IDAT by the EBCDIC equivalents of those letters. Additional naming conventions for chunk types are discussed 
          in the next section.

          Chunk Data
          The data bytes appropriate to the chunk type, if any. This field can be of zero length.

          CRC
          A 4-byte CRC (Cyclic Redundancy Check) calculated on the preceding bytes in the chunk, including the chunk 
          type code and chunk data fields, but not including the length field. The CRC is always present, even for 
          chunks containing no data. See CRC algorithm.
      */
      
      //read 4 bytes to figure out length of data
      //int guaranteed to be at least 32 bit, int = ok
      fread(chunkFieldBuffer,4,1,pngFile);
      int chunkLength = chunkFieldBuffer[3]+chunkFieldBuffer[2]*8+chunkFieldBuffer[1]*16+chunkFieldBuffer[0]*24;
      printf("Temp length: %d\n", chunkLength);
      
      //Obtain IHDR chunk
      Chunk * IHDR_CHUNK = malloc(sizeof(Chunk)*1); //allocate a new chunk for critical IHDR chunk ptr
      IHDR_CHUNK->length = malloc(sizeof(char)*4); //32 bit length 
      IHDR_CHUNK->type = malloc(sizeof(char)*4); //32 bit type code (checking bits to determine types)
      IHDR_CHUNK->data = malloc(sizeof(char)*chunkLength); //we know how much memory to allocate from the chunks 4 byte length chunk
      IHDR_CHUNK->crc = malloc(sizeof(char)*4); //32 bit crc

      strcpy(IHDR_CHUNK->length,"h");
      printf("length struct: %s\n", IHDR_CHUNK->length);



      //Read 4 bytes at a time until EOF
      while(fread(byteBuffer,1,1,pngFile)){
          //printf("Chunk Buffer: %x \n", byteBuffer[0] & 0xff);
          numBytes++;
      }
      printf("Number Byte: %d\n", numBytes);

      fclose(pngFile); //close the file
      //free allocated memory
      free(IHDR_CHUNK->crc);
      free(IHDR_CHUNK->data);
      free(IHDR_CHUNK->type);
      free(IHDR_CHUNK->length);
      free(IHDR_CHUNK);

    }

}
